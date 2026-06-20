#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include<signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include<fcntl.h>
#include "hostapd.h"
#include "dnsmasq.h"
#include "cli.h"
#include "firewall.h"

int pid1 = -1,pid2 = -1;
char iface[32];
int modified_env = 0;
int activated_firewall = 0;

void cleanup(){

    static int check = 0;
    if(check){return;}
    check = 1;

    if(pid1>0) {kill(pid1,SIGTERM);}
    if(pid2>0) {kill(pid2,SIGTERM);}
    if(activated_firewall){
        firewall_teardown();
    }
    char cmd[256];
    if(modified_env){
        snprintf(cmd, sizeof(cmd), "ip addr flush dev %s", iface);
        system(cmd);
        if (system("which nmcli > /dev/null 2>&1") == 0)
        {
            snprintf(cmd, sizeof(cmd),"nmcli device set %s managed yes", iface);
            system(cmd);
        }
    }
    system("rm -rf /run/hotspotctl");

    printf("\nRestored system state\n");
    printf("Terminated hotspotctl\n");

}

int prepare_environment(HotspotConfig cfg){
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "ip link set %s up", cfg.iface);
    system(cmd);
    if (system("which nmcli > /dev/null 2>&1") == 0)
    {
        snprintf(cmd, sizeof(cmd), "nmcli device set %s managed no", iface);
        system(cmd);
    }
    sleep(1);
    snprintf(cmd, sizeof(cmd), "ip addr flush dev %s", cfg.iface);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "ip addr add 192.168.42.1/24 dev %s", cfg.iface);
    system(cmd);
    return 0;
}

void handle_signal_interrupt(int signal_number){
    (void)signal_number;

    exit(0);
}

int main(int argc,char* argv[])
{
    //Check for root access
    if(geteuid() != 0){
        fprintf(stderr,"Hotspotctl requires root privilages.\n");
        fprintf(stderr,"Please run it again using : sudo %s\n",argv[0]);
        return 1;
    }

    //Handle program exit
    if(atexit(cleanup)!=0){
        fprintf(stderr,"Failed to clean up\n");
        return 1;
    }

    struct sigaction response_action;
    response_action.sa_handler = handle_signal_interrupt;
    sigemptyset(&response_action.sa_mask);
    response_action.sa_flags = 0;
    sigaction(SIGINT, &response_action, NULL);
    sigaction(SIGTERM, &response_action, NULL);
    
    //Prepare configuration
    HotspotConfig cfg = get_cli_cfg(argc,argv);
    strcpy(iface,cfg.iface);

    //Flag altered env
    if(prepare_environment(cfg)==0){
        modified_env = 1;
    }
    
    if(create_hostapd_conf(&cfg)){
        fprintf(stderr,"An error occured while creating hostapd.conf\n");
        exit(1);
    }
    if(create_dnsmasq_conf(&cfg)){
        fprintf(stderr,"An error occured while creating dnsmasq.conf\n");
        exit(1);
    }

    //Bringing up Access Point
    pid1 = fork();
    if (pid1 == 0)
    {
        int log_hostapd_f = open("/run/hotspotctl/hostapd.log",O_WRONLY | O_CREAT | O_TRUNC,0644);
        if(log_hostapd_f!=-1){
            dup2(log_hostapd_f,STDOUT_FILENO);
            dup2(log_hostapd_f,STDERR_FILENO);
            close(log_hostapd_f);
        }
        execlp("hostapd", "hostapd", "/run/hotspotctl/hostapd.conf", (char *)NULL);
        _exit(1);
    }
    
    sleep(2);
    //Bringing up DNS and DHCP
    pid2 = fork();
    if (pid2 == 0)
    {
        int log_dnsmasq_f = open("/run/hotspotctl/dnsmasq.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (log_dnsmasq_f != -1)
        {
            dup2(log_dnsmasq_f, STDOUT_FILENO);
            dup2(log_dnsmasq_f, STDERR_FILENO);
            close(log_dnsmasq_f);
        }
        execlp("dnsmasq", "dnsmasq", "--conf-file=/run/hotspotctl/dnsmasq.conf", "--keep-in-foreground", "--log-facility=-", (char *)NULL);
        _exit(1);
    }


    //Setup routing

    if(firewall_enable_forwarding()==0){
        activated_firewall = 1;
    }
    firewall_setup(cfg.iface,cfg.uplink);

    //Success message
    printf("Created Hotspot Successfully\n");
    printf("Connection Name : %s\n",cfg.ssid);
    printf("Password : %s\n",cfg.password);

    //Keeping parent process alive while children processes still exist
    wait(NULL);
    wait(NULL);
    

    return 0;
}
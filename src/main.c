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

void cleanup(){

    static int check = 0;
    if(check){return;}
    check = 1;

    if(pid1>0) {kill(pid1,SIGTERM);}
    if(pid2>0) {kill(pid2,SIGTERM);}
    firewall_teardown();

}

void handle_signal_interrupt(int signal_number){
    (void)signal_number;

    exit(0);
}

int main(int argc,char* argv[])
{
    
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

    HotspotConfig cfg = get_cli_cfg(argc,argv);

    //Prepare the Environment
    char cmd[256];
    snprintf(cmd,sizeof(cmd),"ip link set %s up",cfg.iface); 
    system(cmd);
    snprintf(cmd,sizeof(cmd),"nmcli device set %s managed no",cfg.iface);
    system(cmd);
    sleep(1);
    snprintf(cmd,sizeof(cmd),"ip addr flush dev %s",cfg.iface);
    system(cmd);
    snprintf(cmd,sizeof(cmd),"ip addr add 192.168.42.1/24 dev %s",cfg.iface);
    system(cmd);
    create_hostapd_conf(&cfg);
    create_dnsmasq_conf(&cfg);

    pid1 = fork();

    if (pid1 == 0)
    {
        int log_f = open("/run/hotspotctl/hostapd.log",O_WRONLY | O_CREAT | O_TRUNC,0644);
        if(log_f!=-1){
            dup2(log_f,STDOUT_FILENO);
            dup2(log_f,STDERR_FILENO);
            close(log_f);
        }
        execlp("hostapd", "hostapd", "/run/hotspotctl/hostapd.conf", (char *)NULL);
        _exit(1);
    }

    pid2 = fork();

    if (pid2 == 0)
    {
        execlp("dnsmasq", "dnsmasq", "--conf-file=/run/hotspotctl/dnsmasq.conf", "--keep-in-foreground", (char *)NULL);
        _exit(1);
    }
     
    firewall_enable_forwarding();
    firewall_setup(cfg.iface,cfg.uplink);
    printf("Created Hotspot Successfully\n");
    printf("Connection Name : %s\n",cfg.ssid);
    printf("Password : %s\n",cfg.password);


    wait(NULL);
    wait(NULL);
    

    return 0;
}
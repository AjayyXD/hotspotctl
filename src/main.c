#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "hostapd.h"
#include "dnsmasq.h"
#include "cli.h"
#include "firewall.h"
#include "version.h"
#include "docs.h"

int pid1 = -1,pid2 = -1;
char iface[32];
int modified_env = 0;
int activated_firewall = 0;
int isDaemon = 0;

void cleanup(){

    static int check = 0;
    if(check){return;}
    if(!isDaemon){return;}
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

    printf("\n[*] Restored system state\n");
    printf("[*] Terminated hotspotctl\n");

}

int prepare_environment(HotspotConfig cfg){
    char cmd[256];
    snprintf(cmd,sizeof(cmd),"rfkill unblock wlan");
    system(cmd);
    snprintf(cmd, sizeof(cmd), "ip link set %s up", cfg.iface);
    system(cmd);
    if (system("which nmcli > /dev/null 2>&1") == 0)
    {
        snprintf(cmd, sizeof(cmd), "nmcli device set %s managed no", cfg.iface);
        system(cmd);
    }
    sleep(1);
    snprintf(cmd, sizeof(cmd), "ip addr flush dev %s", cfg.iface);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "ip addr add 192.168.42.1/24 dev %s", cfg.iface);
    system(cmd);
    modified_env = 1;
    return 0;
}

void handle_signal_interrupt(int signal_number){
    (void)signal_number;

    exit(0);
}

void daemonize()
{
    pid_t pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "[-] Failed to fork\n");
        exit(1);
    }
    if (pid > 0)
    {
        exit(0);
    }

    if (setsid() < 0)
    {
        fprintf(stderr, "[-] Setsid failed\n");
        exit(1);
    }
    pid = fork();
    if (pid > 0)
    {
        exit(0);
    }
    int devnull = open("/dev/null", O_RDWR);
    dup2(devnull, STDIN_FILENO);
    dup2(devnull, STDOUT_FILENO);
    dup2(devnull, STDERR_FILENO);
    close(devnull);

    chdir("/");
}

int kill_hotspotctl(){
    FILE *f = fopen("/run/hotspotctl/hotspotctl.pid", "r");
    if (!f)
    {
        return 3;
    }
    char line[32];
    fgets(line, sizeof(line), f);
    pid_t pid = atoi(line);
    if (pid <= 0)
    {
        return 2;
    }

    if (kill(pid, SIGTERM) == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int write_state(HotspotConfig cfg){
    FILE* fp = fopen("/run/hotspotctl/hotspotctl.state","w");
    if(fp==NULL){
        return 1;
    }
    fprintf(fp,"SSID : %s\n",cfg.ssid);
    fprintf(fp,"IFACE : %s\n",cfg.iface);
    fprintf(fp,"UPLINK : %s\n",cfg.uplink);
    if(strcmp(cfg.hw_mode,"g")==0){
        fprintf(fp,"BAND : 2.4GHz\n");
    }else{
        fprintf(fp,"BAND : 5GHz\n");
    }
    fclose(fp);
    return 0;
}

int root_access(char* argv[]){
    if(geteuid() != 0){
        fprintf(stderr, "[-] Hotspotctl requires root privileges.\n");
        fprintf(stderr, "[-] Please run it again using : sudo %s\n", argv[0]);
        return 1;
    }
    return 0;
}

int status(){
    char buffer[64];
    FILE *fp = fopen("/run/hotspotctl/hotspotctl.state", "r");
    if (fp == NULL)
    {
        fprintf(stderr, "[-] Error occured while fetching the state\n");
        return 1;;
    }
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        fprintf(stdout, buffer);
    }
    return 0;
}

int check_mode(HotspotConfig *cfg,int argc,char *argv[]){
    if (strcmp(argv[1], "start") == 0)
    {
        if(root_access(argv)) return 1;
        int status = kill_hotspotctl();
        if(status==0){
            fprintf(stdout,"[*] Killing existing hotspotctl\n");
        }else if(status==1 || status==2 ){
            fprintf(stderr,"[-] Hotspotctl already exists, failed to kill exisiting hotspotctl\n");
            exit(1);
        }
        start_parse(cfg, argc, argv);
    }
    else if (strcmp(argv[1], "stop") == 0)
    {
        if(root_access(argv)) return 1;
        int status = kill_hotspotctl();
        if(status==0){
            fprintf(stdout,"[*] Hotspotctl going down\n");
            exit(0);
        }else if(status==1){
            fprintf(stderr, "[-] Failed to stop\n");
            exit(1);
        }else if(status==2){
            fprintf(stderr,"[-] Error, invalid pid\n");
            exit(1);
        }else if(status==3){
            fprintf(stderr, "[-] Error, could not open file\n");
            exit(1);
        }
        
    }else if(strcmp(argv[1],"--version")==0){
        fprintf(stdout,"[*] hotspotctl v%s\n",version);
        exit(0);
    }else if (strcmp(argv[1],"--help")==0){
        help();
        exit(0);
    }else if(strcmp(argv[1],"--connected")==0){
        char iface [64];
        FILE* fp = fopen("/run/hotspotctl/hotspotctl.state","r");
        if(fp == NULL){
            fprintf(stderr,"[-] Error occured while fetching the state\n");
            exit(1);
        }
        char buffer[64];
        fgets(buffer,sizeof(iface),fp);
        fgets(buffer,sizeof(iface),fp);
        sscanf(buffer,"IFACE : %s",iface);

        
        get_connected_devices(iface);
        exit(0);
    }else if(strcmp(argv[1],"--status") == 0){
        int status_result = status();
        exit(status_result);
    }
    else{
        fprintf(stdout,"[-] Unknown usage\n");
        exit(1);
    }

    return 0;
}



int main(int argc,char* argv[])
{
    
    
    //Handle program exit
    if(atexit(cleanup)!=0){
        fprintf(stderr,"[-] Failed to clean up\n");
        return 1;
    }
    
    //signal interrupts
    struct sigaction response_action;
    response_action.sa_handler = handle_signal_interrupt;
    sigemptyset(&response_action.sa_mask);
    response_action.sa_flags = 0;
    sigaction(SIGINT, &response_action, NULL);
    sigaction(SIGTERM, &response_action, NULL);
    sigaction(SIGHUP, &response_action,NULL);

    

    //Prepare configuration
    HotspotConfig cfg = get_cli_cfg(argc,argv);
    

    if(check_mode(&cfg,argc,argv)){
        exit(1);
    }

    strcpy(iface, cfg.iface);

    // Flag altered env
    if (prepare_environment(cfg))
    {
        fprintf(stderr,"[-] An error occurred while preparing the environment\n");
        exit(1);
    }

    if (create_hostapd_conf(&cfg))
    {
        fprintf(stderr, "[-] An error occurred while creating hostapd.conf\n");
        exit(1);
    }
    if (create_dnsmasq_conf(&cfg))
    {
        fprintf(stderr, "[-] An error occurred while creating dnsmasq.conf\n");
        exit(1);
    }

    //Bringing up Access Point
    pid1 = fork();
    if (pid1 == 0)
    {
        if(!cfg.debug_mode){
            int log_hostapd_f = open("/run/hotspotctl/hostapd.log",O_WRONLY | O_CREAT | O_TRUNC,0644);
            if(log_hostapd_f!=-1){
                dup2(log_hostapd_f,STDOUT_FILENO);
                dup2(log_hostapd_f,STDERR_FILENO);
                close(log_hostapd_f);
            }    
        }
        execlp("hostapd", "hostapd", "/run/hotspotctl/hostapd.conf", (char *)NULL);
        _exit(1);
    }
    
    sleep(2);
    //Bringing up DNS and DHCP
    pid2 = fork();
    if (pid2 == 0)
    {
        if(!cfg.debug_mode){
            int log_dnsmasq_f = open("/run/hotspotctl/dnsmasq.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (log_dnsmasq_f != -1)
            {
                dup2(log_dnsmasq_f, STDOUT_FILENO);
                dup2(log_dnsmasq_f, STDERR_FILENO);
                close(log_dnsmasq_f);
            }
        }
        execlp("dnsmasq", "dnsmasq", "--conf-file=/run/hotspotctl/dnsmasq.conf", "--keep-in-foreground", "--log-facility=-", (char *)NULL);
        _exit(1);
    }


    //Setup routing

    if(firewall_enable_forwarding()==0){
        activated_firewall = 1;
    }
    firewall_setup(cfg.iface,cfg.uplink);

    if(write_state(cfg)){
        fprintf(stdout,"[-] Could not write state file\n");
        exit(1);
    }

    //Success message
    printf("[*] Created Hotspot Successfully\n");
    printf("[*] Connection Name : %s\n",cfg.ssid);
    printf("[*] Password : %s\n",cfg.password);
    if (strcmp(argv[1], "start") == 0)
    {
        daemonize();
        isDaemon = 1;
        FILE *f = fopen("/run/hotspotctl/hotspotctl.pid", "w");
        if (f)
        {
            fprintf(f, "%d\n", getpid());
            fclose(f);
        }
        else
        {
            fprintf(stderr, "No directory\n");
        }
        
    }
    
    //Keeping parent process alive while children processes still exist
    while(1){
        pause();
    }
    

    return 0;
}
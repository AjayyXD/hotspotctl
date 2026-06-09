#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "hostapd.h"
#include "dnsmasq.h"
#include "cli.h"

int main(int argc,char* argv[])
{
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

    int pid = fork();

    if (pid == 0)
    {
        execlp("hostapd", "hostapd", "/run/hotspotctl/hostapd.conf", (char *)NULL);
        _exit(1);
    }

    int pid2 = fork();

    if (pid2 == 0)
    {
        execlp("dnsmasq", "dnsmasq", "--conf-file=/run/hotspotctl/dnsmasq.conf", "--keep-in-foreground", (char *)NULL);
        _exit(1);
    }

    wait(NULL);
    wait(NULL);

    return 0;
}
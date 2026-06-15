#include<stdio.h>
#include<stdlib.h>
#include "firewall.h"

int firewall_enable_forwarding(){
    system("sysctl -w net.ipv4.ip_forward=1 > /dev/null 2>&1");
    return 0;
}

int firewall_setup(char *iface,char *uplink){

    system("nft add table ip hotspotctl_nat");
    system("nft add table ip hotspotctl_filter");
    system("nft add chain ip hotspotctl_nat postrouting { type nat hook postrouting priority 100 \\; }");
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "nft add rule ip hotspotctl_nat postrouting oifname \"%s\" masquerade",uplink);
    system(cmd);
    system("nft add chain ip hotspotctl_filter forward { type filter hook forward priority 0 \\; }");
    snprintf(cmd, sizeof(cmd), "nft add rule ip hotspotctl_filter forward iifname \"%s\" oifname \"%s\" accept",iface,uplink);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "nft add rule ip hotspotctl_filter forward iifname \"%s\" oifname \"%s\" ct state established,related accept",uplink,iface);
    system(cmd);
    return 0;
}

int firewall_teardown(){
    system("nft delete table ip hotspotctl_nat");
    system("nft delete table ip hotspotctl_filter");
    return 0;
}
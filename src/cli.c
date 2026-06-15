#define _GNU_SOURCE
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include "hostapd.h"
#include "auto.h"

HotspotConfig  get_cli_cfg(int argc,char *argv[]){

    // Initialize Default Values
    HotspotConfig cfg;
    strcpy(cfg.ssid, "Hotspotctl");
    strcpy(cfg.iface, "wlan0");
    strcpy(cfg.uplink,"eth0");
    strcpy(cfg.password, "strongpassword");
    cfg.channel = 6;
    cfg.max_clients = 10;

    int opt;

    //Check if no flags
    if(argc<2){
        printf("[*] No configuration flags provided. Running automatic setup...\n");
        if (get_auto_cfg(&cfg))
        {
            fprintf(stderr,"[-] Unable to auto fetch\n");
            exit(1);
        }
        return cfg;
    }

    //Parse the arguments
    while ((opt = getopt(argc, argv, "i:s:p:c:u:a")) != -1)
    {
        switch (opt)
        {
        case 'i':
            strncpy(cfg.iface, optarg, sizeof(cfg.iface) - 1);
            break;
        case 's':
            strncpy(cfg.ssid, optarg, sizeof(cfg.ssid) - 1);
            break;
        case 'p':
            strncpy(cfg.password, optarg, sizeof(cfg.password) - 1);
            break;
        case 'c':
            cfg.channel = atoi(optarg);
            break;
        case 'u' :
            strncpy(cfg.uplink,optarg,sizeof(cfg.uplink)-1);
            break;
        case 'a' :
            if(get_auto_cfg(&cfg)){
                fprintf(stderr,"[-] Some error occurred while auto fetching");
                exit(1);
            }
            break;
        default:
            fprintf(stderr, "[*] Usage: %s [-i interface] [-s ssid] [-p password] [-c channel] [-u uplink]\n", argv[0]);
            exit(1);
        }
    }
    return cfg;
}


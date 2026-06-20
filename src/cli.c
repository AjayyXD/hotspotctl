#define _GNU_SOURCE
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include "hostapd.h"
#include "auto.h"

void channel_error(){
    fprintf(stderr, "[-] Error : Hotspotctl only supports universally safe channels.\n");
    fprintf(stderr, "    Please choose a clean channel to prevent interference or crashes\n");
    fprintf(stderr, "    -> For 2.4GHz : 1, 6, 11\n");
    fprintf(stderr, "    -> For 5.0GHz : 36, 40, 44, 48\n");
    exit(1);
}

HotspotConfig  get_cli_cfg(int argc,char *argv[]){

    // Initialize Default Values
    HotspotConfig cfg;
    strcpy(cfg.ssid, "Hotspotctl");
    strcpy(cfg.iface, "wlan0");
    strcpy(cfg.uplink,"eth0");
    strcpy(cfg.password, "strongpassword");
    cfg.channel = 6;
    cfg.max_clients = 10;
    strcpy(cfg.hw_mode,"g");
    strcpy(cfg.ht_capab,"");

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
    while ((opt = getopt(argc, argv, "b:i:s:p:c:u:a")) != -1)
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
            if(strlen(optarg)<8){
                fprintf(stderr,"[-] Entered password must contain atleast 8 characters\n");
                exit(1);
            }else if(strlen(optarg)>63){
                fprintf(stderr,"[-] Entered password must contain less than 64 characters\n");
                exit(1);
            }
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
        case 'b' :
            char hw_mode[4];
            strncpy(hw_mode,optarg,sizeof(hw_mode)-1);
            if(strcmp(hw_mode,"a")==0){
                if(support_5g()){
                strcpy(cfg.hw_mode,"a");
                strcpy(cfg.ht_capab, "ht_capab=[HT40-]");
                cfg.channel = 48;
                }else{
                    fprintf(stderr,"Network card does not support 5GHz, Reverting back to 2.4GHz\n");
                }
            }else if(strcmp(hw_mode,"g")==0){
                cfg.channel = 6;
                strcpy(cfg.hw_mode,"g");
                strcpy(cfg.ht_capab,"");
            }else{
                fprintf(stderr,"Invalid -b flag, using 2.4GHz as default\n");
                cfg.channel = 6;
                strcpy(cfg.hw_mode, "g");
                strcpy(cfg.ht_capab, "");
            }
            break;
        default:
            fprintf(stderr, "[*] Usage: %s [-i interface] [-s ssid] [-p password] [-c channel] [-u uplink]\n", argv[0]);
            exit(1);
        }
    }

    if(strcmp(cfg.hw_mode,"g")==0){
        if(cfg.channel!=1 && cfg.channel!=6 && cfg.channel!=11){
            channel_error();
        }
    }else{
        if(cfg.channel!=36 && cfg.channel!=40 && cfg.channel!= 44 && cfg.channel!= 48){
            channel_error();
        }
    }
    return cfg;
}


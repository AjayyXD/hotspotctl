#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "auto.h"

int get_auto_cfg(HotspotConfig *cfg){
    FILE *f = popen("ls /sys/class/net","r");
    char buffer[256];
    while(fgets(buffer,sizeof(buffer),f)!=NULL){

        buffer[strcspn(buffer, "\r\n")] = '\0';
        if(strcmp(buffer,"lo")==0){continue;}

        if(buffer[0]=='w'){
            strncpy(cfg->iface,buffer,sizeof(cfg->iface)-1);
            cfg->iface[sizeof(cfg->iface) - 1] = '\0';
        }else if(buffer[0]=='e'){
            strncpy(cfg->uplink,buffer,sizeof(cfg->uplink)-1);
            cfg->uplink[sizeof(cfg->uplink) - 1] = '\0';
        }
    }
    pclose(f);
    return 0;
}


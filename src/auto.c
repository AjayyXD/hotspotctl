#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "auto.h"

int support_5g()
{
    FILE *f = popen("iw list", "r");
    if (!f)
    {
        return 0;
    }
    char line[256];
    while (fgets(line, sizeof(line), f))
    {
        int freq = 0;
        if (sscanf(line, " * %d MHz", &freq) == 1)
        {
            if (freq >= 5000 && freq <= 5900)
            {
                pclose(f);
                return 1;
            }
        }
    }
    pclose(f);
    return 0;
}

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

        if (support_5g())
        {
            strcpy(cfg->hw_mode, "a");
            strcpy(cfg->ht_capab, "ht_capab=[HT40-]");
            cfg->channel = 48;
        }
        else
        {
            fprintf(stderr,"5GHz not supported, using 2.4GHz instead\n");
            strcpy(cfg->ht_capab, "g");
            strcpy(cfg->ht_capab, "");
        }
    }
    pclose(f);
    return 0;
}



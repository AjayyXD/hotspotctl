#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include "hostapd.h"
#include "auto.h"


int create_hostapd_conf(HotspotConfig *cfg){
        int dir = mkdir("/run/hotspotctl",0755);
        if(dir != 0 && errno != EEXIST){
                perror("mkdir failed");
                return 1;
        }
        FILE *f = fopen("/run/hotspotctl/hostapd.conf","w");
        fprintf(f, "interface=%s\n"
                   "driver=nl80211\n"
                   "ssid=%s\n"
                   "country_code=IN\n"
                   "hw_mode=%s\n"
                   "channel=%d\n"
                   "ieee80211n=1\n"
                   "%s\n"
                   "wmm_enabled=1\n"
                   "wpa=2\n"
                   "auth_algs=1\n"
                   "wpa_key_mgmt=WPA-PSK\n"
                   "wpa_pairwise=CCMP\n"
                   "wpa_passphrase=%s\n"
                   "max_num_sta=%d\n"
                   "logger_stdout=-1\n"
                   "logger_stdout_level=2\n",cfg->iface,cfg->ssid,cfg->hw_mode,cfg->channel,cfg->ht_capab,cfg->password,cfg->max_clients);
        fclose(f);
        return 0;
}





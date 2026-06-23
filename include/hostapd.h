#ifndef hostapd_h
#define hostapd_h


typedef struct
{
    char ssid[64];
    char password[64];
    char iface[32];
    char uplink[32];
    char hw_mode[64];
    char ht_capab[64];
    char country_code[8];
    int channel;
    int max_clients;
    int debug_mode;
} HotspotConfig;


int create_hostapd_conf(HotspotConfig *cfg);

#endif
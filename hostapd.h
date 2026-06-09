#ifndef hostapd_h
#define hostapd_h


typedef struct
{
    char ssid[64];
    char password[64];
    char iface[32];
    int channel;
    int max_clients;
} HotspotConfig;


int create_hostapd_conf(HotspotConfig *cfg);

#endif
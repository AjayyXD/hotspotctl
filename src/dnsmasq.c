#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "hostapd.h"

int create_dnsmasq_conf(HotspotConfig *cfg)
{
    FILE *f = fopen("/run/hotspotctl/dnsmasq.conf", "w");
    if (f == NULL)
    {
        printf("Could not make dnsmasq conf");
        return 1;
    }

    fprintf(f,
            "interface=%s\n"
            "bind-interfaces\n"
            "port=0\n"
            "dhcp-range=192.168.42.10,192.168.42.100,255.255.255.0,24h\n"
            "dhcp-option=3,192.168.42.1\n"
            "dhcp-option=6,8.8.8.8,8.8.4.4\n"
            "no-resolv\n",
            cfg->iface);
    fclose(f);
    return 0;
}
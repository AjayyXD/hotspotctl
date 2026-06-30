#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include<sys/stat.h>
#include <stdlib.h>
#include<fcntl.h>
#include<signal.h>
#include "hostapd.h"
#include "auto.h"

#define FLAG_SSID (1 << 0)
#define FLAG_PASSWORD (1 << 1)
#define FLAG_IFACE (1 << 2)
#define FLAG_UPLINK (1 << 3)
#define FLAG_COUNTRY (1 << 4)
#define FLAG_CHANNEL (1 << 5)
#define FLAG_BAND (1 << 6)
#define REQUIRED_FLAGS (FLAG_SSID | FLAG_PASSWORD | FLAG_IFACE | FLAG_UPLINK | FLAG_COUNTRY | FLAG_CHANNEL | FLAG_BAND)

void channel_error()
{
    fprintf(stderr, "[-] Error : Hotspotctl only supports universally safe channels.\n");
    fprintf(stderr, "    Please choose a clean channel to prevent interference or crashes\n");
    fprintf(stderr, "    -> For 2.4GHz : 1, 6, 11\n");
    fprintf(stderr, "    -> For 5.0GHz : 36, 40, 44, 48\n");
    exit(1);
}

int start_parse(HotspotConfig *cfg, int argc, char *argv[])
{
    int shifted_argc = argc - 2;
    char **shifted_argv = argv + 2;
    int opt;

    if (argc-2 == 0)
    {
        printf("[*] No configuration flags provided. Running automatic setup...\n");
        if (get_auto_cfg(cfg))
        {
            fprintf(stderr, "[-] Unable to auto fetch\n");
            exit(1);
        }
        return 0;
    }

    if (strcmp(argv[2], "-a") == 0)
    {
        if (get_auto_cfg(cfg))
        {
            fprintf(stderr, "[-] Some error occurred while auto fetching");
            exit(1);
        }
    }
    else if (strcmp(argv[2], "-m") != 0)
    {
        fprintf(stderr, "[-] Error : The first flag must be -a or -m\n");
        exit(1);
    }
    opterr = 0;
    unsigned int flags_provided = 0;
    while ((opt = getopt(shifted_argc, shifted_argv, "b:i:s:p:c:u:r:d")) != -1)
    {
        switch (opt)
        {
        case 'i':
            strncpy(cfg->iface, optarg, sizeof(cfg->iface) - 1);
            flags_provided = flags_provided | FLAG_IFACE;
            break;
        case 's':
            strncpy(cfg->ssid, optarg, sizeof(cfg->ssid) - 1);
            flags_provided = flags_provided | FLAG_SSID;
            break;
        case 'p':
            if (strlen(optarg) < 8)
            {
                fprintf(stderr, "[-] Entered password must contain at least 8 characters\n");
                exit(1);
            }
            else if (strlen(optarg) > 63)
            {
                fprintf(stderr, "[-] Entered password must contain less than 64 characters\n");
                exit(1);
            }
            strncpy(cfg->password, optarg, sizeof(cfg->password) - 1);
            flags_provided = flags_provided | FLAG_PASSWORD;
            break;
        case 'c':
            cfg->channel = atoi(optarg);
            flags_provided = flags_provided | FLAG_CHANNEL;
            break;
        case 'u':
            strncpy(cfg->uplink, optarg, sizeof(cfg->uplink) - 1);
            flags_provided = flags_provided | FLAG_UPLINK;
            break;
        case 'b':
            char hw_mode[4];
            strncpy(hw_mode, optarg, sizeof(hw_mode) - 1);
            if (strcmp(hw_mode, "a") == 0)
            {
                if (support_5g())
                {
                    strcpy(cfg->hw_mode, "a");
                    strcpy(cfg->ht_capab, "ht_capab=[HT40-]");
                    cfg->channel = 48;
                }
                else
                {
                    fprintf(stderr, "Network card does not support 5GHz, Reverting back to 2.4GHz\n");
                }
            }
            else if (strcmp(hw_mode, "g") == 0)
            {
                cfg->channel = 6;
                strcpy(cfg->hw_mode, "g");
                strcpy(cfg->ht_capab, "");
            }
            else
            {
                fprintf(stderr, "Invalid -b flag, using 2.4GHz as default\n");
                cfg->channel = 6;
                strcpy(cfg->hw_mode, "g");
                strcpy(cfg->ht_capab, "");
            }
            flags_provided = flags_provided | FLAG_BAND;
            break;
        case 'r':
            if (strlen(optarg) != 2)
            {
                fprintf(stderr, "[-] Error : Invalid country code\n");
                exit(1);
            }
            strncpy(cfg->country_code, optarg, sizeof(cfg->country_code) - 1);
            flags_provided = flags_provided | FLAG_COUNTRY;
            break;
        case 'd':
            cfg->debug_mode = 1;
            break;
        case '?':
            fprintf(stderr, "[-] Error : Unknown flag\n");
            fprintf(stderr, "[*] Usage: %s [-a/-m] [-i interface] [-s ssid] [-p password] [-r region/country code][-c channel] [-b band] [-u uplink]\n", argv[0]);
            fprintf(stderr, "[*] Example: %s -a -s MyWifi -p mypassword\n", argv[0]);
            fprintf(stderr, "[*] Example: %s -m -i wlp8s0 -u enp7s0 -s MyWifi -p mypassword -b a -c 48 -r IN\n", argv[0]);
            exit(1);
        default:
            exit(1);
        }
    }

    if ((flags_provided & REQUIRED_FLAGS) != REQUIRED_FLAGS && strcmp(argv[1], "-m") == 0)
    {
        fprintf(stderr, "[-] Error: Manual mode (-m) requires all configuration flags.\n");
        if (!(flags_provided & FLAG_SSID))
        {
            fprintf(stderr, "  ->Missing -s (SSID)\n");
        }
        if (!(flags_provided & FLAG_PASSWORD))
        {
            fprintf(stderr, "  ->Missing -p (Password)\n");
        }
        if (!(flags_provided & FLAG_IFACE))
        {
            fprintf(stderr, "  ->Missing -i (Interface)\n");
        }
        if (!(flags_provided & FLAG_UPLINK))
        {
            fprintf(stderr, "  ->Missing -u (Uplink)\n");
        }
        if (!(flags_provided & FLAG_CHANNEL))
        {
            fprintf(stderr, "  ->Missing -c (Channel)\n");
        }
        if (!(flags_provided & FLAG_BAND))
        {
            fprintf(stderr, "  ->Missing -b (a/g)\n");
        }
        if (!(flags_provided & FLAG_COUNTRY))
        {
            fprintf(stderr, "  ->Missing -r (Region/Country code)\n");
        }
        exit(1);
    }

    return 0;
}
int display_status(){
    return 0;
}




HotspotConfig get_cli_cfg()
{
    // Initialize Default Values
    HotspotConfig cfg;
    strcpy(cfg.ssid, "Hotspotctl");
    strcpy(cfg.iface, "wlan0");
    strcpy(cfg.uplink, "eth0");
    strcpy(cfg.password, "strongpassword");
    strcpy(cfg.country_code, "IN");
    cfg.channel = 6;
    cfg.max_clients = 10;
    strcpy(cfg.hw_mode, "g");
    strcpy(cfg.ht_capab, "");
    cfg.debug_mode = 0;

   

    if (strcmp(cfg.hw_mode, "g") == 0)
    {
        if (cfg.channel != 1 && cfg.channel != 6 && cfg.channel != 11)
        {
            channel_error();
        }
    }
    else
    {
        if (cfg.channel != 36 && cfg.channel != 40 && cfg.channel != 44 && cfg.channel != 48)
        {
            channel_error();
        }
    }
    return cfg;
}
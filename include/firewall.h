#ifndef firewall_h
#define firewall_h

int firewall_enable_forwarding();

int firewall_setup(char *iface,char *uplink);

int firewall_teardown();

#endif
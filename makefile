CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude

TARGET = hotspotctl

OBJS = main.o hostapd.o dnsmasq.o cli.o firewall.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: src/main.c include/hostapd.h include/dnsmasq.h
	$(CC) $(CFLAGS) -c src/main.c

hostapd.o: src/hostapd.c include/hostapd.h
	$(CC) $(CFLAGS) -c src/hostapd.c

dnsmasq.o: src/dnsmasq.c include/hostapd.h
	$(CC) $(CFLAGS) -c src/dnsmasq.c

cli.o: src/cli.c include/cli.h include/hostapd.h
	$(CC) $(CFLAGS) -c src/cli.c

firewall.o: src/firewall.c include/firewall.h
	$(CC) $(CFLAGS) -c src/firewall.c

clean:
	rm -f $(OBJS) $(TARGET)
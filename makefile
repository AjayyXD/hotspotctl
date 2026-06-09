CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = hotspotctl

OBJS = main.o hostapd.o dnsmasq.o cli.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c hostapd.h dnsmasq.h
	$(CC) $(CFLAGS) -c main.c

hostapd.o: hostapd.c hostapd.h
	$(CC) $(CFLAGS) -c hostapd.c

dnsmasq.o: dnsmasq.c hostapd.h
	$(CC) $(CFLAGS) -c dnsmasq.c

cli.o: cli.c cli.h hostapd.h
	$(CC) $(CFLAGS) -c cli.c

clean:
	rm -f $(OBJS) $(TARGET)
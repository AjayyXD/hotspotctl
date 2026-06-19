

---

## What is hotspotctl?

hotspotctl turns your Linux machine into a WiFi access point in one command.
It manages hostapd, dnsmasq, and nftables automatically — and restores your
system cleanly when you're done.

No GUI. No bloat. Just a hotspot.

---

## Features

```
 ✓  Auto detects WiFi and ethernet interfaces
 ✓  2.4GHz and 5GHz band support with auto detection
 ✓  Sets up NAT and IP forwarding via nftables
 ✓  DHCP server for connected devices
 ✓  Clean teardown — zero leftover rules or files
 ✓  Logs to /run/hotspotctl/ — terminal stays clean
 ✓  Works with or without NetworkManager
```

---

## Installation

**Dependencies**
```bash
sudo pacman -S hostapd dnsmasq nftables iproute2
```

**Build and install**
```bash
git clone https://github.com/AjayyXD/hotspotctl
cd hotspotctl
make
sudo make install
```

---

## Usage

**Auto mode — detects everything automatically:**
```bash
sudo hotspotctl -a
```

**Manual mode — full control:**
```bash
sudo hotspotctl -i wlp8s0 -u enp7s0 -s MyWifi -p mypassword -c 36 -b a
```

**Stop:** `Ctrl+C` — system is fully restored.

---

## Flags

```
  -a          auto detect interfaces and band
  -i <iface>  WiFi interface        (e.g. wlp8s0)
  -u <iface>  uplink interface      (e.g. enp7s0)
  -s <ssid>   network name          (e.g. MyWifi)
  -p <pass>   password              (min 8 chars)
  -c <ch>     channel               (e.g. 6, 36, 48)
  -b <band>   band: g=2.4GHz a=5GHz (e.g. -b a)
```

---

## How it works

```
  [laptop]─────────────────────────────[internet]
     │  enp7s0 (ethernet, uplink)
     │
     │  wlp8s0 (WiFi, access point)
     │
  ┌──┴──────────────────────────────┐
  │         192.168.42.0/24         │
  │                                 │
  │  [phone]  [tablet]  [laptop]    │
  │  .10       .11        .12       │
  └─────────────────────────────────┘

  hostapd   → broadcasts the access point
  dnsmasq   → assigns IPs via DHCP
  nftables  → NAT and IP forwarding
```

---

## Logs

```bash
tail -f /run/hotspotctl/hostapd.log    # access point events
tail -f /run/hotspotctl/dnsmasq.log    # DHCP leases
```

---

## Project Structure

```
hotspotctl/
├── src/
│   ├── main.c       — entry point, signal handling, cleanup
│   ├── cli.c        — argument parsing
│   ├── auto.c       — interface and band auto detection
│   ├── hostapd.c    — hostapd config and management
│   ├── dnsmasq.c    — dnsmasq config and management
│   └── firewall.c   — nftables NAT and forwarding
├── include/
│   └── *.h          — headers
└── makefile
```

---

## Roadmap

```
v0.1  ✓  basic hotspot, auto detection, 5GHz, clean teardown
v0.2  →  connected client display, channel scanning
v0.3  →  WireGuard VPN routing, kill switch
v0.4  →  DNS blocklist, per-client bandwidth limiting
v0.5  →  AUR package, man page
```

---

## Why not NetworkManager?

NetworkManager can create a hotspot — but it's inflexible. hotspotctl is
built for terminal users who want explicit control:

```
NetworkManager    →  fixed channel, no scanning, no VPN routing
hotspotctl        →  auto channel selection, VPN routing (coming),
                      scriptable, works without a desktop environment
```

---

## License

MIT — see LICENSE

---

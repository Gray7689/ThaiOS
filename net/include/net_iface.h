// ThaiOS Network Interface Header
// ==================================
#ifndef _NET_IFACE_H
#define _NET_IFACE_H

#include <thaios.h>

#define IFACE_NAME_MAX 16
#define MAC_ADDR_LEN   6
#define MTU_DEFAULT    1500

typedef struct network_interface {
    char name[IFACE_NAME_MAX];
    u8 mac[MAC_ADDR_LEN];
    u32 ipv4_addr;
    u32 ipv4_netmask;
    u32 ipv4_gateway;
    u32 mtu;
    u64 flags;
    int (*send)(struct network_interface *iface, u8 *data, usize len);
    int (*recv)(struct network_interface *iface, u8 *buf, usize *len);
    void *priv;
    struct network_interface *next;
} net_iface_t;

void net_iface_register(net_iface_t *iface);
net_iface_t *net_iface_by_name(const char *name);
int net_iface_send(net_iface_t *iface, u8 *data, usize len);

#endif

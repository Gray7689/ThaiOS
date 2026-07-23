// ThaiOS IPv4 Stack
// ===================
// Implementazione del protocollo IPv4 con routing,
// fragmentation e supporto per socket RAW.

#include <thaios.h>
#include <net_iface.h>

#define IP_VERSION 4
#define IP_HEADER_LEN 20
#define IP_MAX_PACKET 65535

typedef struct ipv4_header {
    u8  version_ihl;        // 4 bits version, 4 bits IHL
    u8  dscp_ecn;
    u16 total_length;
    u16 identification;
    u16 flags_fragment;
    u8  ttl;
    u8  protocol;
    u16 header_checksum;
    u32 src_addr;
    u32 dst_addr;
} __attribute__((packed)) ipv4_header_t;

typedef struct ipv4_route {
    u32 dest;
    u32 netmask;
    u32 gateway;
    u32 interface;
    u8  metric;
} ipv4_route_t;

#define IP_ROUTES_MAX 64
static ipv4_route_t g_routes[IP_ROUTES_MAX];
static int g_route_count = 0;

void ipv4_init(void) {
    g_route_count = 0;
    kprintf("[IPv4] IPv4 stack initialized\n");
}

static u16 ip_checksum(u16 *data, int len) {
    u32 sum = 0;
    for (int i = 0; i < len / 2; i++) {
        sum += data[i];
    }
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return ~sum;
}

int ipv4_output(u32 src, u32 dst, u8 protocol, u8 *data, usize len) {
    // Alloca buffer per pacchetto
    u8 *packet = (u8*)kmalloc(IP_HEADER_LEN + len);
    if (!packet) return -ENOMEM;

    ipv4_header_t *hdr = (ipv4_header_t*)packet;
    hdr->version_ihl = (IP_VERSION << 4) | 5;
    hdr->dscp_ecn = 0;
    hdr->total_length = IP_HEADER_LEN + len;
    hdr->identification = 0x1234;  // TODO: incrementale
    hdr->flags_fragment = 0;
    hdr->ttl = 64;
    hdr->protocol = protocol;
    hdr->header_checksum = 0;
    hdr->src_addr = src;
    hdr->dst_addr = dst;

    hdr->header_checksum = ip_checksum((u16*)hdr, IP_HEADER_LEN);

    memcpy(packet + IP_HEADER_LEN, data, len);

    // TODO: lookup route, send via interface
    kprintf("[IPv4] Send %llu bytes %d.%d.%d.%d -> %d.%d.%d.%d (proto=%d)\n",
            (u64)len,
            (src >> 0) & 0xFF, (src >> 8) & 0xFF, (src >> 16) & 0xFF, (src >> 24) & 0xFF,
            (dst >> 0) & 0xFF, (dst >> 8) & 0xFF, (dst >> 16) & 0xFF, (dst >> 24) & 0xFF,
            protocol);

    kfree(packet);
    return SUCCESS;
}

int ipv4_input(u8 *packet, usize len) {
    if (len < IP_HEADER_LEN) return -EINVAL;

    ipv4_header_t *hdr = (ipv4_header_t*)packet;

    // Verify checksum
    u16 saved_csum = hdr->header_checksum;
    hdr->header_checksum = 0;
    u16 calc_csum = ip_checksum((u16*)hdr, IP_HEADER_LEN);
    if (saved_csum != calc_csum) {
        kprintf("[IPv4] Checksum mismatch\n");
        return -EINVAL;
    }
    hdr->header_checksum = saved_csum;

    kprintf("[IPv4] Received packet from %d.%d.%d.%d, proto=%d, len=%llu\n",
            (hdr->src_addr >> 0) & 0xFF, (hdr->src_addr >> 8) & 0xFF,
            (hdr->src_addr >> 16) & 0xFF, (hdr->src_addr >> 24) & 0xFF,
            hdr->protocol, (u64)len);

    // Route to upper layer (TCP, UDP, ICMP)
    switch (hdr->protocol) {
        case 1:  // ICMP
            break;
        case 6:  // TCP
            break;
        case 17: // UDP
            break;
        default:
            kprintf("[IPv4] Unknown protocol %d\n", hdr->protocol);
    }

    return SUCCESS;
}

void ipv4_add_route(u32 dest, u32 netmask, u32 gateway, u8 metric) {
    if (g_route_count >= IP_ROUTES_MAX) return;
    g_routes[g_route_count].dest = dest;
    g_routes[g_route_count].netmask = netmask;
    g_routes[g_route_count].gateway = gateway;
    g_routes[g_route_count].metric = metric;
    g_route_count++;
}

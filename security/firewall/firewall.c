// ThaiOS Firewall
// =================
// Stateful packet filter con supporto per NAT, port forwarding,
// e deep packet inspection (DPI) base.

#include <thaios.h>
#include <net_iface.h>

#define FW_MAX_RULES 1024
#define FW_MAX_CONNS 65536

typedef enum fw_action {
    FW_ALLOW,
    FW_DENY,
    FW_DROP,
    FW_LOG,
    FW_NAT
} fw_action_t;

typedef enum fw_direction {
    FW_INBOUND,
    FW_OUTBOUND,
    FW_FORWARD
} fw_direction_t;

typedef struct fw_rule {
    bool enabled;
    char name[64];
    fw_action_t action;
    fw_direction_t direction;
    u32 src_addr;
    u32 src_mask;
    u32 dst_addr;
    u32 dst_mask;
    u16 src_port_start;
    u16 src_port_end;
    u16 dst_port_start;
    u16 dst_port_end;
    u8  protocol;          // 0=any, 6=TCP, 17=UDP, 1=ICMP
    char iface[IFACE_NAME_MAX];
    u8  log_level;
    u64 packets_matched;
    u64 bytes_matched;
} fw_rule_t;

typedef struct fw_conn {
    u32 src_addr;
    u32 dst_addr;
    u16 src_port;
    u16 dst_port;
    u8  protocol;
    u64 established_at;
    u64 last_seen;
    u64 packets;
    u64 bytes;
    bool active;
} fw_conn_t;

static fw_rule_t g_fw_rules[FW_MAX_RULES];
static int g_fw_rule_count = 0;
static fw_conn_t g_fw_conns[FW_MAX_CONNS];
static int g_fw_conn_count = 0;

void fw_init(void) {
    g_fw_rule_count = 0;
    g_fw_conn_count = 0;
    kprintf("[FW] Stateful firewall initialized\n");

    // Default rules: allow all (permissive until configured)
    fw_rule_t default_allow = {
        .enabled = true,
        .name = "Default Allow All",
        .action = FW_ALLOW,
        .direction = FW_OUTBOUND,
        .protocol = 0,
        .packets_matched = 0,
        .bytes_matched = 0
    };
    fw_add_rule(&default_allow);
}

int fw_add_rule(fw_rule_t *rule) {
    if (g_fw_rule_count >= FW_MAX_RULES) return -ENOMEM;
    g_fw_rules[g_fw_rule_count++] = *rule;
    kprintf("[FW] Rule added: %s (action=%d)\n", rule->name, rule->action);
    return SUCCESS;
}

int fw_remove_rule(int index) {
    if (index < 0 || index >= g_fw_rule_count) return -EINVAL;
    for (int i = index; i < g_fw_rule_count - 1; i++) {
        g_fw_rules[i] = g_fw_rules[i + 1];
    }
    g_fw_rule_count--;
    return SUCCESS;
}

fw_action_t fw_process_packet(u32 src, u32 dst, u16 src_port, u16 dst_port,
                               u8 protocol, fw_direction_t dir, usize len) {
    for (int i = 0; i < g_fw_rule_count; i++) {
        fw_rule_t *rule = &g_fw_rules[i];
        if (!rule->enabled) continue;
        if (rule->direction != dir && rule->direction != FW_FORWARD) continue;
        if (rule->protocol != 0 && rule->protocol != protocol) continue;

        // Check addresses
        if (rule->src_addr && (src & rule->src_mask) != (rule->src_addr & rule->src_mask))
            continue;
        if (rule->dst_addr && (dst & rule->dst_mask) != (rule->dst_addr & rule->dst_mask))
            continue;

        rule->packets_matched++;
        rule->bytes_matched += len;

        // Track connection state
        if (rule->action == FW_ALLOW) {
            for (int j = 0; j < FW_MAX_CONNS; j++) {
                if (!g_fw_conns[j].active) {
                    g_fw_conns[j].src_addr = src;
                    g_fw_conns[j].dst_addr = dst;
                    g_fw_conns[j].src_port = src_port;
                    g_fw_conns[j].dst_port = dst_port;
                    g_fw_conns[j].protocol = protocol;
                    g_fw_conns[j].established_at = 0;
                    g_fw_conns[j].active = true;
                    g_fw_conns[j].packets = 1;
                    g_fw_conns[j].bytes = len;
                    g_fw_conn_count++;
                    break;
                }
            }
        }

        return rule->action;
    }

    return FW_DENY;  // Default: deny
}

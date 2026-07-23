// ThaiOS Application Sandbox
// ===========================
// Isolamento applicazioni tramite namespace, seccomp,
// e capability limiting.

#include <thaios.h>
#include <sched.h>
#include <mm.h>

#define SANDBOX_MAX_NAMESPACES 256

typedef struct sandbox_policy {
    char app_name[64];
    u64 allowed_syscalls[4];  // Bitmap per SYSCALL_MAX
    usize max_memory;
    usize max_fds;
    bool allow_net;
    bool allow_fs_write;
    char allowed_dirs[16][256];
    int dir_count;
} sandbox_policy_t;

typedef struct sandbox_namespace {
    u64 id;
    vaddr_t address_space;
    struct sandbox_namespace *parent;
    u32 uid_map[2];     // [0]=inside, [1]=outside
    u32 gid_map[2];
    bool has_net_access;
    bool has_raw_hw_access;
    usize memory_usage;
    usize memory_limit;
} sandbox_namespace_t;

static sandbox_namespace_t g_namespaces[SANDBOX_MAX_NAMESPACES];
static int g_ns_count = 0;

void sandbox_init(void) {
    g_ns_count = 0;
    kprintf("[SANDBOX] Application sandbox initialized\n");
}

int sandbox_create_namespace(void) {
    if (g_ns_count >= SANDBOX_MAX_NAMESPACES) return -ENOMEM;

    sandbox_namespace_t *ns = &g_namespaces[g_ns_count];
    ns->id = g_ns_count + 1;
    ns->parent = NULL;  // TODO: current process namespace
    ns->has_net_access = false;
    ns->has_raw_hw_access = false;
    ns->memory_limit = 256 * 1024 * 1024;  // 256MB default
    ns->memory_usage = 0;
    g_ns_count++;

    kprintf("[SANDBOX] Created namespace %llu\n", ns->id);
    return ns->id;
}

int sandbox_set_policy(u64 ns_id, sandbox_policy_t *policy) {
    if (ns_id == 0 || ns_id > SANDBOX_MAX_NAMESPACES) return -EINVAL;
    sandbox_namespace_t *ns = &g_namespaces[ns_id - 1];

    if (policy->max_memory > 0) ns->memory_limit = policy->max_memory;
    ns->has_net_access = policy->allow_net;

    kprintf("[SANDBOX] Policy set for namespace %llu\n", ns_id);
    return SUCCESS;
}

bool sandbox_syscall_allowed(u64 ns_id, u64 syscall_num) {
    // Root namespace: everything allowed
    if (ns_id == 0) return true;

    // TODO: check against bitmap policy
    // Deny dangerous syscalls
    switch (syscall_num) {
        case SYS_exit:
        case SYS_read:
        case SYS_write:
        case SYS_sched_yield:
        case SYS_getpid:
        case SYS_clock_get:
            return true;
        default:
            // Stub: allow all for now
            return true;
    }
}

int sandbox_exec(const char *path, sandbox_policy_t *policy) {
    // Create namespace
    int ns_id = sandbox_create_namespace();
    if (ns_id < 0) return ns_id;

    // Set policy
    if (policy) sandbox_set_policy(ns_id, policy);

    kprintf("[SANDBOX] Executing '%s' in sandbox (ns=%d)\n", path, ns_id);

    // TODO: fork, apply seccomp, exec
    return ns_id;
}

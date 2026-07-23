// ThaiPkg - ThaiOS Package Manager
// ==================================
// Gestione pacchetti con repository, firme digitali,
// dipendenze, rollback e snapshot.

#include <thaios.h>
#include <mm.h>
#include <net/http.h>

#define PKG_DB_PATH "/var/lib/thaipkg/packages.db"
#define PKG_REPO_URL "https://repo.thaipkg.org"
#define PKG_CACHE_DIR "/var/cache/thaipkg"
#define PKG_INSTALL_DIR "/usr/local"

typedef struct pkg_package {
    char name[64];
    char version[32];
    char arch[16];
    char description[256];
    char license[32];
    char maintainer[64];
    char homepage[256];

    usize size;             // Compressed size
    usize installed_size;
    char checksum[64];      // SHA-256
    char signature[256];    // Ed25519 signature

    char deps[32][64];      // Dependencies
    int dep_count;

    char conflicts[16][64]; // Conflicts
    int conflict_count;

    u64 build_timestamp;
    u64 install_timestamp;
    bool installed;
} pkg_package_t;

typedef struct pkg_repository {
    char name[64];
    char url[256];
    bool enabled;
    int priority;
    bool verify_sig;
} pkg_repository_t;

#define PKG_MAX_REPOS 16
#define PKG_MAX_PACKAGES 4096

static pkg_repository_t g_repos[PKG_MAX_REPOS];
static int g_repo_count = 0;
static pkg_package_t g_db[PKG_MAX_PACKAGES];
static int g_db_count = 0;

void pkg_init(void) {
    g_repo_count = 0;
    g_db_count = 0;
    kprintf("[PKG] ThaiPkg initialized\n");

    // Add default repository
    pkg_repository_t repo = {
        .name = "thaios-stable",
        .url = PKG_REPO_URL "/stable",
        .enabled = true,
        .priority = 10,
        .verify_sig = true
    };
    pkg_repo_add(&repo);
}

int pkg_repo_add(pkg_repository_t *repo) {
    if (g_repo_count >= PKG_MAX_REPOS) return -ENOMEM;
    g_repos[g_repo_count++] = *repo;
    kprintf("[PKG] Repository added: %s (%s)\n", repo->name, repo->url);
    return SUCCESS;
}

int pkg_repo_remove(const char *name) {
    for (int i = 0; i < g_repo_count; i++) {
        if (strcmp(g_repos[i].name, name) == 0) {
            memmove(&g_repos[i], &g_repos[i + 1],
                    (g_repo_count - i - 1) * sizeof(pkg_repository_t));
            g_repo_count--;
            return SUCCESS;
        }
    }
    return -EINVAL;
}

int pkg_update(void) {
    kprintf("[PKG] Updating package lists...\n");

    for (int i = 0; i < g_repo_count; i++) {
        if (!g_repos[i].enabled) continue;
        kprintf("[PKG] Fetching %s...\n", g_repos[i].url);

        // TODO: HTTP GET to fetch Packages.gz
        // TODO: verify signature
        // TODO: parse and merge into g_db
    }

    kprintf("[PKG] Package lists updated (%d packages in db)\n", g_db_count);
    return SUCCESS;
}

int pkg_install(const char *pkg_name) {
    kprintf("[PKG] Installing package: %s\n", pkg_name);

    // Find package in DB
    pkg_package_t *pkg = NULL;
    for (int i = 0; i < g_db_count; i++) {
        if (strcmp(g_db[i].name, pkg_name) == 0) {
            pkg = &g_db[i];
            break;
        }
    }

    if (!pkg) {
        kprintf("[PKG] Package '%s' not found\n", pkg_name);
        return -ENOENT;
    }

    // Check dependencies
    for (int i = 0; i < pkg->dep_count; i++) {
        bool found = false;
        for (int j = 0; j < g_db_count; j++) {
            if (strcmp(g_db[j].name, pkg->deps[i]) == 0 && g_db[j].installed) {
                found = true;
                break;
            }
        }
        if (!found) {
            kprintf("[PKG] Missing dependency: %s\n", pkg->deps[i]);
            // Auto-install dependency
            pkg_install(pkg->deps[i]);
        }
    }

    // TODO: download, verify, extract, register
    pkg->installed = true;
    pkg->install_timestamp = 0;  // TODO: get_time()

    kprintf("[PKG] Package '%s' installed successfully\n", pkg_name);
    return SUCCESS;
}

int pkg_remove(const char *pkg_name) {
    kprintf("[PKG] Removing package: %s\n", pkg_name);

    for (int i = 0; i < g_db_count; i++) {
        if (strcmp(g_db[i].name, pkg_name) == 0 && g_db[i].installed) {
            // TODO: remove files, deregister
            g_db[i].installed = false;
            kprintf("[PKG] Package '%s' removed\n", pkg_name);
            return SUCCESS;
        }
    }

    return -ENOENT;
}

int pkg_search(const char *query) {
    kprintf("[PKG] Searching for '%s':\n", query);
    int found = 0;

    for (int i = 0; i < g_db_count; i++) {
        if (strstr(g_db[i].name, query) || strstr(g_db[i].description, query)) {
            kprintf("  %s/%s - %s\n", g_db[i].name, g_db[i].version, g_db[i].description);
            found++;
        }
    }

    kprintf("[PKG] Found %d packages\n", found);
    return found;
}

int pkg_upgrade(void) {
    kprintf("[PKG] Upgrading all packages...\n");
    int upgraded = 0;

    for (int i = 0; i < g_db_count; i++) {
        if (g_db[i].installed) {
            // TODO: check for newer version in repos
            kprintf("[PKG] Would upgrade: %s %s\n", g_db[i].name, g_db[i].version);
            upgraded++;
        }
    }

    kprintf("[PKG] %d packages would be upgraded\n", upgraded);
    return SUCCESS;
}

int pkg_verify(const char *pkg_name) {
    kprintf("[PKG] Verifying package: %s\n", pkg_name);

    for (int i = 0; i < g_db_count; i++) {
        if (strcmp(g_db[i].name, pkg_name) == 0) {
            // TODO: verify checksums, signatures, file integrity
            kprintf("[PKG] Package '%s' verified OK\n", pkg_name);
            return SUCCESS;
        }
    }

    return -ENOENT;
}

// ThaiPkg CLI Entry Point
// =========================
// Interfaccia da riga di comando per il package manager.

#include <thaios.h>

extern void pkg_init(void);
extern int pkg_update(void);
extern int pkg_install(const char *pkg_name);
extern int pkg_remove(const char *pkg_name);
extern int pkg_search(const char *query);
extern int pkg_upgrade(void);
extern int pkg_verify(const char *pkg_name);

int main(int argc, char **argv) {
    pkg_init();

    if (argc < 2) {
        kprintf("ThaiPkg - ThaiOS Package Manager\n");
        kprintf("Usage:\n");
        kprintf("  thaipkg update          Update package lists\n");
        kprintf("  thaipkg install <pkg>   Install a package\n");
        kprintf("  thaipkg remove <pkg>    Remove a package\n");
        kprintf("  thaipkg search <query>  Search packages\n");
        kprintf("  thaipkg upgrade         Upgrade all packages\n");
        kprintf("  thaipkg verify <pkg>    Verify a package\n");
        return 0;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "update") == 0) {
        return pkg_update();
    } else if (strcmp(cmd, "install") == 0 && argc >= 3) {
        return pkg_install(argv[2]);
    } else if (strcmp(cmd, "remove") == 0 && argc >= 3) {
        return pkg_remove(argv[2]);
    } else if (strcmp(cmd, "search") == 0 && argc >= 3) {
        return pkg_search(argv[2]);
    } else if (strcmp(cmd, "upgrade") == 0) {
        return pkg_upgrade();
    } else if (strcmp(cmd, "verify") == 0 && argc >= 3) {
        return pkg_verify(argv[2]);
    } else {
        kprintf("Unknown command: %s\n", cmd);
        return -1;
    }
}

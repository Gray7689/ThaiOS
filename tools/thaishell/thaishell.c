// ThaiShell - ThaiOS Shell & Scripting Language
// ================================================
// Shell interattiva con scripting, pipe, alias, cron,
// autocomplete, colori e plugin.

#include <thaios.h>
#include <mm.h>
#include <sched.h>

#define SHELL_MAX_LINE 4096
#define SHELL_MAX_ARGS 128
#define SHELL_HISTORY_SIZE 100
#define SHELL_MAX_ALIASES 64
#define SHELL_MAX_VARS   256

typedef struct shell_alias {
    char name[64];
    char value[512];
} shell_alias_t;

typedef struct shell_var {
    char name[64];
    char value[1024];
    bool export;
} shell_var_t;

typedef struct shell_state {
    char cwd[4096];
    char prompt[128];
    shell_alias_t aliases[SHELL_MAX_ALIASES];
    int alias_count;
    shell_var_t vars[SHELL_MAX_VARS];
    int var_count;
    char history[SHELL_HISTORY_SIZE][SHELL_MAX_LINE];
    int history_count;
    int history_pos;
    bool running;
    bool interactive;
    int last_exit_code;
} shell_state_t;

static shell_state_t g_shell;

// Built-in commands
static int builtin_cd(int argc, char **argv);
static int builtin_echo(int argc, char **argv);
static int builtin_exit(int argc, char **argv);
static int builtin_alias(int argc, char **argv);
static int builtin_export(int argc, char **argv);
static int builtin_ls(int argc, char **argv);
static int builtin_pwd(int argc, char **argv);
static int builtin_cat(int argc, char **argv);
static int builtin_help(int argc, char **argv);
static int builtin_which(int argc, char **argv);
static int builtin_ps(int argc, char **argv);
static int builtin_kill(int argc, char **argv);
static int builtin_clear(int argc, char **argv);

typedef struct builtin {
    const char *name;
    int (*handler)(int argc, char **argv);
    const char *help;
} builtin_t;

static builtin_t g_builtins[] = {
    {"cd",     builtin_cd,     "Change directory"},
    {"echo",   builtin_echo,   "Print arguments"},
    {"exit",   builtin_exit,   "Exit the shell"},
    {"alias",  builtin_alias,  "Manage aliases"},
    {"export", builtin_export, "Export environment variable"},
    {"ls",     builtin_ls,     "List directory contents"},
    {"pwd",    builtin_pwd,    "Print working directory"},
    {"cat",    builtin_cat,    "Concatenate files"},
    {"help",   builtin_help,   "Show help"},
    {"which",  builtin_which,  "Locate a command"},
    {"ps",     builtin_ps,     "List processes"},
    {"kill",   builtin_kill,   "Kill a process"},
    {"clear",  builtin_clear,  "Clear terminal"},
    {NULL, NULL, NULL}
};

void shell_init(void) {
    memset(&g_shell, 0, sizeof(g_shell));
    strcpy(g_shell.cwd, "/");
    strcpy(g_shell.prompt, "thai> ");
    g_shell.running = true;
    g_shell.interactive = true;

    kprintf("[SHELL] ThaiShell initialized\n");
}

void shell_set_prompt(const char *p) {
    strncpy(g_shell.prompt, p, sizeof(g_shell.prompt) - 1);
}

static void shell_add_history(const char *line) {
    if (g_shell.history_count < SHELL_HISTORY_SIZE) {
        strncpy(g_shell.history[g_shell.history_count], line, SHELL_MAX_LINE - 1);
        g_shell.history_count++;
    } else {
        memmove(&g_shell.history[0], &g_shell.history[1],
                (SHELL_HISTORY_SIZE - 1) * SHELL_MAX_LINE);
        strncpy(g_shell.history[SHELL_HISTORY_SIZE - 1], line, SHELL_MAX_LINE - 1);
    }
}

static void shell_expand_vars(char *line) {
    char result[SHELL_MAX_LINE];
    int ri = 0;

    for (int i = 0; line[i] && ri < SHELL_MAX_LINE - 1; i++) {
        if (line[i] == '$') {
            char varname[64];
            int vi = 0;
            i++;
            if (line[i] == '{') {
                i++;
                while (line[i] && line[i] != '}' && vi < 63)
                    varname[vi++] = line[i++];
            } else {
                while (line[i] && (isalnum(line[i]) || line[i] == '_') && vi < 63)
                    varname[vi++] = line[i++];
                i--;
            }
            varname[vi] = '\0';

            // Look up variable
            char *val = shell_get_var(varname);
            if (val) {
                int vl = strlen(val);
                int copy = (ri + vl < SHELL_MAX_LINE - 1) ? vl : SHELL_MAX_LINE - 1 - ri;
                memcpy(result + ri, val, copy);
                ri += copy;
            }
        } else {
            result[ri++] = line[i];
        }
    }
    result[ri] = '\0';
    strcpy(line, result);
}

static int shell_execute(char *line) {
    // Tokenize
    char *argv[SHELL_MAX_ARGS];
    int argc = 0;
    char *saveptr;

    char *token = strtok_r(line, " \t", &saveptr);
    while (token && argc < SHELL_MAX_ARGS) {
        argv[argc++] = token;
        token = strtok_r(NULL, " \t", &saveptr);
    }

    if (argc == 0) return 0;

    // Check alias
    for (int i = 0; i < g_shell.alias_count; i++) {
        if (strcmp(argv[0], g_shell.aliases[i].name) == 0) {
            // Expand alias
            char expanded[SHELL_MAX_LINE];
            snprintf(expanded, sizeof(expanded), "%s", g_shell.aliases[i].value);
            for (int j = 1; j < argc; j++) {
                strncat(expanded, " ", sizeof(expanded) - strlen(expanded) - 1);
                strncat(expanded, argv[j], sizeof(expanded) - strlen(expanded) - 1);
            }
            return shell_execute(expanded);
        }
    }

    // Check builtins
    for (int i = 0; g_builtins[i].name; i++) {
        if (strcmp(argv[0], g_builtins[i].name) == 0) {
            return g_builtins[i].handler(argc, argv);
        }
    }

    // External command
    kprintf("Unknown command: %s\n", argv[0]);
    kprintf("Try 'help' for available commands\n");
    return -1;
}

void shell_loop(void) {
    char line[SHELL_MAX_LINE];

    while (g_shell.running) {
        kprintf("%s", g_shell.prompt);
        kprintf("> ");  // Visual prompt

        // TODO: readline with autocomplete
        // For now, stub input
        // fgets(line, sizeof(line), stdin);

        // Simulate: read from serial
        int i = 0;
        char c;
        while (1) {
            // c = serial_read_char();  // Stub
            c = '\n';  // Placeholder
            if (c == '\n' || c == '\r') {
                line[i] = '\0';
                break;
            }
            if (i < SHELL_MAX_LINE - 1) {
                line[i++] = c;
            }
        }

        if (strlen(line) == 0) continue;

        shell_add_history(line);
        shell_expand_vars(line);

        int ret = shell_execute(line);
        g_shell.last_exit_code = ret;
    }
}

// Built-in implementations
static int builtin_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        kprintf("%s%c", argv[i], (i < argc - 1) ? ' ' : '\n');
    }
    if (argc == 1) kprintf("\n");
    return 0;
}

static int builtin_exit(int argc, char **argv) {
    g_shell.running = false;
    return 0;
}

static int builtin_help(int argc, char **argv) {
    kprintf("ThaiShell built-in commands:\n");
    for (int i = 0; g_builtins[i].name; i++) {
        kprintf("  %-10s %s\n", g_builtins[i].name, g_builtins[i].help);
    }
    return 0;
}

static int builtin_clear(int argc, char **argv) {
    kprintf("\033[2J\033[H");  // ANSI escape
    return 0;
}

static int builtin_pwd(int argc, char **argv) {
    kprintf("%s\n", g_shell.cwd);
    return 0;
}

static int builtin_cd(int argc, char **argv) {
    if (argc < 2) {
        strcpy(g_shell.cwd, "/");
        return 0;
    }
    // TODO: real path resolution
    strncpy(g_shell.cwd, argv[1], sizeof(g_shell.cwd) - 1);
    return 0;
}

static int builtin_alias(int argc, char **argv) {
    if (argc < 2) {
        for (int i = 0; i < g_shell.alias_count; i++) {
            kprintf("alias %s='%s'\n", g_shell.aliases[i].name, g_shell.aliases[i].value);
        }
        return 0;
    }

    if (argc >= 3) {
        if (g_shell.alias_count < SHELL_MAX_ALIASES) {
            strncpy(g_shell.aliases[g_shell.alias_count].name, argv[1], 63);
            strncpy(g_shell.aliases[g_shell.alias_count].value, argv[2], 511);
            g_shell.alias_count++;
        }
    }
    return 0;
}

static int builtin_export(int argc, char **argv) {
    if (argc < 2) {
        for (int i = 0; i < g_shell.var_count; i++) {
            if (g_shell.vars[i].export) {
                kprintf("export %s=%s\n", g_shell.vars[i].name, g_shell.vars[i].value);
            }
        }
        return 0;
    }

    char *eq = strchr(argv[1], '=');
    if (eq) {
        *eq = '\0';
        shell_set_var(argv[1], eq + 1, true);
    }
    return 0;
}

static int builtin_ls(int argc, char **argv) {
    const char *path = (argc > 1) ? argv[1] : g_shell.cwd;
    // TODO: read directory via VFS
    kprintf(".  ..\n");
    return 0;
}

static int builtin_cat(int argc, char **argv) {
    if (argc < 2) {
        kprintf("Usage: cat <file>\n");
        return -1;
    }
    // TODO: open and read file via VFS
    kprintf("(file content would appear here)\n");
    return 0;
}

static int builtin_which(int argc, char **argv) {
    if (argc < 2) return -1;
    for (int i = 0; g_builtins[i].name; i++) {
        if (strcmp(argv[1], g_builtins[i].name) == 0) {
            kprintf("%s: shell built-in\n", argv[1]);
            return 0;
        }
    }
    kprintf("%s: not found\n", argv[1]);
    return -1;
}

static int builtin_ps(int argc, char **argv) {
    kprintf("  PID  NAME\n");
    // TODO: iterate process list
    return 0;
}

static int builtin_kill(int argc, char **argv) {
    if (argc < 2) return -1;
    // TODO: send signal to process
    kprintf("Killed PID %s (stub)\n", argv[1]);
    return 0;
}

// Variable management
char *shell_get_var(const char *name) {
    for (int i = 0; i < g_shell.var_count; i++) {
        if (strcmp(g_shell.vars[i].name, name) == 0) {
            return g_shell.vars[i].value;
        }
    }
    return NULL;
}

void shell_set_var(const char *name, const char *value, bool export_var) {
    for (int i = 0; i < g_shell.var_count; i++) {
        if (strcmp(g_shell.vars[i].name, name) == 0) {
            strncpy(g_shell.vars[i].value, value, sizeof(g_shell.vars[i].value) - 1);
            g_shell.vars[i].export = export_var;
            return;
        }
    }
    if (g_shell.var_count < SHELL_MAX_VARS) {
        strncpy(g_shell.vars[g_shell.var_count].name, name, 63);
        strncpy(g_shell.vars[g_shell.var_count].value, value, 1023);
        g_shell.vars[g_shell.var_count].export = export_var;
        g_shell.var_count++;
    }
}

// ThaiOS Virtual File System
// ============================
// Layer di astrazione per file system multipli.
// Supporta mount, open, read, write, close, ioctl, readdir.

#include <thaios.h>
#include <mm.h>
#include <sched.h>

#define VFS_MAX_FILES   4096
#define VFS_MAX_MOUNTS  64
#define VFS_MAX_PATH    4096
#define VFS_MAX_FS      16

typedef enum file_type {
    FILE_UNKNOWN,
    FILE_REGULAR,
    FILE_DIRECTORY,
    FILE_BLOCK,
    FILE_CHAR,
    FILE_SYMLINK,
    FILE_PIPE
} file_type_t;

typedef struct inode {
    u64 ino;
    file_type_t type;
    u64 size;
    u32 uid;
    u32 gid;
    u16 mode;
    u64 mtime;
    u64 ctime;
    void *fs_priv;          // Dati specifici del FS
} inode_t;

typedef struct file_operations {
    int (*open)(inode_t *inode);
    int (*close)(inode_t *inode);
    isize (*read)(inode_t *inode, void *buf, usize count, u64 offset);
    isize (*write)(inode_t *inode, const void *buf, usize count, u64 offset);
    int (*ioctl)(inode_t *inode, u64 request, void *arg);
    int (*readdir)(inode_t *inode, u32 index, char *name, u32 name_size);
    int (*truncate)(inode_t *inode, u64 size);
    int (*sync)(inode_t *inode);
} file_operations_t;

typedef struct filesystem {
    char name[32];
    int (*mount)(struct filesystem *fs, const char *source, const char *target);
    int (*unmount)(struct filesystem *fs);
    inode_t *(*getroot)(struct filesystem *fs);
    file_operations_t *ops;
    struct filesystem *next;
} filesystem_t;

typedef struct file_descriptor {
    inode_t *inode;
    u64 flags;
    u64 offset;
    struct thread *owner;
    bool used;
} file_descriptor_t;

typedef struct mount_point {
    char target[VFS_MAX_PATH];
    filesystem_t *fs;
    inode_t *root;
    struct mount_point *next;
} mount_point_t;

static filesystem_t *g_filesystems = NULL;
static mount_point_t *g_mounts = NULL;
static file_descriptor_t g_fds[VFS_MAX_FILES];

void vfs_init(void) {
    for (int i = 0; i < VFS_MAX_FILES; i++) {
        g_fds[i].used = false;
    }
    kprintf("[VFS] Virtual File System initialized\n");
}

int vfs_register_filesystem(filesystem_t *fs) {
    fs->next = g_filesystems;
    g_filesystems = fs;
    kprintf("[VFS] Registered filesystem: %s\n", fs->name);
    return SUCCESS;
}

int vfs_mount(const char *source, const char *target, const char *fstype) {
    filesystem_t *fs = g_filesystems;
    while (fs) {
        if (strcmp(fs->name, fstype) == 0) {
            mount_point_t *mp = (mount_point_t*)kmalloc(sizeof(mount_point_t));
            if (!mp) return -ENOMEM;

            strncpy(mp->target, target, VFS_MAX_PATH - 1);
            mp->fs = fs;
            mp->root = fs->getroot(fs);
            mp->next = g_mounts;
            g_mounts = mp;

            kprintf("[VFS] Mounted %s on %s (%s)\n", source, target, fstype);
            return SUCCESS;
        }
        fs = fs->next;
    }
    return -ENOTSUP;
}

int vfs_open(const char *path, u64 flags) {
    // Find mount point
    mount_point_t *mp = g_mounts;
    while (mp) {
        if (strncmp(path, mp->target, strlen(mp->target)) == 0) {
            // Found mount point, delegate to FS
            if (mp->fs && mp->fs->ops && mp->fs->ops->open) {
                int ret = mp->fs->ops->open(mp->root);
                if (ret < 0) return ret;

                // Alloca FD
                for (int i = 0; i < VFS_MAX_FILES; i++) {
                    if (!g_fds[i].used) {
                        g_fds[i].used = true;
                        g_fds[i].inode = mp->root;
                        g_fds[i].flags = flags;
                        g_fds[i].offset = 0;
                        g_fds[i].owner = sched_current();
                        return i;
                    }
                }
            }
            return -ENFILE;
        }
        mp = mp->next;
    }
    return -ENOENT;
}

int vfs_close(int fd) {
    if (fd < 0 || fd >= VFS_MAX_FILES || !g_fds[fd].used) return -EBADF;
    if (g_fds[fd].inode && g_fds[fd].inode->fs_priv) {
        // Close filesystem specific
    }
    g_fds[fd].used = false;
    return SUCCESS;
}

isize vfs_read(int fd, void *buf, usize count) {
    if (fd < 0 || fd >= VFS_MAX_FILES || !g_fds[fd].used) return -EBADF;
    file_descriptor_t *fd_ent = &g_fds[fd];

    if (fd_ent->inode && fd_ent->inode->fs_priv) {
        // TODO: dispatch to FS read
        memset(buf, 0, count);
        return count;
    }
    return -EINVAL;
}

isize vfs_write(int fd, const void *buf, usize count) {
    if (fd < 0 || fd >= VFS_MAX_FILES || !g_fds[fd].used) return -EBADF;
    return count;  // Stub
}

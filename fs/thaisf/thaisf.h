// ThaiFS - ThaiOS Native File System
// =====================================
// File system proprietario con B-tree, checksum, compressione,
// deduplicazione, snapshot e crittografia nativa.

#ifndef _THAIFS_H
#define _THAIFS_H

#include <thaios.h>

#define THAIFS_MAGIC      0x5448414950530001  // "THAIPS\0\1"
#define THAIFS_BLOCK_SIZE 4096
#define THAIFS_MAX_NAME   256
#define THAIFS_MAX_FILE   (16ULL * 1024 * 1024 * 1024)  // 16TB

typedef struct thaisf_superblock {
    u64 magic;
    u64 version;
    u64 block_size;
    u64 num_blocks;
    u64 root_inode;
    u64 free_block_bitmap;
    u64 journal_start;
    u64 journal_size;
    u64 checksum;               // CRC64 del superblock
    u8  uuid[16];
    u8  label[64];
    u64 flags;                  // Encryption, compression flags
    u64 snapshot_root;
    u64 feature_flags;          // Supported features bitmap
    u64 created_at;
    u64 mount_count;
} __attribute__((packed)) thaisf_superblock_t;

typedef struct thaisf_inode {
    u64 ino;
    u16 mode;
    u32 uid;
    u32 gid;
    u64 size;
    u64 atime;
    u64 mtime;
    u64 ctime;
    u64 blocks;                 // Number of 4K blocks
    u32 blocks_checksum[16];    // Checksum per block extent
    u64 extent_tree_root;       // B-tree root for extents
    u64 xattr_tree_root;        // Extended attributes B-tree
    u64 refcount;               // Per COW
    u64 encryption_key_id;      // Per-file encryption key reference
    u8  compression_algo;       // 0=none, 1=zstd, 2=lz4
    u8  reserved[47];
} __attribute__((packed)) thaisf_inode_t;

typedef struct thaisf_extent {
    u64 logical_block;
    u64 physical_block;
    u64 length;
    u32 checksum;
    u16 flags;                  // Compressed, encrypted, etc
} __attribute__((packed)) thaisf_extent_t;

typedef struct thaisf_dirent {
    u64 inode;
    u8  file_type;
    u8  name_len;
    u16 name_hash;
    char name[THAIFS_MAX_NAME];
} __attribute__((packed)) thaisf_dirent_t;

// ThaiFS operations
int thaisf_mount(const char *device, const char *mountpoint);
int thaisf_unmount(void);
int thaisf_create(const char *path, u16 mode);
int thaisf_read(const char *path, void *buf, usize count, u64 offset);
int thaisf_write(const char *path, const void *buf, usize count, u64 offset);
int thaisf_mkdir(const char *path, u16 mode);
int thaisf_unlink(const char *path);
int thaisf_symlink(const char *target, const char *linkpath);
int thaisf_readdir(const char *path, u32 index, char *name, u32 *type);
int thaisf_stat(const char *path, thaisf_inode_t *stat);
int thaisf_snapshot_create(const char *name);
int thaisf_snapshot_restore(const char *name);
int thaisf_check(void);         // fsck

#endif

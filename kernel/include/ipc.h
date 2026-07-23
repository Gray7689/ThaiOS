#ifndef _IPC_H
#define _IPC_H

#include <thaios.h>

#define IPC_MAX_MSG_SIZE 4096
#define IPC_MAX_QUEUES   1024

typedef enum ipc_msg_type {
    IPC_EMPTY,
    IPC_DATA,
    IPC_SIGNAL,
    IPC_REQUEST,
    IPC_REPLY
} ipc_msg_type_t;

typedef struct ipc_message {
    ipc_msg_type_t type;
    u64 sender;
    u64 target;
    usize size;
    u8 data[IPC_MAX_MSG_SIZE];
} ipc_message_t;

typedef struct ipc_queue {
    u64 id;
    struct ipc_queue *next;
    ipc_message_t *msgs;
    usize count;
    usize capacity;
    struct thread *waiting_thread;
} ipc_queue_t;

typedef struct shared_mem {
    u64 id;
    vaddr_t addr;
    usize size;
    u64 flags;
    usize refcount;
} shared_mem_t;

void ipc_init(void);
int ipc_queue_create(u64 id);
int ipc_queue_destroy(u64 id);
int ipc_send(u64 target_queue, ipc_message_t *msg);
int ipc_recv(u64 queue_id, ipc_message_t *msg);
int ipc_send_recv(u64 target_queue, ipc_message_t *msg, ipc_message_t *reply);

int shm_create(usize size, u64 flags);
int shm_attach(int shm_id, vaddr_t *addr);
int shm_detach(vaddr_t addr);
int shm_destroy(int shm_id);

#endif

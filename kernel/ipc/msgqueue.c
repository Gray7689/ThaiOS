// ThaiOS IPC - Message Queues
// =============================
// Message passing tra processi/thread con buffering circolare.

#include <thaios.h>
#include <ipc.h>
#include <sched.h>
#include <mm.h>

static ipc_queue_t *g_queue_head = NULL;
static usize g_queue_count = 0;

void ipc_init(void) {
    g_queue_head = NULL;
    g_queue_count = 0;
    kprintf("[IPC] Message queue system initialized\n");
}

int ipc_queue_create(u64 id) {
    // Controlla se esiste già
    ipc_queue_t *q = g_queue_head;
    while (q) {
        if (q->id == id) return -EINVAL;
        q = q->next;
    }

    ipc_queue_t *queue = (ipc_queue_t*)kmalloc(sizeof(ipc_queue_t));
    if (!queue) return -ENOMEM;

    queue->id = id;
    queue->capacity = 64;
    queue->count = 0;
    queue->next = g_queue_head;
    queue->waiting_thread = NULL;

    queue->msgs = (ipc_message_t*)kcalloc(queue->capacity, sizeof(ipc_message_t));
    if (!queue->msgs) {
        kfree(queue);
        return -ENOMEM;
    }

    g_queue_head = queue;
    g_queue_count++;
    return SUCCESS;
}

int ipc_queue_destroy(u64 id) {
    ipc_queue_t *prev = NULL;
    ipc_queue_t *curr = g_queue_head;

    while (curr) {
        if (curr->id == id) {
            if (prev) prev->next = curr->next;
            else g_queue_head = curr->next;

            if (curr->waiting_thread) {
                curr->waiting_thread->state = THREAD_READY;
                sched_add(curr->waiting_thread);
            }

            kfree(curr->msgs);
            kfree(curr);
            g_queue_count--;
            return SUCCESS;
        }
        prev = curr;
        curr = curr->next;
    }

    return -EINVAL;
}

int ipc_send(u64 target_queue, ipc_message_t *msg) {
    ipc_queue_t *q = g_queue_head;
    while (q && q->id != target_queue) q = q->next;
    if (!q) return -EINVAL;

    if (q->count >= q->capacity) {
        // TODO: blocco o buffer esteso
        return -ENOMEM;
    }

    q->msgs[q->count++] = *msg;

    // Risveglia thread in attesa
    if (q->waiting_thread) {
        q->waiting_thread->state = THREAD_READY;
        sched_add(q->waiting_thread);
        q->waiting_thread = NULL;
    }

    return SUCCESS;
}

int ipc_recv(u64 queue_id, ipc_message_t *msg) {
    ipc_queue_t *q = g_queue_head;
    while (q && q->id != queue_id) q = q->next;
    if (!q) return -EINVAL;

    if (q->count == 0) {
        q->waiting_thread = sched_current();
        sched_current()->state = THREAD_BLOCKED;
        sched_remove(sched_current());
        sched_yield();
    }

    if (q->count > 0) {
        *msg = q->msgs[0];
        for (usize i = 1; i < q->count; i++) {
            q->msgs[i - 1] = q->msgs[i];
        }
        q->count--;
        return SUCCESS;
    }

    return -1;
}

int ipc_send_recv(u64 target_queue, ipc_message_t *msg, ipc_message_t *reply) {
    int ret = ipc_send(target_queue, msg);
    if (ret < 0) return ret;

    u64 reply_queue = msg->sender;
    return ipc_recv(reply_queue, reply);
}

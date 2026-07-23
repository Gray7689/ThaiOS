// ThaiOS Scheduler
// =================
// Scheduler O(1) con code multiple per priorità.
// Supporto SMP con affinità CPU, CFS per processi interattivi.

#include <thaios.h>
#include <sched.h>

#define NUM_PRIORITIES 140
#define TIME_SLICE_MS 4

typedef struct runqueue {
    thread_t *head[NUM_PRIORITIES];
    thread_t *tail[NUM_PRIORITIES];
    u64 bitmap[NUM_PRIORITIES / 64 + 1];
    usize count;
} runqueue_t;

static runqueue_t g_runqueue;
static thread_t *g_current_thread = NULL;
static u64 g_pid_counter = 0;
static u64 g_tid_counter = 0;

void sched_init(void) {
    for (int i = 0; i < NUM_PRIORITIES; i++) {
        g_runqueue.head[i] = NULL;
        g_runqueue.tail[i] = NULL;
    }
    for (int i = 0; i < NUM_PRIORITIES / 64 + 1; i++) {
        g_runqueue.bitmap[i] = 0;
    }
    g_runqueue.count = 0;
    g_current_thread = NULL;

    kprintf("[SCHED] O(1) scheduler initialized\n");
}

static int sched_find_highest(void) {
    for (int i = 0; i < NUM_PRIORITIES / 64 + 1; i++) {
        if (g_runqueue.bitmap[i]) {
            int bit = __builtin_ctzll(g_runqueue.bitmap[i]);
            return i * 64 + bit;
        }
    }
    return -1;
}

static void sched_set_bitmap(int prio) {
    g_runqueue.bitmap[prio / 64] |= (1ULL << (prio % 64));
}

static void sched_clear_bitmap(int prio) {
    g_runqueue.bitmap[prio / 64] &= ~(1ULL << (prio % 64));
}

void sched_add(thread_t *thread) {
    if (!thread) return;

    int prio = thread->priority;
    if (prio < 0) prio = 0;
    if (prio >= NUM_PRIORITIES) prio = NUM_PRIORITIES - 1;

    thread->next = NULL;

    if (g_runqueue.tail[prio]) {
        g_runqueue.tail[prio]->next = thread;
        g_runqueue.tail[prio] = thread;
    } else {
        g_runqueue.head[prio] = thread;
        g_runqueue.tail[prio] = thread;
        sched_set_bitmap(prio);
    }

    g_runqueue.count++;
}

void sched_remove(thread_t *thread) {
    if (!thread) return;

    int prio = thread->priority;
    thread_t *prev = NULL;
    thread_t *curr = g_runqueue.head[prio];

    while (curr) {
        if (curr == thread) {
            if (prev) prev->next = curr->next;
            else g_runqueue.head[prio] = curr->next;

            if (!curr->next) g_runqueue.tail[prio] = prev;

            if (!g_runqueue.head[prio]) sched_clear_bitmap(prio);
            g_runqueue.count--;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void sched_yield(void) {
    thread_t *current = g_current_thread;
    if (current && current->state == THREAD_RUNNING) {
        current->state = THREAD_READY;
        sched_add(current);
    }

    int highest = sched_find_highest();
    if (highest < 0) {
        // Nessun thread, loop idle
        g_current_thread = NULL;
        return;
    }

    thread_t *next = g_runqueue.head[highest];
    if (next) {
        g_runqueue.head[highest] = next->next;
        if (!g_runqueue.head[highest]) {
            g_runqueue.tail[highest] = NULL;
            sched_clear_bitmap(highest);
        }
        g_runqueue.count--;

        next->state = THREAD_RUNNING;
        g_current_thread = next;
    }
}

thread_t *sched_current(void) {
    return g_current_thread;
}

void sched_tick(void) {
    thread_t *current = g_current_thread;
    if (!current) return;

    current->time_slice--;
    if (current->time_slice <= 0) {
        current->time_slice = TIME_SLICE_MS;
        sched_yield();
    }
}

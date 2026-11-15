#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(demo, LOG_LEVEL_DBG);

#define STACK_SIZE 1024

#define PRIO_THREAD 1
#define INCREMENTS 1000000

struct k_sem done_sem;
struct k_sem done_mutex_sem;
static K_MUTEX_DEFINE(counter_mutex);

static volatile uint32_t counter = 0;
static volatile uint32_t counter_with_mutex = 0;

void worker_thread(void *p1, void *p2, void *p3)
{
    const char *name = k_thread_name_get(k_current_get());
    for (int i = 0; i < INCREMENTS; i++) {
        counter++;
    }

    LOG_INF("[%s] finished", name);
    k_sem_give(&done_sem);
}

void worker_thread_with_mutex(void *p1, void *p2, void *p3)
{
    const char *name = k_thread_name_get(k_current_get());
    for (int i = 0; i < INCREMENTS; i++) {
        k_mutex_lock(&counter_mutex, K_FOREVER);
        counter_with_mutex++;
        k_mutex_unlock(&counter_mutex);
    }

    LOG_INF("[%s] finished", name);
    k_sem_give(&done_mutex_sem);
}

K_THREAD_DEFINE(worker_a, STACK_SIZE, worker_thread_with_mutex,
                NULL, NULL, NULL, PRIO_THREAD, 0, 0);
K_THREAD_DEFINE(worker_b, STACK_SIZE, worker_thread_with_mutex,
                NULL, NULL, NULL, PRIO_THREAD, 0, 0);
K_THREAD_DEFINE(worker_aa, STACK_SIZE, worker_thread,
                NULL, NULL, NULL, PRIO_THREAD, 0, 0);   
K_THREAD_DEFINE(worker_bb, STACK_SIZE, worker_thread,
                NULL, NULL, NULL, PRIO_THREAD, 0, 0);

int main(void)
{
    LOG_INF("Zephyr Thread Mutex Demo");
    int64_t time = k_uptime_get();

    k_sem_init(&done_mutex_sem, 0, 2);
    k_sem_init(&done_sem, 0, 2);
    k_sem_take(&done_mutex_sem, K_FOREVER);
    k_sem_take(&done_mutex_sem, K_FOREVER);
    k_sem_take(&done_sem, K_FOREVER);
    k_sem_take(&done_sem, K_FOREVER);

    if(counter_with_mutex != 2 * INCREMENTS) {
        LOG_ERR("Counter value is incorrect: %d", counter_with_mutex);
    } else {
        LOG_INF("[WITH MUTEX] Counter value is correct: %d", counter_with_mutex);
    }
    if(counter != 2 * INCREMENTS) {
        LOG_ERR("[NO MUTEX  ] Counter value is incorrect: %d", counter);
    } else {
        LOG_INF("[NO MUTEX  ] Counter value is correct: %d", counter);
    }

    /**
     * Why using mutex prevents the counter from being corrupted?
     * because the mutex ensures that only one thread can access 
     * the critical section of code (where the counter is incremented) at a time. 
     * When a thread locks the mutex, other threads attempting to lock it will 
     * be blocked until the mutex is unlocked. This prevents race conditions and 
     * ensures that the counter is updated correctly without interference from 
     * other threads.
     */
    if(counter != counter_with_mutex) {
        LOG_ERR("[WITH_MUTEX] %d != %d [NO_MUTEX  ]", counter_with_mutex, counter);
    }
    LOG_INF("Execution time: %lld ms", k_uptime_delta(&time));
    return 0;
}


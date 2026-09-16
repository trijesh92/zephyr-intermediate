#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include<zephyr/task_wdt/task_wdt.h>

LOG_MODULE_REGISTER(demo, LOG_LEVEL_DBG);

#define STACK_SIZE 1024

static int chan = -1;

/* ================================================================== */
/*  Shared message type                                               */
/* ================================================================== */

struct sensor_msg {
    uint32_t timestamp_ms;
    int32_t value;
    uint32_t seq;
};

/* ================================================================== */
/*  Thread-to-thread pipeline                                */
/* ================================================================== */

#define QUEUE_DEPTH 50
#define COUNT       400

K_MSGQ_DEFINE(sensor_msg_q, sizeof(struct sensor_msg), QUEUE_DEPTH, 4);

static K_SEM_DEFINE(prod_done, 0, 1);
static K_SEM_DEFINE(cons_done, 0, 1);

static volatile bool prod_finished;

static void producer(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    k_thread_name_set(k_current_get(), "prod");

    for (int i = 0; i < COUNT; i++) {
        struct sensor_msg msg = {
            .timestamp_ms = k_uptime_get_32(),
            .value = 100 + i,
            .seq = (uint32_t)i,
        };

        int ret = k_msgq_put(&sensor_msg_q, &msg, K_MSEC(200));
        if (ret == 0) {
            LOG_INF("[PROD] sent seq=%u val=%d q=%u/%d",
                    msg.seq,
                    msg.value,
                    k_msgq_num_used_get(&sensor_msg_q),
                    QUEUE_DEPTH);
        } else {
            LOG_WRN("[PROD] put failed ret=%d", ret);
        }

        k_msleep(50);
    }

    prod_finished = true;
    LOG_INF("[PROD] done");
    k_sem_give(&prod_done);
}

static void consumer(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    k_thread_name_set(k_current_get(), "cons");

    while (true) {
        struct sensor_msg msg = {0};

        int ret = k_msgq_get(&sensor_msg_q, &msg, K_MSEC(300));
        if (ret != 0) {
            if (prod_finished && k_msgq_num_used_get(&sensor_msg_q) == 0) {
                break;
            }

            LOG_WRN("[CONS] timeout waiting for message");
            continue;
        }

        uint32_t latency = k_uptime_get_32() - msg.timestamp_ms;

        LOG_INF("[CONS] got seq=%u val=%d q=%u/%d latency=%ums",
                msg.seq,
                msg.value,
                k_msgq_num_used_get(&sensor_msg_q),
                QUEUE_DEPTH,
                latency);
        
        task_wdt_feed(chan);
        if (msg.seq == COUNT/2) {
            k_msleep(1500);
        }
        k_msleep(60);
    }

    LOG_INF("[CONS] done");
    k_sem_give(&cons_done);
}

/* ================================================================== */
/*  Runtime threads                                                   */
/* ================================================================== */

K_THREAD_STACK_DEFINE(prod_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(cons_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(health_stack, STACK_SIZE);

static struct k_thread prod_thread;
static struct k_thread cons_thread;
static struct k_thread health_thread;
/* ================================================================== */
/*  Watchdog task                                                     */
/* ================================================================== */
void watchdog_timeout_callback(int channelId, void *arg)
{
    k_tid_t thread = (k_tid_t)arg;
    LOG_ERR("[WATCHDOG] (channel %d) Thread %p (%s) has exceeded its watchdog timeout!", channelId, thread, k_thread_name_get(thread));
}

/* ================================================================== */
/*  Health monitor                                                    */
/* ================================================================== */

static void health_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    while(true) {
        uint32_t used = k_msgq_num_used_get(&sensor_msg_q);
        if(used > (QUEUE_DEPTH * 3 / 4)) {
            LOG_WRN("[HEALTH] queue is %u/%u full", used, QUEUE_DEPTH);
        }
        k_msleep(120);
    }
}

/* ================================================================== */
/*  Main                                                              */
/* ================================================================== */

int main(void)
{
    LOG_INF("=== L5 Task1 ===");
    LOG_INF("sizeof(sensor_msg)=%u", sizeof(struct sensor_msg));

    k_msgq_purge(&sensor_msg_q);
    prod_finished = false;

    k_thread_create(&prod_thread, prod_stack,
                    K_THREAD_STACK_SIZEOF(prod_stack), producer, 
                    NULL, NULL, NULL, 5, 0, K_NO_WAIT);

    /*
    * Let producer get ahead.
    * This makes the queue buffer real messages instead of direct handoff.
    */
    k_msleep(180);

    k_thread_create(&cons_thread,
                    cons_stack,
                    K_THREAD_STACK_SIZEOF(cons_stack), consumer,
                    NULL, NULL, NULL, 5, 0, K_NO_WAIT);

    k_thread_create(&health_thread,
                health_stack,
                K_THREAD_STACK_SIZEOF(health_stack), health_fn,
                NULL, NULL, NULL, 6, 0, K_NO_WAIT);

    task_wdt_init(NULL);
    chan = task_wdt_add(1000,watchdog_timeout_callback,(void *)k_current_get());
    

    k_sem_take(&prod_done, K_FOREVER);
    k_sem_take(&cons_done, K_FOREVER);

    LOG_INF("\n=== Demo complete ===");

    return 0;
}


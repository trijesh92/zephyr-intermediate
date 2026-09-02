#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(demo, LOG_LEVEL_DBG);

#define STACK_SIZE 1024

#define PRIO_COP -1
#define PRIO_LOW 7
#define PRIO_MED 5
#define PRIO_HIGH 3

#define T_PRIO_LOW_SLEEP 300
#define T_PRIO_MED_SLEEP 200
#define T_PRIO_HIGH_SLEEP 100

void t_cop_fn(void *p1, void *p2, void *p3)
{
    for(uint8_t i=0;i<5;i++)
    {
        LOG_INF("T_COP  step=%d tick=%d", i, k_uptime_get_32());
    }
    k_yield();
}

void t_low_fn(void *p1, void *p2, void *p3)
{
    uint8_t i = 0;
    while (1) {
        LOG_INF("T_LOW  step=%d tick=%d", i, k_uptime_get_32());
        k_msleep(T_PRIO_LOW_SLEEP);
        i++;
    }
}

void t_med_fn(void *p1, void *p2, void *p3)
{
    uint8_t i = 0;
    while (1) {
        LOG_INF("T_MED  step=%d tick=%d", i, k_uptime_get_32());
        k_msleep(T_PRIO_MED_SLEEP);
        i++;
    }
}

void t_high_fn(void *p1, void *p2, void *p3)
{
    uint8_t i = 0;
    while (1) {
        LOG_INF("T_HIGH step=%d tick=%d", i, k_uptime_get_32());
        k_msleep(T_PRIO_HIGH_SLEEP);
        i++;
    }
}

K_THREAD_DEFINE(thread_low, STACK_SIZE, t_low_fn,
                NULL, NULL, NULL, PRIO_LOW, 0, 0);
K_THREAD_DEFINE(thread_med, STACK_SIZE, t_med_fn,
                NULL, NULL, NULL, PRIO_MED, 0, 0);
K_THREAD_DEFINE(thread_high, STACK_SIZE, t_high_fn,
                NULL, NULL, NULL, PRIO_HIGH, 0, 0);
K_THREAD_DEFINE(thread_cop, STACK_SIZE, t_cop_fn,
                NULL, NULL, NULL, PRIO_COP, 0, 0);

int main(void)
{
    LOG_INF("Zephyr Thread Scheduling Demo");
    return 0;
}


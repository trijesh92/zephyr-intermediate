/*
 * Lecture 5 - Homework Starter Code
 * ================================================================
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <stdbool.h>
#include <zephyr/zbus/zbus.h>

LOG_MODULE_REGISTER(MAIN, LOG_LEVEL_DBG);

#define STACK_SIZE    1024
#define SENSOR_MS     100
#define LOGGER_MS     400
#define SAMPLE_COUNT  50

struct sensor_sample {
    uint32_t seq;
    int32_t temperature;
    uint32_t timestamp_ms;
};

K_SEM_DEFINE(demo_done, 0, 2);

ZBUS_CHAN_DEFINE(result_chan, struct sensor_sample,
                 NULL, NULL, ZBUS_OBSERVERS(display_lis, logger_sub),
                 ZBUS_MSG_INIT(
                     .seq = 0,
                     .temperature = 0,
                     .timestamp_ms = 0));

static void display_callback(const struct zbus_channel *chan);
ZBUS_LISTENER_DEFINE(display_lis, display_callback);
ZBUS_MSG_SUBSCRIBER_DEFINE(logger_sub);

static void fake_sensor(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    for (uint32_t seq = 0; seq < SAMPLE_COUNT; seq++) {
        struct sensor_sample sample = {
            .seq = seq,
            .temperature = 40 + (int32_t)seq,
            .timestamp_ms = k_uptime_get_32(),
        };

        LOG_INF("[SENSOR] acquired seq=%u temperature=%d timestamp=%u  current_tick=%u",
            sample.seq, sample.temperature, sample.timestamp_ms, k_uptime_get_32());

        int ret = zbus_chan_pub(&result_chan, &sample, K_MSEC(100));

        if (ret != 0) {
            LOG_WRN("[SENSOR] publish failed: %d", ret);
        }

        k_msleep(SENSOR_MS);
    }

    LOG_INF("[SENSOR] done");
    k_sem_give(&demo_done);
}

static void display_callback(const struct zbus_channel *chan)
{
    const struct sensor_sample *result = zbus_chan_const_msg(chan);

    LOG_INF("[DISPLAY] seq=%u value=%d timestamp=%ums current_tick=%u", 
        result->seq, result->temperature, result->timestamp_ms, k_uptime_get_32());
}

static void sensor_logging_callback(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);
    const struct zbus_channel *chan;
    uint32_t received = 0;
    while(received < SAMPLE_COUNT)
    {
        struct sensor_sample msg;
        int ret = zbus_sub_wait_msg(&logger_sub, &chan, &msg, K_MSEC(1500));
        if (ret != 0) {
            LOG_WRN("[LOGGER] timeout ret=%d", ret);
            break;
        }
        LOG_INF("[LOGGER] seq=%u value=%d timestamp=%ums current_tick=%u", 
            msg.seq, msg.temperature, msg.timestamp_ms, k_uptime_get_32());
        k_msleep(LOGGER_MS);
        received++;
    }
    k_sem_give(&demo_done);
}

K_THREAD_DEFINE(fake_sensor_thread, STACK_SIZE, fake_sensor,
                NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(sensor_logging_thread, STACK_SIZE, sensor_logging_callback,
                NULL, NULL, NULL, 6, 0, 0);

int main(void)
{
    LOG_INF("=== L5 Homework: Publisher-Subscriber ===");
    
    k_sem_take(&demo_done, K_FOREVER);
    k_sem_take(&demo_done, K_FOREVER);
    LOG_INF("done!");
    return 0;
}
    
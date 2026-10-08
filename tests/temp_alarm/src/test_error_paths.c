/*
 * temp_alarm - Error-path unit tests (homework skeleton)
 *
 * Task 2: fill in the stubs according to TEST_SPEC.md. Each stub calls
 * ztest_test_skip(), so the binary builds and runs cleanly until the test is
 * implemented.
 *
 * Run:
 *   west twister -T tests/temp_alarm -p native_sim
 */

#include <errno.h>
#include <zephyr/fff.h>
#include <zephyr/ztest.h>

#include "sensor_fake.h"
#include "temp_alarm.h"

#define DEFAULT_THRESHOLD 30

/* Same before() hook as in test_basic.c: a clean fake and an initialised module. */
static void before(void *fixture)
{
	ARG_UNUSED(fixture);

	sensor_fake_reset();
	sensor_fake_init();

	zassume_ok(temp_alarm_init(&fake_sensor_dev, DEFAULT_THRESHOLD),
		   "precondition: temp_alarm_init must succeed");
}

/*
 * ============================================================================
 * Test Suite: temp_alarm_sensor_errors
 *
 * Failures from the sensor driver, injected through the fakes.
 * ============================================================================
 */
ZTEST_SUITE(temp_alarm_sensor_errors, NULL, NULL, before, NULL, NULL);

ZTEST(temp_alarm_sensor_errors, test_fetch_error_propagates)
{
	fake_sensor_sample_fetch_fake.return_val = -EIO;

	zassert_equal(temp_alarm_check(), -EIO,
		      "fetch error must be propagated");
	zassert_equal(fake_sensor_channel_get_fake.call_count, 0,
		      "channel must not be read after a fetch error");
}

ZTEST(temp_alarm_sensor_errors, test_channel_get_error_propagates)
{
	fake_sensor_sample_fetch_fake.return_val = 0;
	/* Use the configured fake return value rather than a custom callback. */
	fake_sensor_channel_get_fake.custom_fake = NULL;
	fake_sensor_channel_get_fake.return_val = -EIO;

	zassert_equal(temp_alarm_check(), -EIO,
		      "channel read error must be propagated");
	zassert_equal(fake_sensor_sample_fetch_fake.call_count, 1,
		      "the sensor must be sampled before the channel read");
}

ZTEST(temp_alarm_sensor_errors, test_fail_then_recover)
{
	fake_sensor_sample_fetch_fake.return_val = -EIO;
	zassert_equal(temp_alarm_check(), -EIO,
		      "the first fetch error must be propagated");
	zassert_equal(fake_sensor_sample_fetch_fake.call_count, 1,
		      "the first check must fetch once");

	fake_sensor_sample_fetch_fake.return_val = 0;
	zassert_equal(temp_alarm_check(), 0,
		      "the module must recover when the next fetch succeeds");
	zassert_equal(fake_sensor_sample_fetch_fake.call_count, 2,
		      "the second check must fetch again");
	zassert_equal(fake_sensor_channel_get_fake.call_count, 1,
		      "the channel must be read after the successful fetch");
}

/*
 * ============================================================================
 * Test Suite: temp_alarm_invalid_input
 *
 * Arguments the public API must reject.
 * ============================================================================
 */
ZTEST_SUITE(temp_alarm_invalid_input, NULL, NULL, before, NULL, NULL);

ZTEST(temp_alarm_invalid_input, test_get_status_null)
{
	zassert_equal(temp_alarm_get_status(NULL), -EINVAL,
		      "a NULL status pointer must be rejected");
}

ZTEST(temp_alarm_invalid_input, test_init_without_device)
{
	zassert_equal(temp_alarm_init(NULL, DEFAULT_THRESHOLD), -ENODEV,
		      "init must reject a NULL sensor device");
}

ZTEST(temp_alarm_invalid_input, test_init_threshold_out_of_range)
{
	zassert_equal(temp_alarm_init(&fake_sensor_dev, -1), -EINVAL,
		      "init must reject a threshold below the allowed range");
	zassert_equal(temp_alarm_init(&fake_sensor_dev, 101), -EINVAL,
		      "init must reject a threshold above the allowed range");
}

ZTEST(temp_alarm_invalid_input, test_set_threshold_out_of_range)
{
	zassert_equal(temp_alarm_set_threshold(-1), -EINVAL,
		      "set_threshold must reject a threshold below the allowed range");
	zassert_equal(temp_alarm_set_threshold(101), -EINVAL,
		      "set_threshold must reject a threshold above the allowed range");
}

#include "settings.h"
#include "global.h"
//owner header
#include "Task.h"

static uint32_t lastCheckChargerOnlineTime   = 0;
static uint32_t lastCheckFuelGaugeOnlineTime = 0;

void TaskDebug(const char *str, uint32_t time)
{
    if (TaskDebugFlag) {
        LOG_INFO("%s %d", str, time);
    }
}

void checkBatteryLevels()
{
    if (online_devices.bq40z50 == false)
        return;

    batteryVoltage = (bq40z50.bq40z50getVoltageMv() / 1000.0);

    batteryLevelPercent = bq40z50.bq40z50getRelativeStateOfCharge(true);

    batteryTempC = bq40z50.bq40z50getTemperatureC();

    batteryChargingPercentPerHour =
        (float)bq40z50.bq40z50getAverageCurrentMa() / bq40z50.bq40z50getFullChargeCapacityMah() * 100.0;

    if (batteryLevelPercent == 0.0) {
        batteryLevelPercent = 50.0;
    } else if (batteryLevelPercent <= 99.80 && batteryVoltage >= 8.38) {
        batteryLevelPercent = 100.0;
    }
    return;
}

void handleChargerTask()
{
    if (online_devices.mp2762) {
        if (millis() - lastCheckChargerOnlineTime > 30 * 1000) {
            if (mp2762.isPresent()) {
                online_devices.mp2762 = true;
                TaskDebug("mp2762 is still online-", millis());
            } else {
                online_devices.mp2762 = false;
                TaskDebug("mp2762 is offline first time-", millis());
            }
            lastCheckChargerOnlineTime = millis();
        }
        mp2762.mp2762getChargeStatus();
    } else {
        I2cEnd();
        I2cBegin();
        if (mp2762.isPresent()) {
            online_devices.mp2762 = true;
            TaskDebug("mp2762 is offline but online again-", millis());
        } else {
            online_devices.mp2762 = false;
            TaskDebug("mp2762 is offline second time-", millis());
        }
        lastCheckChargerOnlineTime = millis();
    }
}

void handleFuelgaugeTask()
{
    if (online_devices.bq40z50) {
        if (millis() - lastCheckFuelGaugeOnlineTime > 30 * 1000) {
            if (bq40z50.isPresent()) {
                online_devices.bq40z50 = true;
                TaskDebug("bq40z50 is still online-", millis());
            } else {
                online_devices.bq40z50 = false;
                TaskDebug("bq40z50 is offline first time-", millis());
            }
            lastCheckFuelGaugeOnlineTime = millis();
        }
        checkBatteryLevels();
        if (TaskDebugFlag) {
            Serial.printf("batteryLevelPercent:");
            Serial.println(batteryLevelPercent);
            Serial.printf("batteryVoltage:");
            Serial.println(batteryVoltage);
            Serial.printf("batteryChargingPercentPerHour:");
            Serial.println(batteryChargingPercentPerHour);
            Serial.printf("batteryTempC:");
            Serial.println(batteryTempC);
        }
    } else {

        if (bq40z50.isPresent()) {
            online_devices.bq40z50 = true;
            TaskDebug("bq40z50 is offline but online again-", millis());
        } else {
            online_devices.bq40z50 = false;
            TaskDebug("bq40z50 is offline second time-", millis());
        }
        lastCheckFuelGaugeOnlineTime = millis();
    }
}

void BatteryCheckTask()
{
    // static uint32_t lastBatteryChargerUpdate = 0;
    static uint32_t lastBatteryFuelGaugeUpdate = 0;
    // 统一处理所有定时任务
    // if (millis() - lastBatteryChargerUpdate >= 1000)
    // {
    //     lastBatteryChargerUpdate = millis();
    //     handleChargerTask();
    // }

    if (millis() - lastBatteryFuelGaugeUpdate >= 3000) {
        lastBatteryFuelGaugeUpdate = millis();
        handleFuelgaugeTask();
    }
}

// void WatchdogFeedTask()
//{
//     WatchdogFeed();
// }
static struct rt_thread led_thread;
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t rt_led_thread_stack[1024];
static void led_thread_entry(void *para);
static rt_uint8_t led_thread_priority = 6;

static struct rt_thread key_thread;
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t rt_key_thread_stack[1024];
static void key_thread_entry(void *para);
static rt_uint8_t key_thread_priority = 6;

void user_task_init()
{
    rt_thread_init(&led_thread,
                   "led_thread",
                   led_thread_entry,
                   RT_NULL,
                   &rt_led_thread_stack,
                   sizeof(rt_led_thread_stack),
                   led_thread_priority,
                   100);
    rt_thread_init(&key_thread,
                   "key_thread",
                   key_thread_entry,
                   RT_NULL,
                   &rt_key_thread_stack,
                   sizeof(rt_key_thread_stack),
                   key_thread_priority,
                   100);
    rt_thread_startup(&led_thread);
    rt_thread_startup(&key_thread);
}

static void led_thread_entry(void *para)
{
    LOG_INFO("Entry %s", __func__);
    while (1) {
        ledStatusUpdate();
        rt_thread_mdelay(100);
    }
}

static void key_thread_entry(void *para)
{
    LOG_INFO("Entry %s", __func__);
    while (1) {
        keyScan();
        rt_thread_mdelay(10);
    }
}


#include "settings.h"
#include "global.h"

#include "Begin.h"
HardwareI2C i2c(M4_I2C1, 0, 9, 0, 8);
Charger mp2762(&i2c);
Fuelgauge bq40z50(&i2c);

void Debugprint_Begin(const char *str, uint32_t time)
{
    if (BeginDebug)
    {
        LOG_INFO("%s %d", str, time);
    }
}

void I2cBegin()
{
    if (present_devices.i2c)
    {
        if (i2c.begin())
        {
            online_devices.i2c = true;
            Debugprint_Begin("I2C online Time:", millis());
        }
        else
        {
            Debugprint_Begin("I2C not online Time:", millis());
        }
    }
}

void I2cEnd()
{
    if (online_devices.i2c)
    {
        i2c.end();
        online_devices.i2c = false;
        Debugprint_Begin("I2C offline Time:", millis());
    }
}

void Mp2762Begin()
{
    if (present_devices.mp2762)
    {
        if (online_devices.i2c)
        {
            if (mp2762.isPresent())
            {
                online_devices.mp2762 = true;
                Debugprint_Begin("MP2762 online Time:", millis());
                mp2762.mp2762registerReset();
                // Set Charger Current
                mp2762.mp2762setFastChargeVoltageMv(6600);

                // Setting precharge current to 880mA
                mp2762.mp2762setPrechargeCurrentMa(500);

                mp2762.mp2762setFastChargeCurrentMa(1600);
                // get charge status
                mp2762.mp2762getChargeStatus();
            }
            else
            {
                Debugprint_Begin("MP2762 not online Time:", millis());
            }
        }
        else
        {
            Debugprint_Begin("I2C not online Time:", millis());
        }
    }
}

void Bq40z50Begin()
{
    if (present_devices.bq40z50)
    {
        if (online_devices.i2c)
        {
            if (bq40z50.isPresent())
            {
                online_devices.bq40z50 = true;
                Debugprint_Begin("BQ40Z50 online Time:", millis());
                checkBatteryLevels();
            }
            else
            {
                Debugprint_Begin("BQ40Z50 not online Time:", millis());
            }
        }
        else
        {
            Debugprint_Begin("I2C not online Time:", millis());
        }
    }
}

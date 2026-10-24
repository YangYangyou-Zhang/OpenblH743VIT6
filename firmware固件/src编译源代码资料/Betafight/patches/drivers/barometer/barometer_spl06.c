/*
 * This file is part of Cleanflight, Betaflight and INAV.
 *
 * This file is free software: you may copy, redistribute and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 *
 * Ported from the INAV SPL06 driver to the Betaflight barometer API.
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#ifdef USE_BARO_SPL06

#include "common/maths.h"
#include "drivers/bus.h"
#include "drivers/time.h"
#include "drivers/barometer/barometer.h"
#include "drivers/barometer/barometer_spl06.h"

typedef struct {
    int16_t c0;
    int16_t c1;
    int32_t c00;
    int32_t c10;
    int16_t c01;
    int16_t c11;
    int16_t c20;
    int16_t c21;
    int16_t c30;
} spl06_coeffs_t;

static spl06_coeffs_t spl06_cal;
static int32_t spl06_pressure_raw = 0;
static int32_t spl06_temperature_raw = 0;
static uint8_t spl06_chip_id = 0;
static DMA_DATA_ZERO_INIT uint8_t sensor_data[SPL06_PRESSURE_LEN];

static int8_t spl06_samples_to_cfg_reg_value(uint8_t sample_rate)
{
    switch (sample_rate) {
    case 1:
        return 0;
    case 2:
        return 1;
    case 4:
        return 2;
    case 8:
        return 3;
    case 16:
        return 4;
    case 32:
        return 5;
    case 64:
        return 6;
    case 128:
        return 7;
    default:
        return -1; // invalid
    }
}

static int32_t spl06_raw_value_scale_factor(uint8_t oversampling_rate)
{
    switch (oversampling_rate) {
    case 1:
        return 524288;
    case 2:
        return 1572864;
    case 4:
        return 3670016;
    case 8:
        return 7864320;
    case 16:
        return 253952;
    case 32:
        return 516096;
    case 64:
        return 1040384;
    case 128:
        return 2088960;
    default:
        return -1; // invalid
    }
}

static int32_t spl06_raw_value(const uint8_t *data)
{
    return (int32_t)((data[0] & 0x80 ? 0xFF000000 : 0) | ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[2]);
}

static bool spl06StartUT(baroDev_t *baro)
{
    return busWriteRegisterStart(&baro->dev, SPL06_MODE_AND_STATUS_REG, SPL06_MEAS_TEMPERATURE);
}

static bool spl06ReadUT(baroDev_t *baro)
{
    if (busBusy(&baro->dev, NULL)) {
        return false;
    }

    return busReadRegisterBufferStart(&baro->dev, SPL06_TEMPERATURE_START_REG, sensor_data, SPL06_TEMPERATURE_LEN);
}

static bool spl06GetUT(baroDev_t *baro)
{
    if (busBusy(&baro->dev, NULL)) {
        return false;
    }

    spl06_temperature_raw = spl06_raw_value(sensor_data);

    return true;
}

static bool spl06StartUP(baroDev_t *baro)
{
    return busWriteRegisterStart(&baro->dev, SPL06_MODE_AND_STATUS_REG, SPL06_MEAS_PRESSURE);
}

static bool spl06ReadUP(baroDev_t *baro)
{
    if (busBusy(&baro->dev, NULL)) {
        return false;
    }

    return busReadRegisterBufferStart(&baro->dev, SPL06_PRESSURE_START_REG, sensor_data, SPL06_PRESSURE_LEN);
}

static bool spl06GetUP(baroDev_t *baro)
{
    if (busBusy(&baro->dev, NULL)) {
        return false;
    }

    spl06_pressure_raw = spl06_raw_value(sensor_data);

    return true;
}

// Returns temperature in degrees centigrade
static float spl06_compensate_temperature(int32_t temperature_raw)
{
    const float t_raw_sc = (float)temperature_raw / spl06_raw_value_scale_factor(SPL06_TEMPERATURE_OVERSAMPLING);
    const float temp_comp = (float)spl06_cal.c0 / 2 + t_raw_sc * spl06_cal.c1;

    return temp_comp;
}

// Returns pressure in Pascal
static float spl06_compensate_pressure(int32_t pressure_raw, int32_t temperature_raw)
{
    const float p_raw_sc = (float)pressure_raw / spl06_raw_value_scale_factor(SPL06_PRESSURE_OVERSAMPLING);
    const float t_raw_sc = (float)temperature_raw / spl06_raw_value_scale_factor(SPL06_TEMPERATURE_OVERSAMPLING);

    const float pressure_cal = (float)spl06_cal.c00 + p_raw_sc * ((float)spl06_cal.c10 + p_raw_sc * ((float)spl06_cal.c20 + p_raw_sc * spl06_cal.c30));
    const float p_temp_comp = t_raw_sc * ((float)spl06_cal.c01 + p_raw_sc * ((float)spl06_cal.c11 + p_raw_sc * spl06_cal.c21));

    return pressure_cal + p_temp_comp;
}

static void spl06Calculate(int32_t *pressure, int32_t *temperature)
{
    if (pressure) {
        *pressure = lrintf(spl06_compensate_pressure(spl06_pressure_raw, spl06_temperature_raw));
    }

    if (temperature) {
        *temperature = lrintf(spl06_compensate_temperature(spl06_temperature_raw) * 100);
    }
}

static bool spl06DetectAddress(extDevice_t *dev)
{
    for (int retry = 0; retry < 5; retry++) {
        delay(50);
        if (busReadRegisterBuffer(dev, SPL06_CHIP_ID_REG, &spl06_chip_id, 1) && spl06_chip_id == SPL06_DEFAULT_CHIP_ID) {
            return true;
        }
    }

    return false;
}

static bool spl06ReadCalibration(extDevice_t *dev)
{
    uint8_t sstatus = 0;
    uint8_t caldata[SPL06_CALIB_COEFFS_LEN];

    for (int retry = 0; retry < 10; retry++) {
        if (busReadRegisterBuffer(dev, SPL06_MODE_AND_STATUS_REG, &sstatus, 1) && (sstatus & SPL06_MEAS_CFG_COEFFS_RDY)) {
            break;
        }
        delay(10);
    }

    if (!(sstatus & SPL06_MEAS_CFG_COEFFS_RDY)) {
        return false;
    }

    if (!busReadRegisterBuffer(dev, SPL06_CALIB_COEFFS_START, caldata, SPL06_CALIB_COEFFS_LEN)) {
        return false;
    }

    spl06_cal.c0  = (caldata[0] & 0x80 ? 0xF000 : 0) | ((uint16_t)caldata[0] << 4) | (((uint16_t)caldata[1] & 0xF0) >> 4);
    spl06_cal.c1  = ((caldata[1] & 0x8 ? 0xF000 : 0) | ((uint16_t)caldata[1] & 0x0F) << 8) | (uint16_t)caldata[2];
    spl06_cal.c00 = (caldata[3] & 0x80 ? 0xFFF00000 : 0) | ((uint32_t)caldata[3] << 12) | ((uint32_t)caldata[4] << 4) | (((uint32_t)caldata[5] & 0xF0) >> 4);
    spl06_cal.c10 = (caldata[5] & 0x8 ? 0xFFF00000 : 0) | (((uint32_t)caldata[5] & 0x0F) << 16) | ((uint32_t)caldata[6] << 8) | (uint32_t)caldata[7];
    spl06_cal.c01 = ((uint16_t)caldata[8] << 8) | ((uint16_t)caldata[9]);
    spl06_cal.c11 = ((uint16_t)caldata[10] << 8) | (uint16_t)caldata[11];
    spl06_cal.c20 = ((uint16_t)caldata[12] << 8) | (uint16_t)caldata[13];
    spl06_cal.c21 = ((uint16_t)caldata[14] << 8) | (uint16_t)caldata[15];
    spl06_cal.c30 = ((uint16_t)caldata[16] << 8) | (uint16_t)caldata[17];

    return true;
}

static bool spl06ConfigureMeasurements(extDevice_t *dev)
{
    uint8_t reg_value;

    reg_value = SPL06_TEMP_USE_EXT_SENSOR | spl06_samples_to_cfg_reg_value(SPL06_TEMPERATURE_OVERSAMPLING);
    if (!busWriteRegister(dev, SPL06_TEMPERATURE_CFG_REG, reg_value)) {
        return false;
    }

    reg_value = spl06_samples_to_cfg_reg_value(SPL06_PRESSURE_OVERSAMPLING);
    if (!busWriteRegister(dev, SPL06_PRESSURE_CFG_REG, reg_value)) {
        return false;
    }

    reg_value = 0;
    if (SPL06_TEMPERATURE_OVERSAMPLING > 8) {
        reg_value |= SPL06_TEMPERATURE_RESULT_BIT_SHIFT;
    }
    if (SPL06_PRESSURE_OVERSAMPLING > 8) {
        reg_value |= SPL06_PRESSURE_RESULT_BIT_SHIFT;
    }
    if (!busWriteRegister(dev, SPL06_INT_AND_FIFO_CFG_REG, reg_value)) {
        return false;
    }

    return true;
}

bool spl06Detect(baroDev_t *baro)
{
    extDevice_t *dev = &baro->dev;
    bool defaultAddressApplied = false;

    delay(20);

    if (dev->bus->busType != BUS_TYPE_I2C) {
        return false;
    }

    if (dev->busType_u.i2c.address == 0) {
        dev->busType_u.i2c.address = SPL06_I2C_ADDR;
        defaultAddressApplied = true;
    }

    if (!spl06DetectAddress(dev)) {
        // SPL06 SDO/ADDR pin may be tied high, moving the device to 0x77
        if (defaultAddressApplied) {
            dev->busType_u.i2c.address = SPL06_I2C_ADDR_ALT;
        }
        if (!spl06DetectAddress(dev)) {
            if (defaultAddressApplied) {
                dev->busType_u.i2c.address = 0;
            }
            return false;
        }
    }

    if (!spl06ReadCalibration(dev) || !spl06ConfigureMeasurements(dev)) {
        if (defaultAddressApplied) {
            dev->busType_u.i2c.address = 0;
        }
        return false;
    }

    busDeviceRegister(dev);

    baro->combined_read = false;
    baro->ut_delay = SPL06_MEASUREMENT_TIME(SPL06_TEMPERATURE_OVERSAMPLING) * 1000;
    baro->up_delay = SPL06_MEASUREMENT_TIME(SPL06_PRESSURE_OVERSAMPLING) * 1000;
    baro->start_ut = spl06StartUT;
    baro->get_ut = spl06GetUT;
    baro->read_ut = spl06ReadUT;
    baro->start_up = spl06StartUP;
    baro->get_up = spl06GetUP;
    baro->read_up = spl06ReadUP;
    baro->calculate = spl06Calculate;

    return true;
}

#endif

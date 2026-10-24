/*
 * This file is part of Cleanflight, Betaflight and INAV.
 *
 * This file is free software: you may copy, redistribute and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 * Ported from the INAV QMC5883P driver to the Betaflight compass API.
 * Orientation is left to the Betaflight mag alignment (MAG_QMC5883P_ALIGN).
 */

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "platform.h"

#ifdef USE_MAG_QMC5883P

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/bus.h"
#include "drivers/bus_i2c.h"
#include "drivers/bus_i2c_busdev.h"
#include "drivers/sensor.h"
#include "drivers/time.h"

#include "compass.h"
#include "compass_qmc5883p.h"

#define QMC5883P_MAG_I2C_ADDRESS 0x2C

#define QMC5883P_REG_ID           0x00
#define QMC5883P_ID_VAL           0x80

#define QMC5883P_REG_DATA_OUTPUT_X 0x01
#define QMC5883P_DATA_BYTES        6

#define QMC5883P_REG_STATUS        0x09
#define QMC5883P_STATUS_DRDY_MASK  0x01

#define QMC5883P_REG_CONF1         0x0A
#define QMC5883P_REG_CONF2         0x0B

#define QMC5883P_CONF1_MODE_CONTINUOUS 0x03
#define QMC5883P_CONF1_ODR_200HZ       (0x03 << 2)
#define QMC5883P_CONF1_OSR1_8          (0x00 << 4)
#define QMC5883P_CONF1_OSR2_8          (0x03 << 6)

#define QMC5883P_CONF2_RNG_8G          (0x02 << 2)
#define QMC5883P_CONF2_RESET           0x80

// This register has no definition in the datasheet and is only mentioned in an example.
#define QMC5883P_REG_DATA_SIGN         0x29
#define QMC5883P_DATA_SIGN_MAGIC_VALUE 0x06

static bool qmc5883pInit(magDev_t *magDev)
{
    extDevice_t *dev = &magDev->dev;
    busDeviceRegister(dev);

    bool ack = true;

    ack = ack && busWriteRegister(dev, QMC5883P_REG_CONF2, QMC5883P_CONF2_RESET);
    delay(30);
    ack = ack && busWriteRegister(dev, QMC5883P_REG_DATA_SIGN, QMC5883P_DATA_SIGN_MAGIC_VALUE);
    ack = ack && busWriteRegister(dev, QMC5883P_REG_CONF2, QMC5883P_CONF2_RNG_8G);
    ack = ack && busWriteRegister(dev, QMC5883P_REG_CONF1,
                                  QMC5883P_CONF1_MODE_CONTINUOUS |
                                  QMC5883P_CONF1_ODR_200HZ |
                                  QMC5883P_CONF1_OSR1_8 |
                                  QMC5883P_CONF1_OSR2_8);

    if (!ack) {
        return false;
    }

    magDev->magOdrHz = 200;
    return true;
}

static bool qmc5883pRead(magDev_t *magDev, int16_t *magData)
{
    extDevice_t *dev = &magDev->dev;
    uint8_t status = 0;
    uint8_t buf[QMC5883P_DATA_BYTES];

    if (!busReadRegisterBuffer(dev, QMC5883P_REG_STATUS, &status, sizeof(status))) {
        return false;
    }

    if ((status & QMC5883P_STATUS_DRDY_MASK) == 0) {
        return false;
    }

    if (!busReadRegisterBuffer(dev, QMC5883P_REG_DATA_OUTPUT_X, buf, sizeof(buf))) {
        return false;
    }

    // Raw axes. Board mounting is handled by the BF mag alignment (MAG_QMC5883P_ALIGN).
    magData[X] = (int16_t)(buf[1] << 8 | buf[0]);
    magData[Y] = (int16_t)(buf[3] << 8 | buf[2]);
    magData[Z] = (int16_t)(buf[5] << 8 | buf[4]);

    return true;
}

bool qmc5883pDetect(magDev_t *magDev)
{
    extDevice_t *dev = &magDev->dev;

    if (dev->bus->busType == BUS_TYPE_I2C && dev->busType_u.i2c.address == 0) {
        dev->busType_u.i2c.address = QMC5883P_MAG_I2C_ADDRESS;
    }

    for (int retry = 0; retry < 5; retry++) {
        // Must write reset first - don't care about the result
        busWriteRegister(dev, QMC5883P_REG_CONF2, QMC5883P_CONF2_RESET);
        delay(30);

        uint8_t sig = 0;

        if (busReadRegisterBuffer(dev, QMC5883P_REG_ID, &sig, sizeof(sig)) && sig == QMC5883P_ID_VAL) {
            magDev->init = qmc5883pInit;
            magDev->read = qmc5883pRead;
            return true;
        }
    }

    return false;
}

#endif

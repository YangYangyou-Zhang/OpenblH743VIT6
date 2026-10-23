/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * Copyright (c) 2026 洋洋柚 (@zhangyangyangyou)
 * OpenblH743VIT6 target, modified from the Betaflight/INAV unified
 * target template. Licensed under GPL-3.0 as above.
 */


#pragma once

#define USE_TARGET_CONFIG

#define TARGET_BOARD_IDENTIFIER "OPH7"
#define USBD_PRODUCT_STRING  "OpenblH743VIT6"

#define LED0                    PE3
#define LED1                    PE2

// *************** SPI1 OSD **************************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI1
#define MAX7456_CS_PIN          PC4

// *************** SPI2 / SPI3 IMU (2x MPU6500) ******
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PD6
#define SPI3_SCK_AF             GPIO_AF6_SPI3
#define SPI3_MISO_AF            GPIO_AF6_SPI3
#define SPI3_MOSI_AF            GPIO_AF5_SPI3

#define USE_DUAL_GYRO
#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS

#define USE_IMU_MPU6500
#define IMU_MPU6500_ALIGN       CW90_DEG_FLIP
#define MPU6500_SPI_BUS         BUS_SPI2
#define MPU6500_CS_PIN          PB12

#define IMU_MPU6500_ALIGN_1     CW90_DEG_FLIP
#define MPU6500_SPI_BUS_1       BUS_SPI3
#define MPU6500_CS_PIN_1        PD7

// *************** I2C / Baro / Mag ******************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB6
#define I2C1_SDA                PB7

#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_BMP280
#define USE_BARO_SPL06
#define SPL06_I2C_ADDR          0x77

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_QMC5883P

// *************** UART ******************************
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3

#define USE_UART3
#define UART3_TX_PIN            PD8
#define UART3_RX_PIN            PD9

#define USE_UART4
#define UART4_TX_PIN            PA0
#define UART4_RX_PIN            PA1

#define USE_UART6
#define UART6_TX_PIN            PC6
#define UART6_RX_PIN            PC7

#define USE_UART7
#define UART7_TX_PIN            PE8
#define UART7_RX_PIN            PE7

#define USE_UART8
#define UART8_TX_PIN            PE1
#define UART8_RX_PIN            PE0

#define SERIAL_PORT_COUNT       9

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART6

// *************** ADC *******************************
#define USE_ADC
#define ADC_INSTANCE            ADC1
#define ADC_CHANNEL_1_PIN       PC0
#define ADC_CHANNEL_2_PIN       PC1
#define VBAT_ADC_CHANNEL        ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL ADC_CHN_2

// *************** SD Card (SDMMC1, 4-bit) ***********
#define USE_SDCARD
#define USE_SDCARD_SDIO
#define SDCARD_SDIO_DEVICE      SDIODEV_1
#define SDCARD_SDIO_4BIT
#define SDCARD_SDIO_NORMAL_SPEED

// *************** others ****************************
#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_VBAT | FEATURE_CURRENT_METER | FEATURE_BLACKBOX)

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff

#define MAX_PWM_OUTPUT_PORTS    12

#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define USE_DSHOT
#define USE_ESC_SENSOR

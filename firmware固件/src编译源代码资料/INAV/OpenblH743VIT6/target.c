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


#include <stdint.h>

#include "platform.h"

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/sensor.h"

BUSDEV_REGISTER_SPI_TAG(busdev_mpu6500,   DEVHW_MPU6500,  MPU6500_SPI_BUS,   MPU6500_CS_PIN,   NONE,   0,  DEVFLAGS_NONE,  IMU_MPU6500_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_mpu6500_1, DEVHW_MPU6500,  MPU6500_SPI_BUS_1, MPU6500_CS_PIN_1, NONE,   1,  DEVFLAGS_NONE,  IMU_MPU6500_ALIGN_1);

timerHardware_t timerHardware[] = {

    DEF_TIM(TIM1,  CH1, PE9,  TIM_USE_OUTPUT_AUTO, 0, 8),   // PWM1
    DEF_TIM(TIM1,  CH2, PE11, TIM_USE_OUTPUT_AUTO, 0, 9),   // PWM2
    DEF_TIM(TIM1,  CH3, PE13, TIM_USE_OUTPUT_AUTO, 0, 10),  // PWM3
    DEF_TIM(TIM1,  CH4, PE14, TIM_USE_OUTPUT_AUTO, 0, 11),  // PWM4

    DEF_TIM(TIM4,  CH1, PD12, TIM_USE_OUTPUT_AUTO, 0, 0),   // PWM5
    DEF_TIM(TIM4,  CH2, PD13, TIM_USE_OUTPUT_AUTO, 0, 1),   // PWM6
    DEF_TIM(TIM4,  CH3, PD14, TIM_USE_OUTPUT_AUTO, 0, 2),   // PWM7
    DEF_TIM(TIM4,  CH4, PD15, TIM_USE_OUTPUT_AUTO, 0, 0),   // PWM8  no HW DMA (TIM4_CH4) -> PWM only

    DEF_TIM(TIM15, CH1, PE5,  TIM_USE_OUTPUT_AUTO, 0, 12),  // PWM9
    DEF_TIM(TIM15, CH2, PE6,  TIM_USE_OUTPUT_AUTO, 0, 0),   // PWM10 no HW DMA (TIM15_CH2) -> PWM only

    DEF_TIM(TIM3,  CH4, PB1,  TIM_USE_OUTPUT_AUTO, 0, 3),   // PWM11
    DEF_TIM(TIM3,  CH3, PB0,  TIM_USE_OUTPUT_AUTO, 0, 4),   // PWM12
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);

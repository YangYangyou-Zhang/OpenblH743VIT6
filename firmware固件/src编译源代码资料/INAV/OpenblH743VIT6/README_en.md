# OpenblH743VIT6 — INAV Configuration

INAV target for the custom STM32H743VIT6 (2MB Flash) flight controller. Pin assignments treat ArduPilot's `hwdef.dat` as the hardware source of truth.

## Hardware Configuration

| Peripheral | Interface | Pins |
|------|------|------|
| Gyro/Accel IMU0 | SPI2 | SCK=PB13, MISO=PB14, MOSI=PB15, CS=PB12 |
| Gyro/Accel IMU1 | SPI3 | SCK=PB3, MISO=PB4, MOSI=PD6, CS=PD7 |
| OSD (MAX7456) | SPI1 | SCK=PA5, MISO=PA6, MOSI=PA7, CS=PC4 |
| Barometer BMP280 / SPL06 | I2C1 | SCL=PB6, SDA=PB7 |
| Magnetometer QMC5883P | I2C1 | SCL=PB6, SDA=PB7 |
| SD card | SDMMC1 4-bit | Blackbox log |
| 12 PWM outputs | — | See table below |

IMU orientation: `CW90_DEG_FLIP` (calibrated on the actual board, corresponding to hwdef's `ROTATION_ROLL_180` + 90° yaw).

## PWM Output & DShot

Of the 12 PWM outputs, 10 support DShot; 2 can only do regular PWM because the STM32H743 hardware lacks the corresponding DMA requests:

| Output | Pins | Timer | DShot |
|------|------|--------|-------|
| PWM1–4 | PE9 / PE11 / PE13 / PE14 | TIM1_CH1–4 | ✅ |
| PWM5–7 | PD12 / PD13 / PD14 | TIM4_CH1–3 | ✅ |
| PWM8 | PD15 | TIM4_CH4 | ❌ No DMA |
| PWM9 | PE5 | TIM15_CH1 | ✅ |
| PWM10 | PE6 | TIM15_CH2 | ❌ No DMA |
| PWM11–12 | PB1 / PB0 | TIM3_CH4 / TIM3_CH3 | ✅ |

> PWM8 / PWM10 can only do PWM output (servo or PWM-mode ESC), not DShot.
> INAV has no bitbang; DShot must go through DMA, so these two channels cannot do DShot in INAV.

## Dual IMU (gyro_to_use)

The board has two MPU6500 (IMU0=SPI2, IMU1=SPI3). INAV's `MAX_GYRO_COUNT=1`, so **only one IMU is used at a time**; dual-gyro redundancy is not supported. Use the `gyro_to_use` parameter to select:

| Value | Which IMU |
|----|-----------|
| `0` (default) | First (SPI2, CS=PB12) |
| `1` | Second (SPI3, CS=PD7) |

CLI switch (e.g. switch to the second IMU):
```
set gyro_to_use = 1
save
```

Switch back to the first: `set gyro_to_use = 0` then `save`. To verify the switch worked: check in `resource` whether the `SPI3 SCK/MISO/MOSI` or `SPI2 SCK/MISO/MOSI` group is in use.

## SD Card

SDMMC1 4-bit (PC8–PC12, PD2). The default high-speed mode (~30MHz) fails to initialize when the slot wiring is poor (status shows `SD card: Startup failed`); `SDCARD_SDIO_NORMAL_SPEED` has been enabled in target.h to slow down to ~15MHz. If it still fails, it is most likely a hardware issue such as slot contact/pull-up resistors.

## Build

```bash
# Update the system and install required tools
sudo apt-get update && sudo apt-get upgrade
sudo apt-get install git make ruby cmake gcc-arm-none-eabi

# Clone the iNav source code
git clone https://github.com/iNavFlight/inav.git
cd inav

# Place this folder (OpenblH743VIT6) under ~/inav/src/main/target/

# 1. Create the build directory
mkdir -p build

# 2. Run CMake configuration
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake

# 3. Build the target (-j4 uses 4 CPU threads)
cmake --build build --target OpenblH743VIT6 -j4

# The firmware is generally under ~/inav/build/
```

---

© 2026 洋洋柚 (@zhangyangyangyou) · OpenblH743VIT6

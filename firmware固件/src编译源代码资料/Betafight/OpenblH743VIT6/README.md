# OpenblH743VIT6 — Betaflight Configuration

Betaflight unified target configuration for the custom STM32H743VIT6 (2MB Flash) flight controller. Pin assignments treat ArduPilot's `hwdef.dat` as the hardware source of truth.

## Hardware Configuration

| Peripheral | Interface | Pins |
|------|------|------|
| Gyro/Accel IMU0 | SPI2 | SCK=PB13, MISO=PB14, MOSI=PB15, CS=PB12 |
| Gyro/Accel IMU1 | SPI3 | SCK=PB3, MISO=PB4, MOSI=PD6, CS=PD7 |
| OSD (MAX7456) | SPI1 | SCK=PA5, MISO=PA6, MOSI=PA7, CS=PC4 |
| Barometer SPL06 | I2C1 | SCL=PB6, SDA=PB7 (⚠️ not yet supported, see below) |
| Magnetometer QMC5883P | I2C1 | SCL=PB6, SDA=PB7 (⚠️ not yet supported, see below) |
| SD card | SDMMC1 4-bit | ⚠️ not yet supported (init hangs, disabled) |
| 12 PWM outputs | — | See table below |

IMU orientation: `CW90_DEG_FLIP` (calibrated on the actual board).

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

## DShot Setup

- **One-way DShot**: just select the protocol; it uses regular DMA with no extra setup.
- **Bidirectional DShot (telemetry return)**: run `set dshot_bidir = ON` then `save` in the CLI.
  - Note: enabling bidirectional triggers Betaflight's bitbang mode (`dshot_bitbang` defaults to `AUTO`),
    which switches all motors globally to bitbang. CPU overhead is high; not recommended with all 12 channels enabled.
  - Force one-way to also use bitbang: `set dshot_bitbang = ON` (also a global switch; generally unnecessary).

## Known Issues / Unfinished Items

> **Barometer (SPL06), magnetometer (QMC5883P) and SD card are not yet supported; they will be supported in a future Betaflight release.**

1. **SD card (SDMMC1) not yet supported**: With `USE_SDCARD_SDIO` enabled, SD card initialization hangs the `init()` flow and blocks the subsequent USB initialization, causing "USB completely unrecognized after flashing". Currently `USE_SDCARD` / `USE_SDCARD_SDIO` are **commented out in config.h** to work around it. Re-enable once a future release fixes the SDMMC init dead-wait (similar to INAV's "SD card Startup failed", but BF doesn't add a timeout here).

2. **Barometer SPL06 not yet supported**: Betaflight 4.5.3 has no SPL06 driver (only BMP085/BMP280/BMP388), so the onboard SPL06 cannot be recognized. Re-enable once a future release adds an SPL06 driver (or port `barometer_spl06` from INAV).

3. **Magnetometer QMC5883P not yet supported**: BF only has `compass_qmc5883l.c` (QMC5883L address 0x0D), while the onboard QMC5883P is at address 0x2C. Reuse was attempted (adding an `#ifndef` guard to `QMC5883L_MAG_I2C_ADDRESS` in `compass_qmc5883l.c` + defining 0x2C in config.h) but is not enabled yet, pending a future release. To enable, uncomment `USE_MAG` / `USE_MAG_QMC5883` in config.h.

## Build

```bash
# Update the package list and upgrade the system
sudo apt update && sudo apt upgrade

# Install base build tools (git, make, clang, python, etc.)
sudo apt -y install build-essential git curl clang-18 python3 python-is-python3

# Clone the Betaflight source and check out a stable version (e.g. 4.5.3)
git clone https://github.com/betaflight/betaflight.git
cd betaflight
git checkout 4.5.3

# Automatically download and install the required toolchain
make arm_sdk_install

# On first build or after switching branches, generate default configs for all targets
make configs

# Place this folder (OpenblH743VIT6) under ~/betaflight/src/config/configs/, then build
make config OpenblH743VIT6

# Firmware is under ~/betaflight/obj/: betaflight_4.5.3_STM32H743_OpenblH743VIT6.hex
```

---

© 2026 洋洋柚 (@zhangyangyangyou) · OpenblH743VIT6

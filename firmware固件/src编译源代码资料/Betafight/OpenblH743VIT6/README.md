# OpenblH743VIT6 — Betaflight Configuration

Betaflight unified target configuration for the custom STM32H743VIT6 (2MB Flash) flight controller. Pin assignments treat ArduPilot's `hwdef.dat` as the hardware source of truth.

## Hardware Configuration

| Peripheral | Interface | Pins |
|------|------|------|
| Gyro/Accel IMU0 | SPI2 | SCK=PB13, MISO=PB14, MOSI=PB15, CS=PB12 |
| Gyro/Accel IMU1 | SPI3 | SCK=PB3, MISO=PB4, MOSI=PD6, CS=PD7 |
| OSD (MAX7456) | SPI1 | SCK=PA5, MISO=PA6, MOSI=PA7, CS=PC4 |
| Barometer SPL06 | I2C1 | SCL=PB6, SDA=PB7, 0x77 — supported via local driver patch, see below |
| Magnetometer QMC5883P | I2C1 | SCL=PB6, SDA=PB7, 0x2C — supported via local driver patch, see below |
| SD card | SDMMC1 4-bit | ⚠️ not yet supported (init hangs, disabled) |
| 12 PWM outputs | — | See table below |

IMU orientation: `CW90_DEG_FLIP` (calibrated on the actual board).

> **Flash layout note**: Betaflight's H743 linker places the vector table at `0x08000000`,
> the BF config sector at `0x08020000` and the firmware at `0x08040000`. This **overwrites the
> APM bootloader** used by PX4/ArduPilot. Flash BF with a full chip erase; to return to
> PX4/ArduPilot, reflash the APM bootloader first.

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

> **The SD card is not yet supported. The SPL06 barometer and QMC5883P compass are supported via the local patches in `../patches/`.**

1. **SD card (SDMMC1) not yet supported**: With `USE_SDCARD_SDIO` enabled, SD card initialization hangs the `init()` flow and blocks the subsequent USB initialization, causing "USB completely unrecognized after flashing". Currently `USE_SDCARD` / `USE_SDCARD_SDIO` are **commented out in config.h** to work around it. Re-enable once a future release fixes the SDMMC init dead-wait (similar to INAV's "SD card Startup failed", but BF doesn't add a timeout here).

## SPL06 barometer (local patch)

Betaflight 4.5.3 has no SPL06 driver. A local patch set in `../patches/` adds it
(ported from INAV) and probes both 0x76 and 0x77; the onboard chip is at 0x77.

Apply before building:

```bash
cp -r patches/drivers/barometer/* ../betaflight/src/main/drivers/barometer/
cp -r patches/sensors/*           ../betaflight/src/main/sensors/
cp    patches/cli/settings.c      ../betaflight/src/main/cli/
```

`config.h` already enables `USE_BARO` and `USE_BARO_SPL06`. After flashing, `status` should
show `Barometer`; if AUTO misses it, run `set baro_hardware = SPL06` + `save`.

## QMC5883P compass (local patch)

Betaflight 4.5.3 only has the QMC5883L driver (address 0x0D); the onboard QMC5883P is at 0x2C.
The local patch in `../patches/drivers/compass/` adds a QMC5883P driver ported from INAV.

`config.h` enables `USE_MAG` and `USE_MAG_QMC5883P`, and sets
`MAG_QMC5883P_ALIGN = CW270_DEG_FLIP` (matching PX4 `-R 10`, ROLL_180_YAW_90).
After flashing, `status` should show `Compass`; if AUTO misses it, run
`set mag_hardware = QMC5883P` + `save`, then run the compass calibration.
**Verify the heading direction** — if it is wrong, adjust `align_mag` and recalibrate.

The patches are specific to Betaflight 4.5.3 and must be re-applied after upgrading.

## Build

Tested with Betaflight 4.5.3 and GCC 10.2.1 (the official 4.5.3 toolchain is 10.3.1).

```bash
git clone https://github.com/betaflight/betaflight.git   # or the gitcode mirror
cd betaflight
git checkout 4.5.3

# Apply the SPL06 patch (from this repository's Betafight folder)
cp -r <repo>/firmware固件/src编译源代码资料/Betafight/patches/drivers/barometer/* src/main/drivers/barometer/
cp -r <repo>/firmware固件/src编译源代码资料/Betafight/patches/drivers/compass/*   src/main/drivers/compass/
cp -r <repo>/firmware固件/src编译源代码资料/Betafight/patches/sensors/*           src/main/sensors/
cp    <repo>/firmware固件/src编译源代码资料/Betafight/patches/cli/settings.c      src/main/cli/

# Put this folder (OpenblH743VIT6) at src/config/configs/OpenblH743VIT6/
# Do NOT run `make configs` with 4.5.3 — it clones GitHub's betaflight/config repo.

make OpenblH743VIT6 ARM_SDK_DIR=$HOME/gcc-arm-none-eabi-10-2020-q4-major \
  GCC_REQUIRED_VERSION=10.2.1 -j32
# Output: obj/betaflight_4.5.3_STM32H743_OpenblH743VIT6.hex
```

After flashing, run `defaults` + `save` in the CLI once (otherwise the IMU orientation may still
read the old config value).

---

© 2026 洋洋柚 (@zhangyangyangyou) · OpenblH743VIT6

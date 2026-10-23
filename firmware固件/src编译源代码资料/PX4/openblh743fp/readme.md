# OpenblH743VIT6-FP — Flash-Based Parameter Storage Variant

## What this version is

Same hardware as `../openblh743` (STM32H743VIT6, Board ID 18848, shares the APM bootloader).
Two differences:

1. **Parameters are stored in internal flash and do not depend on the SD card.**
2. **PB0 is a passive buzzer again, PB1 is unused, and PWM outputs drop from 12 to 10.**

### Why parameters had to live on the SD card originally

The original `openblh743` used `CONFIG_BOARD_PARAM_FILE=/fs/microsd/params`.
That is not a design preference — the code region
`FLASH ORIGIN=0x08020000 LENGTH=1920K` fills the entire 2 MB of flash, leaving
**no 128 KB sector for parameters**, so they had to be externalized to the SD card.
Cost: parameters depend entirely on the SD card — whenever the card is unavailable they cannot be
saved (dmesg spams `param export to /fs/microsd/params failed (-1)` /
`param auto save unavailable (-1)`) and the system keeps reporting `Missing FMU SD Card`.

This variant enables PX4's `FLASH_BASED_PARAMS` backend; parameters are written to the last
128 KB sector of flash (0x081E0000). **Parameters can be read, written and saved with no SD card.**

> Note: moving parameters to flash only solves "losing parameters". **Flight logs still go to the
> SD card** (`/fs/microsd/log`).

## Hardware (identical to the original)

| Peripheral | Model | Bus | Notes |
|------|------|------|------|
| IMU0 | MPU6500 (or ICM-42605) | SPI2, CS PB12 | Polling mode; either chip can be fitted |
| IMU1 | MPU6500 (or ICM-42605) | SPI3, CS PD7 | Either chip can be fitted |
| IMU2 (optional) | MPU6500 or ICM-42605 | SPI3, CS PD5 | ICM driver compiled, **not started** (see below) |
| Barometer | SPL06 | I2C (0x76/0x77 auto-scan) | |
| Compass | QMC5883P | I2C1 (0x2C) | Driver rotation `-R 10` (ROLL_180_YAW_90); keep `CAL_MAG0_ROT` = **-1** (see below) |
| Battery | ADC1 ch10/11 | PC0(V)/PC1(I) | Reading ~0 with no battery is normal |
| OSD | AT7456E | SPI1 (PA5/PA6/PA7, CS PC4) | `OSD_ATXXXX_CFG`: 1=NTSC / 2=PAL |
| Buzzer | passive | **PB0 (TIM3_CH3)** | `tone_alarm`, auto-started by rcS |
| CAN | UAVCAN/DroneCAN | PB8(RX)/PB9(TX) | CAN variant only |

### Optional second IMU: ICM-42605 (compiled, not started)

The `icm42605` driver is compiled into all four firmware variants. Both the MPU6500 and the
ICM-42605 (and, via the compatibility patch, the ICM-42688-P) are registered on **all three
IMU chip-select positions**:

| Bus | CS | Registered device types |
|---|---|---|
| SPI2 | PB12 | MPU6500, ICM42605 |
| SPI3 | PD7 | MPU6500, ICM42605 |
| SPI3 | PD5 | MPU6500, ICM42605 |

The MPU6500 startup in `rc.board_sensors` is unchanged. The `icm42605` driver is deliberately
**not started** (the mounting rotation still has to be measured). To test one position:

```
icm42605 start -s -b 3 -c 5  -R <rotation>   # PD5
icm42605 start -s -b 3 -c 7  -R <rotation>   # PD7
icm42605 start -s -b 2 -c 12 -R <rotation>   # PB12
```

Without `-b`/`-c` it probes every registered ICM42605 device; `-c` selects the CS pin number.

- The driver accepts WHO_AM_I `0x42` (ICM-42605) and, after the local patch described below,
  `0x47` (ICM-42688-P). An ICM-42688-P then runs in **ICM-42605 compatibility mode** (16-bit
  FIFO, reset-default filters, no temperature/FIFO-timestamp processing). The native 42688P
  driver is not compiled (+7.6 KB).
- No DRDY line is required: without one the driver falls back to interval-scheduled FIFO reads.
- SPI3 is the only IMU bus with DMA enabled; SPI2 (IMU0) runs in polling mode.

## PWM outputs and buzzer (different from the original — important)

**PWM drops from 12 to 10 outputs**; PB0 is given to the buzzer and PB1 is left unused:

| Output | Timer | Pin |
|------|--------|------|
| PWM 1-4 | TIM1 CH1-4 | PE9 / PE11 / PE13 / PE14 |
| PWM 5-8 | TIM4 CH1-4 | PD12 / PD13 / PD14 / PD15 |
| PWM 9-10 | TIM15 CH1-2 | PE5 / PE6 |
| (no PWM 11/12) | — | PB1 unassigned, PB0 = buzzer |

- **PB1**: left in its reset default state, assigned to nothing.
- **PB0**: passive buzzer, `CONFIG_DRIVERS_TONE_ALARM=y`, `TONE_ALARM_TIMER 3` /
  `TONE_ALARM_CHANNEL 3`, `GPIO_TONE_ALARM = GPIO_TIM3_CH3OUT_1` (PB0, AF2).
  Mute with the QGC parameter `CBRK_BUZZER`.
- **Why three changes are required**: PX4's `ToneAlarmInterfacePWM.cpp` hard-checks that when
  `TONE_ALARM_TIMER == 3`, NuttX's `CONFIG_STM32_TIM3` **must not** be enabled — otherwise it is a
  compile-time `#error`. That is exactly why the buzzer was originally dropped (it fought the PWM
  driver over TIM3). This variant:
  1. `src/timer_config.cpp`: removes the Timer3 entry from `io_timers[]` and the PB1 (TIM3_CH4) /
     PB0 (TIM3_CH3) channels;
  2. `board_config.h`: `DIRECT_PWM_OUTPUT_CHANNELS` 12→10, adds the `TONE_ALARM_*` /
     `GPIO_TONE_ALARM*` defines, and puts `GPIO_TONE_ALARM_IDLE` into `PX4_GPIO_INIT_LIST`;
  3. `nuttx-config/nsh/defconfig`: `CONFIG_STM32H7_TIM3=y` → commented out.
  TIM3 then belongs exclusively to the buzzer.

> **After changing `timer_config.cpp` you must `rm -rf build/<target>` and rebuild cleanly**,
> otherwise the embedded actuator metadata (`actuators.json`, generated at CMake configure time)
> that QGC reads is still the old one.

## Flash layout (different from the original)

| Region | Address | Size |
|------|------|------|
| Bootloader | 0x08000000 | 128 KB |
| Firmware | 0x08020000 | **1792 KB** (was 1920 KB) |
| **Parameter sector** | **0x081E0000** | **128 KB** |

- The **firmware ceiling drops from 1920 KB to 1792 KB** — check headroom before adding features.
- The parameter sector is **not covered by the merged hex** (firmware only reaches ~0x081Dxxxx),
  so **reflashing the firmware does not erase saved parameters**.

## What was trimmed vs. the original

To fit the fixed-wing/VTOL control chain (`fw_mode_manager` + `fw_lat_lon_control`, ~45 KB) and
the optional ICM-42605 driver (~3.5 KB) while **keeping all 14 magnetometer drivers**, both variants disable:

| Cut | Reason |
|------|------|
| `SYSTEMCMDS_TESTS` | Bench self-test command suite (~112 KB) |
| `DRIVERS_GNSS_SEPTENTRIO` | Septentrio-specific GNSS protocol (generic GPS kept) |
| `MODULES_LOCAL_POSITION_ESTIMATOR` | Legacy LPE (EKF2 is the default) |
| `COMMON_TELEMETRY` | FrSky/HOTT/BST telemetry protocols (MAVLink is used instead) |
| `DRIVERS_TAP_ESC` | TAP_ESC-specific ESC protocol (this vehicle uses DShot) |
| `DRIVERS_SMART_BATTERY_BATMON` | SMBus smart-battery driver (analog ADC / INA238 are used instead) |

- The CAN (`default`) variant additionally disables `MODULES_UXRCE_DDS_CLIENT` (CAN and uXRCE-DDS are mutually exclusive); the dds variant keeps it.
- `openbl_h743fp_default` has the least headroom, so it also disables `SYSTEMCMDS_SD_BENCH` and `SYSTEMCMDS_SD_STRESS` (SD benchmark commands).
- **All 14 magnetometer drivers are retained**; DShot + `ESC_BATTERY` (ESC telemetry), UAVCAN, AT7456E OSD, SPL06, MPU6500, the optional ICM-42605 driver, VL53L1X, MS4525/SDP3X and INA238 are retained.
- Everything else (airship, UWB, payload deliverer, wind sensor, gimbal + camera, ESC battery, load monitor, `gyro_fft`, temperature compensation, MC/FW autotune, RPM, PPS, etc.) is retained. The landing-target estimator is kept for future companion-computer precision landing.

Current usage:

| Variant | Flash used | Free |
|------|-----------|------|
| `openbl_h743fp_dds` | 1,757,264 B / 1792 KB = **95.76%** | ~76 KB |
| `openbl_h743fp_default` (CAN) | 1,832,760 B / 1792 KB = **99.88%** | **~2.2 KB** ⚠️ |

## VTOL / Fixed-wing support (2026-10-22)

The firmware includes PX4's fixed-wing control chain:

- `fw_mode_manager`: FW modes (Position/Altitude/Manual/Stabilized/Acro), takeoff/landing, launch detection, runway takeoff
- `fw_lat_lon_control`: TECS, lateral/longitudinal control, NPFG path following

`rc.vtol_apps` starts both modules automatically; the VTOL transition switch is `RC_MAP_TRANS_SW`.
Note: this vehicle has **no airspeed sensor** — the tailsitter front transition completes on attitude/time (not `VT_ARSP_TRANS=10 m/s`) and TECS runs without airspeed. Add a pitot and set `VT_FW_MIN_ALT` / `VT_QC_ALT_LOSS` before the first FW flight.

## LED indication (2026-10-22)

Hardware: PE2 = R, PE3 = G, PE4 = B (active low). PX4's colour codes are `BLUE=0, RED=1, SAFETY=2, GREEN=3`;
the `g_ledmap` in `led.c` must follow that order (the old map was G/B/R, so the blue logical LED lit the green pin).

Current behaviour (arming-state and overload colours are swapped so that hardware with only a single red LED works):

| State | LED |
|---|---|
| Pre-flight checks failing (cannot arm) | **red, 10 Hz** |
| Ready to arm | **red, 1 Hz** |
| Armed, normal | **solid red** (`BOARD_ARMED_LED = LED_RED`) |
| Armed + failsafe | red, 2 Hz |
| Overload/error | **blue, 10 Hz** |

## Precision landing (2026-10-23)

`landing_target_estimator` (LTE, ~3.4 KB) and the navigator `AUTO_PRECLAND` state machine are kept
for a future companion computer. No target source is fitted yet, so the feature is present but has
no input.

| Target input | Path | LTE needed |
|---|---|---|
| MAVLink `LANDING_TARGET`, `position_valid=1`, `frame=MAV_FRAME_LOCAL_NED` | published directly as `landing_target_pose` | no |
| `LANDING_TARGET` with angles (`position_valid=0`) or an IRLock | `irlock_report` → LTE → `landing_target_pose` | yes |

- Trigger: mission land item `land_precision=1/2`, `RTL_PLD_MD=1/2`, or `commander mode auto:precland`.
- Defaults: `PLD_SRCH_ALT=10 m`, `PLD_SRCH_TOUT=10 s`, `PLD_BTOUT=5 s`, `PLD_HACC_RAD=0.2 m`,
  `PLD_FAPPR_ALT=0.1 m`.
- Without a target it searches at `PLD_SRCH_ALT` and falls back to a normal landing.

## Local modifications to PX4 shared code

Besides the board directory `boards/openbl/`, this source tree contains three local changes to
**PX4 shared code**. They are not upstream and will be overwritten by `git pull` /
`git checkout` — back them up before upgrading.
The patched versions of these files are included under `../shared_src/` (same relative paths
as in the PX4 tree); copy them over the corresponding files.

### 1. VL53L1X laser rangefinder driver

File: `src/drivers/distance_sensor/vl53l1x/vl53l1x.cpp`

| Item | Original | This build |
|---|---|---|
| `VL53L1X_DELAY` (polling interval) | 50000 µs | **20000 µs** |
| `VL53L1X_SAMPLE_RATE` (timing budget) | 200 ms | **100 ms** |
| `VL53L1X_INTER_MEAS_MS` (measurement period) | 250 ms | **110 ms** |
| Signal-quality field | ST range status written straight into `quality` | **`rangeStatus == 0 → 100`, otherwise `0`** |

- **Sample rate** ~4 Hz → **~9 Hz**; **range** 4 m → **~2.5–3 m**
- Fixes an upstream field-mapping error: the I2C transaction status (always 0 on success) was
  written into `quality`, and PX4 treats `quality == 0` as "invalid signal", so every valid
  range measurement was discarded by the EKF. With the fix the EKF actually uses the data.
- Enable with:

  ```
  param set SENS_EN_VL53L1X 1     # reboot required
  param set EKF2_RNG_CTRL 2       # always use for height aiding
  ```

### 2. uXRCE-DDS microcdr repository path

File: `src/modules/uxrce_dds_client/Micro-XRCE-DDS-Client/cmake/SuperBuild.cmake`

`GIT_REPOSITORY` was changed from a hard-coded build directory to
`$ENV{HOME}/gitcache/microcdr` (a local clone of Micro-CDR v2.0.1); otherwise the dds target
fails with `Failed to clone repository` after a clean build. Create the cache with:

```bash
git clone --branch v2.0.1 https://github.com/eProsima/Micro-CDR.git ~/gitcache/microcdr
```

or adjust the path to your own copy.
**This is a submodule change and will be reverted by `git submodule update`.**

### 3. ICM-42605 driver also accepts ICM-42688-P

Files: `src/drivers/imu/invensense/icm42605/ICM42605.cpp` and
`src/drivers/imu/invensense/icm42605/InvenSense_ICM42605_registers.hpp`

`WHO_AM_I = 0x47` (ICM-42688-P) is now accepted in addition to `0x42`, both in `probe()` and in
the post-reset check. The rest of the driver is unchanged, so an ICM-42688-P is driven exactly
like an ICM-42605:

- 16-bit FIFO (packet 3), 8 kHz ODR, ±2000 dps / ±16 g;
- no 20-bit hires, no AAF/notch filter setup, no temperature or FIFO-timestamp processing;
- relies on the chip's reset defaults for `INTF_CONFIG0/1` and the filter registers.

**This is a compatibility hack, not a native driver.** Verify accel/gyro data on the bench
(1 g along gravity, zero gyro bias, correct axis signs) before flight. The patched files are in
`../shared_src/`; `git pull` / `git checkout` will revert them.

## Build

1. Toolchain must be **GCC 10.2.1** (`arm-none-eabi-10-2020-q4-major`). H7 code built with
   GCC 13.x will not boot.
2. Commands and artifacts:

```bash
make openbl_h743fp_dds        # DDS variant (uXRCE-DDS)
make openbl_h743fp_default    # CAN variant (UAVCAN/DroneCAN)
# artifacts: build/openbl_h743fp_<label>/openbl_h743fp_<label>.{px4,bin}
```

> Local archive naming: `<target>_1.17.0-alpha1.hex` (bootloader included) / `<target>_1.17.0-alpha1.px4`.

3. Board directory `boards/openbl/h743fp/`, **independent of `boards/openbl/h743/`**.
   Differences from the original:

| File | Change |
|------|------|
| `src/board_config.h` | `#define FLASH_BASED_PARAMS`; `DIRECT_PWM_OUTPUT_CHANNELS` 12→10; `TONE_ALARM_*`/`GPIO_TONE_ALARM*` defines; `GPIO_TONE_ALARM_IDLE` in the init list |
| `src/timer_config.cpp` | Timer3 removed: PB1/PB0 PWM channels deleted |
| `nuttx-config/scripts/script.ld` | `FLASH ... LENGTH = 1920K` → **1792K** |
| `nuttx-config/nsh/defconfig` | `CONFIG_STM32H7_TIM3` disabled; `CONFIG_ARCH_BOARD_CUSTOM_DIR` → `h743fp` |
| `default.px4board` / `dds.px4board` | `CONFIG_BOARD_PARAM_FILE` removed; `CONFIG_DRIVERS_TONE_ALARM=y` added; CAN variant has 3 extra trims |

## Merge bootloader + firmware (merged hex)

```bash
O=arm-none-eabi-objcopy
$O -I binary -O ihex --change-addresses 0x08000000 openbl_h743_bootloader.bin /tmp/bl.hex
$O -I binary -O ihex --change-addresses 0x08020000 openbl_h743fp_dds.bin       /tmp/fw.hex
grep -v ':00000001FF' /tmp/bl.hex > openblh743fp_dds_merged.hex
cat /tmp/fw.hex >> openblh743fp_dds_merged.hex
```

(For the CAN variant, replace `dds` with `default`.)

## Flashing

- Use **ST-Link** to flash `openblh743fp_dds_merged.hex` or `openblh743fp_default_merged.hex`.
- **When switching from the original (SD-parameter) firmware for the first time, do a full chip
  erase in STM32CubeProgrammer first**: the original firmware extended to 0x08200000, so the
  parameter sector still holds stale firmware bytes. A clean erase is safest.
- Afterwards, normal flashing will not erase parameters.

## Verifying

**Parameter persistence**: change a parameter after boot (QGC or NSH `param set`) → power-cycle →
the value should still be there. (With the original firmware and no SD card this always failed.)

**Buzzer**: there should be a startup tune (`tone_alarm`). If silent, check `param show CBRK_BUZZER`
(muted?), `tone_alarm status` in NSH, and that PB0 is really wired to a **passive** buzzer.

**PWM count**: QGC's Actuators page should show `MAIN 1-4` / `MAIN 5-8` / `MAIN 9-10` (10 channels),
with no MAIN 11/12.

## Peripheral/interface status

Identical to the original apart from PWM/buzzer — see `../openblh743/readme.md`: compass driver
rotation `-R 10`, `CAL_MAG0_ROT` kept at -1, serial assignment (RC=ttyS5/USART6, GPS2=ttyS2, TEL2=ttyS3, URT6=ttyS6,
TEL3=ttyS7), AT7456E usage, SPI2 DMA disabled. GPIO table: `../openblh743/gpio.md` (same hardware,
but the PB0/PB1 roles have changed).

## Known notes

- **Flight logging still requires the SD card** (`/fs/microsd/log`) — moving parameters into
  internal flash only removes the SD dependency for *parameter storage*; logs are still written
  to the card.
- Firmware ceiling is **1792 KB**: `openbl_h743fp_dds` has ~76 KB free; `openbl_h743fp_default` has only ~2.2 KB (99.88%) — check headroom before adding features.
- PX4 board-config pitfalls:
  - `default.px4board` is the **base** and `<variant>.px4board` **overrides** it — toggling a
    feature only in the base has no effect; edit the variant file (or both).
  - `landing_target_estimator` defines the `LTEST_*` parameters that `local_position_estimator`
    references — mind the dependency direction (cutting LPE alone is fine; cutting
    landing_target_estimator alone is not).
  - After touching `timer_config.cpp` you must `rm -rf build/<target>`, or QGC's actuator metadata
    will be stale.

---

© 2026 洋洋柚 (@zhangyangyangyou) · OpenblH743VIT6

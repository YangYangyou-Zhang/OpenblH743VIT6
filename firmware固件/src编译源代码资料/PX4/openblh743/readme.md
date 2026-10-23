# OpenblH743VIT6 — Project Background & Build/Flash Instructions

## Project Background

- Custom flight controller, main controller **STM32H743VIT6**, **Board ID = 18848**.
- Runs **PX4 firmware**.
- Shares the **APM bootloader** with ArduPilot (PX4's bundled NuttX bootloader corrupts flash, so it has been deprecated).

Sensors:

| Peripheral | Model | Bus | Notes |
|------|------|------|------|
| IMU0 | MPU6500 (or ICM-42605) | SPI2, CS PB12 | Polling mode; either chip can be fitted |
| IMU1 | MPU6500 (or ICM-42605) | SPI3, CS PD7 | Either chip can be fitted |
| IMU2 (optional) | MPU6500 or ICM-42605 | SPI3, CS PD5 | ICM driver compiled, **not started** (see below) |
| Barometer | SPL06 | I2C (0x76/0x77 auto-scan) | |
| Compass | QMC5883P | I2C1 (0x2C) | Driver rotation `-R 10`; keep `CAL_MAG0_ROT` = **-1** (see below) |
| Battery | ADC1 ch10/11 | PC0(voltage)/PC1(current) | Voltage channel has a pull-down resistor; reading ~0 with no battery connected is normal |
| CAN | UAVCAN/DroneCAN | PB8(RX)/PB9(TX) | 1 Mbps, enabled on main board |

Parameters are stored on the **SD card** (`CONFIG_BOARD_PARAM_FILE=/fs/microsd/params`).

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
- SPI3 is the only IMU bus with DMA enabled (`CONFIG_STM32H7_SPI3_DMA`); SPI2 (IMU0) runs in
  polling mode.

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

1. Toolchain must be **GCC 10.2.1** (`arm-none-eabi-10-2020-q4-major`). H7 code generated by GCC 13.x will not boot.
2. Execute inside the PX4 source tree.

**Two variants** (flash is only 1920KB; CAN and DDS are mutually exclusive — both cannot be enabled at once):

| Variant | Command | Config |
|------|------|------|
| Main board (with CAN) | `make openbl_h743_default` | `default.px4board`: UAVCAN enabled, UXRCE_DDS disabled |
| DDS variant | `make openbl_h743_dds` | `dds.px4board`: UXRCE_DDS enabled, UAVCAN disabled |

> Note: When switching variants, if there is CMake cache residue (e.g. the DDS variant still carrying UAVCAN), first `rm -rf build/openbl_h743_dds` (or the corresponding variant directory) before rebuilding, to ensure a clean configuration.

3. Artifacts:
   - `build/openbl_h743_<label>/openbl_h743_<label>.px4`
   - `build/openbl_h743_<label>/openbl_h743_<label>.bin`

> Local archive naming: `<target>_1.17.0-alpha1.hex` (bootloader included) / `<target>_1.17.0-alpha1.px4`.

## Merge bootloader + firmware (merged hex)

Use arm-none-eabi-objcopy to merge the bootloader (`0x08000000`) and firmware (`0x08020000`) into a single hex for one-shot ST-Link flashing:

```bash
O=arm-none-eabi-objcopy
$O -I binary -O ihex --change-addresses 0x08000000 openbl_h743_bootloader.bin /tmp/bl.hex
$O -I binary -O ihex --change-addresses 0x08020000 openbl_h743_default.bin /tmp/fw.hex
grep -v ':00000001FF' /tmp/bl.hex > openbl_h743_merged.hex
cat /tmp/fw.hex >> openbl_h743_merged.hex
```

## Flash

- Use **ST-Link** to flash `openbl_h743_merged.hex` (bootloader + firmware flashed in one go).
- Flash layout:

| Region | Address | Size |
|------|------|------|
| Bootloader | 0x08000000 | 128 KB |
| Firmware | 0x08020000 | 1920 KB |

## Peripheral/Interface Status (2026-08-29)

- **Compass classification (2026-10-19)**: this PX4 version has **no `CAL_MAGx_EXTERNAL` parameter** — a compass's internal/external status is derived from the I2C bus it is on. `i2c.cpp` marks **both I2C1 and I2C2 as external** (so `-X` scans everything), while a board-level override of `px4_i2c_device_external()` reports the **onboard QMC5883P (I2C1, 0x2C) as internal**. This is what lets PX4 use it as the reference when auto-detecting the orientation of **external** magnetometers (`mag_calibration.cpp` derives an external compass's rotation by comparing it against the internal one).
- **Compass orientation (2026-10-19)**: the onboard compass's mounting offset is baked into the **driver default rotation** — `qmc5883p -X -b 1 -R 10 start` (`ROTATION_ROLL_180_YAW_90`). Therefore **`CAL_MAG0_ROT` must stay at its default `-1`**; do not set YAW 180 manually.
  - ⚠️ Once `CAL_MAG0_ROT` holds a valid value (e.g. after running compass calibration) it **overrides** the driver `-R`; set it back to `-1` after calibration.
  - To change the flight controller's mounting orientation, adjust **`SENS_BOARD_ROT`** (the IMUs also have `CAL_ACC/GYRO_ROT = -1` and follow it), not `CAL_MAG0_ROT`.
- **Serial assignment (2026-08-29)**: `RC` uses `CONFIG_BOARD_SERIAL_RC` (PX4 does **not** recognize `RC_INPUT`; RC actually binds via `RC_SERIAL_PORT` in `board_config.h` → `/dev/ttyS5` = USART6 = RCIN PC6/PC7); added `URT6=/dev/ttyS6` (=UART7 generic serial, PE8/PE7) and `TEL3=/dev/ttyS7` (=UART8, PE1/PE0); swapped `GPS2`/`TEL2` (hardware: **USART3 is GPS2, UART4 is TEL2** → GPS2=/dev/ttyS2, TEL2=/dev/ttyS3). Serial functions are defined by `CONFIG_BOARD_SERIAL_*` in `default.px4board`/`dds.px4board` (valid names: `URT6`/`GPS1~5`/`TEL1~5`/`RC`/`WIFI`/`EXT2`); QGC only lists ports the firmware defines.
- **USB device name**: `OpenblH743VIT6` (renamed; see `CDCACM_PRODUCTSTR/VENDORSTR` in `nuttx-config/nsh/defconfig` and `firmware.prototype`).
- **Safety switch PB2**: enabled the `safety_button` driver (`GPIO_BTN_SAFETY`, active-high when pressed connects to 3.3V, configured with `GPIO_PULLDOWN`, `safety_button start`).
- **Buzzer**: removed (disabled `CONFIG_DRIVERS_TONE_ALARM`). The original PB0 passive buzzer shared TIM3 with PWM11; tone_alarm requires exclusive timer access to change frequency and conflicts with PWM, so the buzzer was eventually dropped and PB0 was reassigned to PWM12.
- **AT7456E OSD (2026-09-05)**: added to the firmware (`CONFIG_DRIVERS_OSD_ATXXXX`). Hardware: SPI1 = PA5(SCK)/PA6(MISO)/PA7(MOSI), CS = PC4 (matches APM hwdef). Board changes: `GPIO_SPI1_*` defines in `nuttx-config/include/board.h`, `CONFIG_STM32H7_SPI1` in `nuttx-config/nsh/defconfig`, an SPI1 bus entry in `src/spi.cpp` (`DRV_OSD_DEVTYPE_ATXXXX`, CS PC4), and `init/rc.board_extras` runs `atxxxx start -s` when `OSD_ATXXXX_CFG` != 0 (`-s` = internal SPI bus). Usage: set **`OSD_ATXXXX_CFG` = 1 (NTSC) or 2 (PAL)** in QGC, then reboot.

## VTOL / Fixed-wing support (2026-10-22)

The firmware includes PX4's fixed-wing control chain:

- `fw_mode_manager`: FW modes (Position/Altitude/Manual/Stabilized/Acro), takeoff/landing, launch detection, runway takeoff
- `fw_lat_lon_control`: TECS, lateral/longitudinal control, NPFG path following

`rc.vtol_apps` starts both modules automatically; the VTOL transition switch is `RC_MAP_TRANS_SW`.
Note: this vehicle has **no airspeed sensor** — the tailsitter front transition completes on attitude/time (not `VT_ARSP_TRANS=10 m/s`) and TECS runs without airspeed. Add a pitot and set `VT_FW_MIN_ALT` / `VT_QC_ALT_LOSS` before the first FW flight.

## Precision landing (2026-10-23)

`landing_target_estimator` (LTE) is kept in the firmware (~3.4 KB) for a future companion computer;
the navigator `AUTO_PRECLAND` state machine is also complete. No onboard target sensor is fitted,
so precision landing is inert until a target source is added.

| Target input | Path | LTE needed |
|---|---|---|
| MAVLink `LANDING_TARGET`, `position_valid=1`, `frame=MAV_FRAME_LOCAL_NED` | published directly as `landing_target_pose` | no |
| `LANDING_TARGET` with angles (`position_valid=0`) or an IRLock | `irlock_report` → LTE → `landing_target_pose` | yes |

- Trigger: mission land item with `land_precision` 1 (opportunistic) or 2 (required),
  `RTL_PLD_MD` = 1/2, or `commander mode auto:precland`.
- Defaults: `PLD_SRCH_ALT=10 m`, `PLD_SRCH_TOUT=10 s`, `PLD_BTOUT=5 s`, `PLD_HACC_RAD=0.2 m`,
  `PLD_FAPPR_ALT=0.1 m`.
- Without a target it searches at `PLD_SRCH_ALT` and then falls back to a normal landing.

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

## Known Notes

- **SPI2 (IMU0) has a DMA conflict**; `CONFIG_STM32H7_SPI2_DMA` is currently temporarily disabled in favor of polling mode. Functionally correct but slightly heavier on CPU; DMA can be re-enabled once the DMA stream allocation is resolved.
- SPI pins (treat APM `hwdef.dat` as the hardware source of truth):
  - SPI1: PA5(SCK) / PA6(MISO) / PA7(MOSI) / PC4(CS) — AT7456E OSD
  - SPI2: PB13(SCK) / PB14(MISO) / PB15(MOSI) / PB12(CS)
  - SPI3: PB3(SCK) / PB4(MISO) / PD6(MOSI); PD7 = IMU CS, PD5 = IMU CS (both accept MPU6500 or ICM-42605)
- **PWM mapping** (`timer_config.cpp`):
  - PWM 1-4 = TIM1 CH1-4: PE9/PE11/PE13/PE14
  - PWM 5-8 = TIM4 CH1-4: PD12/PD13/PD14/PD15
  - PWM 9-10 = TIM15: PE5/PE6
  - PWM 11 = TIM3 CH4: PB1
  - PWM 12 = TIM3 CH3: PB0
- **Flash limit 1920KB**: CAN (UAVCAN) is about 142KB, DDS (UXRCE_DDS) about 65KB; both cannot be enabled simultaneously. The main board has CAN, the DDS variant has DDS. Current usage (with the FW/VTOL modules, the optional ICM-42605 driver and all 14 magnetometer drivers): default **93.22%** (1,832,884 B, ~130 KB headroom), dds **89.20%** (1,753,708 B, ~207 KB headroom). To fit `fw_mode_manager` + `fw_lat_lon_control` (~45 KB), `SYSTEMCMDS_TESTS`, `GNSS_SEPTENTRIO`, `LOCAL_POSITION_ESTIMATOR`, `COMMON_TELEMETRY`, `TAP_ESC` and `DRIVERS_SMART_BATTERY_BATMON` are disabled; the CAN variant also disables `UXRCE_DDS_CLIENT`. The landing-target estimator is kept for future companion-computer precision landing.

---

© 2026 洋洋柚 (@zhangyangyangyou) · OpenblH743VIT6

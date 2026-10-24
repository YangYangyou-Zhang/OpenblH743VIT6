# Betaflight 4.5.3 barometer + compass patch

Local patch set for **Betaflight 4.5.3**, used by `OpenblH743VIT6` to support the onboard
**SPL06-001** barometer and **QMC5883P** magnetometer. Neither driver exists in Betaflight 4.5.3
upstream.

## Files

| File | Purpose |
|---|---|
| `drivers/barometer/barometer_spl06.c/.h` | **New** SPL06-001 driver (ported from INAV); probes **0x76 / 0x77** |
| `drivers/barometer/barometer_bmp280.c` | BMP280 address fallback to 0x77 (optional) |
| `sensors/barometer.h` | Adds `BARO_SPL06 = 12` |
| `sensors/barometer.c` | SPL06 include, default config, AUTO detection |
| `drivers/compass/compass_qmc5883p.c/.h` | **New** QMC5883P driver (ported from INAV), address **0x2C**, chip ID 0x80 |
| `sensors/compass.h` | Adds `MAG_QMC5883P = 10` |
| `sensors/compass.c` | QMC5883P include, I2C bus init, AUTO detection |
| `cli/settings.c` | Adds `"SPL06"` to `baro_hardware` and `"QMC5883P"` to `mag_hardware` |

## Apply

Copy the files over the Betaflight 4.5.3 source tree with the same relative paths:

```bash
cp -r patches/drivers/barometer/* ../betaflight/src/main/drivers/barometer/
cp -r patches/drivers/compass/*   ../betaflight/src/main/drivers/compass/
cp -r patches/sensors/*           ../betaflight/src/main/sensors/
cp    patches/cli/settings.c      ../betaflight/src/main/cli/
```

`OpenblH743VIT6/config.h` must have:

```c
#define USE_BARO
#define USE_BARO_SPL06

#define USE_MAG
#define USE_MAG_QMC5883P
#define MAG_QMC5883P_ALIGN CW270_DEG_FLIP
```

`BARO_I2C_INSTANCE` and `MAG_I2C_INSTANCE` are already `I2CDEV_1`; both chips are on **I2C1
(PB6/PB7)**. SPL06 is at **0x77**, QMC5883P at **0x2C**.

## Build

```bash
make OpenblH743VIT6 \
  ARM_SDK_DIR=$HOME/gcc-arm-none-eabi-10-2020-q4-major \
  GCC_REQUIRED_VERSION=10.2.1 -j32
```

Output: `obj/betaflight_4.5.3_STM32H743_OpenblH743VIT6.hex`.

## Verify

Betaflight Configurator:

- `status` should list **Barometer** and **Compass**.
- If AUTO misses a device:
  ```
  set baro_hardware = SPL06
  set mag_hardware = QMC5883P
  save
  ```
- **Compass orientation**: the driver outputs raw sensor axes; the board alignment is set by
  `MAG_QMC5883P_ALIGN = CW270_DEG_FLIP` (matches PX4 `-R 10`, ROLL_180_YAW_90). Verify the
  heading in Configurator; if it is wrong, adjust `align_mag` (or `MAG_QMC5883P_ALIGN`) and
  re-run the compass calibration.
- Run the normal accelerometer/compass calibration after flashing.

## Note

These are modifications to Betaflight shared source. They are version-specific to **4.5.3** and
will be overwritten by a Betaflight upgrade — re-apply the patch after upgrading.

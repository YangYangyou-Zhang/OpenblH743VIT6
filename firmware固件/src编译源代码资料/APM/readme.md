# OpenblH743VIT6 Flight Controller

STM32H743VIT6 based ArduPilot flight controller hardware definition.
Firmware version: **ArduPilot Copter-4.7.1 / Plane-4.7.1**

## Hardware Configuration

| Feature | Detail |
|---------|--------|
| MCU | STM32H743VIT6 |
| IMU0 | MPU6500 @ SPI2, ROTATION_ROLL_180 |
| IMU1 | MPU6500 @ SPI3, ROTATION_ROLL_180 |
| Barometer | BMP280 / SPL06 @ I2C1 (0x76, 0x77) |
| OSD | AT7456E @ SPI1 |
| Storage | microSD @ SDMMC1 |
| Serial 0 | USB OTG1 |
| Serial 1 | USART1 (GPS0) |
| Serial 2 | USART2 (MSP DisplayPort) |
| Serial 3 | USART3 (GPS1) |
| Serial 4 | UART4 (MAVLink2, 460 baud) |
| Serial 5 | USART6 (RCIN) |
| Serial 6 | UART7 (ESC Telemetry) |
| Serial 7 | UART8 (MAVLink2, 460 baud) |
| CAN | CAN1 @ PB8/PB9 |
| PWM outputs | 11 (TIM1 CH1-4, TIM4 CH1-4, TIM15 CH1-2, TIM3 CH4) |
| RGB LED | PE2(R) / PE3(G) / PE4(B) |
| Buzzer (passive) | PB0 TIM3_CH3 ALARM (ToneAlarm) |
| Safety switch | PB2 (default disabled, press to lock) |
| Compass | I2C2 (I2C:0), auto-probe (IST8310 etc.) |

## Battery ADC

- Voltage divider: 51k / 1k (scale = 52.0)
- Voltage sense: PC0 (ADC1_IN10, pin 10)
- Current sense: PC1 (ADC1_IN11, pin 11)

### ADC Offset Calibration

The STM32H743 ADC has a ~1.78V offset when no battery is connected. Due to this nonlinear offset, a fixed value must be subtracted from the reading.

Since ArduPilot's hwdef layer does not provide a voltage offset parameter (only multiplicative scale), either:

- Calibrate via Mission Planner's battery calibration page after flashing
- Or add an offset in the analog source code if precise readings are required without ground-station calibration

Expected calibration: `BATT_VOLT_OFFSET = 1.78`

## Build Requirements

- GCC 10.2.1 (`gcc-arm-none-eabi-10-2020-q4-major`)
- Python 3.x with required packages
- GCC 13.x produces non-bootable code for STM32H7 — do not use

## Build Commands

```bash
# Set GCC 10 path
export PATH=/path/to/gcc-arm-none-eabi-10-2020-q4-major/bin:$PATH

# Build bootloader
./waf distclean
./waf configure --board OpenblH743VIT6 --bootloader
./waf clean
./waf bootloader

# Build ArduCopter
./waf configure --board OpenblH743VIT6
./waf copter

# Build ArduPlane
./waf plane
```

## Output Files

| File | Use |
|------|-----|
| `AP_Bootloader.hex` | Bootloader for ST-LINK |
| `arducopter_with_bl.hex` | ArduCopter + bootloader (ST-LINK) |
| `arducopter.apj` | ArduCopter (Mission Planner update) |
| `arduplane_with_bl.hex` | ArduPlane + bootloader (ST-LINK) |
| `arduplane.apj` | ArduPlane (Mission Planner update) |

## Flashing

### First time (ST-LINK)
Use STM32CubeProgrammer to flash `arducopter_with_bl.hex` or `arduplane_with_bl.hex`.

### Update via Ground Station
Upload `arducopter.apj` or `arduplane.apj` via Mission Planner firmware update page.

---

© 2026 洋洋柚 (@zhangyangyangyou) · OpenblH743VIT6

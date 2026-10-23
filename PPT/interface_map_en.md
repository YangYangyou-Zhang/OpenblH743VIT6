# OpenblH743VIT6 / MINI Interface Map

Firmware: PX4 v1.17.0-alpha1, ArduPilot Copter/Plane 4.7.1, Betaflight 4.5.3, INAV 9.0.1.

Annotated images (in `PPT/接口标注/`):

- [H743VIT6 top](接口标注/H743VIT6_top_标注.png)
- [H743VIT6 bottom](接口标注/H743VIT6_back_标注.png)
- [H743MINI top](接口标注/H743MINI_top_标注.png)
- [H743MINI bottom](接口标注/H743MINI_back_标注.png)

## 1. Serial ports

| USART | TX / RX pins | PX4 | ArduPilot | Betaflight / INAV | Notes |
|---|---|---|---|---|---|
| USART1 | PA9 / PA10 | **GPS1** (`/dev/ttyS0`) | SERIAL1 | UART1 | VIT6: back 6-pin U10 |
| USART2 | PA2 / PA3 | **TELEM1** (`/dev/ttyS1`) | SERIAL2 | UART2 | Shared with VTX / receiver combo |
| USART3 | PD8 / PD9 | **GPS2** (`/dev/ttyS2`) | SERIAL3 | UART3 | VIT6: front 10-pin silkscreened "GPS1" |
| UART4 | PA0 / PA1 | **TELEM2** (`/dev/ttyS3`) | SERIAL4 | UART4 | |
| USART6 | PC6 / PC7 | **RC** (`/dev/ttyS5`) | SERIAL5 | UART6 | Receiver / SBUS / CRSF |
| UART7 | PE8 / PE7 | **URT6** (`/dev/ttyS6`) | SERIAL6 | UART7 | ESC telemetry / general UART |
| UART8 | PE1 / PE0 | **TELEM3** (`/dev/ttyS7`) | SERIAL7 | UART8 | |

APM: `SERIALn` parameters; `SERIAL0` is USB.
BF/INAV: UART number equals the STM32 USART number; the INAV target defaults `SERIALRX_UART = USART6` (CRSF).

### GPS1 / GPS2 naming on the VIT6

| Physical port | USART | PX4 | ArduPilot | BF/INAV |
|---|---|---|---|---|
| Front 10-pin silkscreened **GPS1** (CN3) | USART3 | **GPS2** | SERIAL3 | UART3 |
| Back 6-pin (U10) | USART1 | **GPS1** | SERIAL1 | UART1 |

**PX4 GPS1 is the back 6-pin U10; the front 10-pin "GPS1" is PX4 GPS2.** The MINI is unambiguous
(pins are labelled TX1/RX1 and TX3/RX3).

## 2. VIT6 connectors

### 2.1 Front

| # | Silkscreen | Ref | Pins | USART | PX4 | APM | BF/INAV | Notes |
|---|---|---|---|---|---|---|---|---|
| 1 | GPS1 (10-pin) | CN3 | 4V3, TX3, RX3, SCL2, SDA2, SAFETY_SW, NC, 3V3, BEEP, GND | USART3 | **GPS2** | SERIAL3 | UART3 | Compass I2C2, safety switch, buzzer |
| 2 | TELEM1 / VTX | U13 | TX2, RX2, VID_OUT, GND, VCC | USART2 | **TELEM1** | SERIAL2 | UART2 | Shared with DJI VTX video |
| 3 | TELEM2 | CN6 | +5V, TX4, RX4, NC, NC, GND | UART4 | **TELEM2** | SERIAL4 | UART4 | 5 V |
| 4 | TELEM3 | CN7 | +5V, TX8, RX8, NC, NC, GND | UART8 | **TELEM3** | SERIAL7 | UART8 | 5 V |
| 5 | TELEM/RC | CN8 | 4V3, TX6, RX6, NC, NC, GND | USART6 | **RC** | SERIAL5 | UART6 | 4V3 |
| 6 | SBUS | CN5 | 4V3, RX6, NC, NC, GND | USART6 | **RC** | SERIAL5 | UART6 | RX only |
| 7 | RX7/TX7/GND | 3-pin | RX7, TX7, GND | UART7 | **URT6** | SERIAL6 | UART7 | ESC telemetry |
| 8 | CAN | CN2 | +5V, CANH, CANL, GND | — | CAN1 | CAN1 | — | MINI has no CAN |
| 9 | I2C1 | CN4 | 4V3, GND, SCL1, SDA1 | — | I2C1 | — | — | External compass |
| 10 | Power module | — | GND, VOE, CUR, 4V3, 5V | — | ADC | — | — | |
| 11 | USB | — | DP, DM, 5V, GND | — | OTG1 | SERIAL0 | — | |
| 12 | PWM | — | PWM1-11 (FP: 1-10), 5V, GND | — | — | — | — | FP variant: PB0 = buzzer |

### 2.2 Bottom

| # | Silkscreen | Ref | Pins | USART | PX4 | APM | BF/INAV | Notes |
|---|---|---|---|---|---|---|---|---|
| 1 | (none, 6-pin) | U10 | 4V3, GND, RX1, TX1, SDA2, SCL2 | USART1 | **GPS1** | SERIAL1 | UART1 | Compass I2C2 |
| 2 | Receiver + telemetry (6-pin) | U15 | RX6, GND, RX2, TX2, GND, VCC | USART6 RX + USART2 | **RC + TELEM1** | SERIAL5 + SERIAL2 | UART6 + UART2 | |
| 3 | RC / telemetry (4-pin) | U12 | 4V3, GND, RX6, TX6 | USART6 | **RC** | SERIAL5 | UART6 | |
| 4 | Analog camera | U16 | VID_IN, GND, 5V | — | — | — | — | |
| 5 | USB | — | DP, DM, 5V, GND | — | OTG1 | SERIAL0 | — | |

USART2 and USART6 are paralleled across several physical connectors — connect only one device per USART.

## 3. MINI connectors

### 3.1 Top

| # | Silkscreen | Ref | Pins | USART | PX4 | APM | BF/INAV |
|---|---|---|---|---|---|---|---|
| 1 | GPS1 (4-pin) | U9 | 4V3, GND, RX1, TX1 | USART1 | **GPS1** | SERIAL1 | UART1 |
| 2 | GPS2 (6-pin) | U10 | 4V3, GND, RX3, TX3, SDA2, SCL2 | USART3 | **GPS2** | SERIAL3 | UART3 |
| 3 | TELEM1 / VTX (5-pin) | — | TX2, RX2, VID_OUT, GND, VCC | USART2 | **TELEM1** | SERIAL2 | UART2 |
| 4 | TELEM2 (4-pin) | U11 | 4V3, GND, RX4, TX4 | UART4 | **TELEM2** | SERIAL4 | UART4 |
| 5 | RC (4-pin) | U14 | 4V3, GND, RX6, TX6 | USART6 | **RC** | SERIAL5 | UART6 |
| 6 | Receiver + telemetry (6-pin) | U17 | RX6, GND, RX2, TX2, GND, VCC | USART6 RX + USART2 | **RC + TELEM1** | SERIAL5 + SERIAL2 | UART6 + UART2 |
| 7 | I2C1 (4-pin) | U8 | 4V3, GND, SDA1, SCL1 | — | I2C1 | — | — |
| 8 | Camera (3-pin) | — | CAM/VID_IN, GND, 4V3 | — | — | — | — |

### 3.2 Bottom

| # | Silkscreen | Ref | Pins | USART | PX4 | APM | BF/INAV | Notes |
|---|---|---|---|---|---|---|---|---|
| 1 | 4-in-1 ESC S1-S4 (8-pin) | U16 | VCC, GND, CUR, RX7, PWM1-4 | UART7 RX | **URT6** | SERIAL6 | UART7 | Current sense; RX7 = ESC telemetry |
| 2 | TELEM3 (4-pin) | U12 | 4V3, GND, RX8, TX8 | UART8 | **TELEM3** | SERIAL7 | UART8 | |
| 3 | USB | — | DP, DM, 5V, GND | — | OTG1 | SERIAL0 | — | |
| 4 | 4-in-1 ESC S5-S8 (8-pin) | U19 | VCC, GND, NC, RX7, PWM5-8 | UART7 RX | **URT6** | SERIAL6 | UART7 | RX7 = ESC telemetry |

The MINI has **no CAN** connector and only RX7 of UART7 (ESC telemetry input).

## 4. Quick wiring

| Purpose | VIT6 | MINI |
|---|---|---|
| PX4 GPS1 | Back U10 (6-pin) | Top U9 (4-pin) |
| PX4 GPS2 | Front "GPS1" 10-pin | Top U10 (6-pin) |
| PX4 TELEM1 | Front TELEM1/VTX | Top TELEM1/VTX |
| PX4 TELEM2 | Front TELEM2 | Top U11 |
| PX4 TELEM3 | Front TELEM3 | Bottom U12 |
| PX4 RC / receiver | Front TELEM/RC or SBUS | Top U14 or U17 |
| PX4 URT6 / ESC telemetry | Front RX7/TX7 | Bottom ESC connectors RX7 (S1-S4 / S5-S8) |
| External compass | Front I2C1 (CN4) | Top U8 |
| CAN | Front CN2 | — |

*Based on the Rev 1.0 schematic and the 2026-10-23 firmware configuration.*

# OpenblH743VIT6 — PX4 固件

STM32H743VIT6 飞控，Board ID = 18848。

## 引脚定义

### SPI

| 总线 | 功能 | SCK | MISO | MOSI | CS |
|------|------|-----|------|------|----|
| SPI1 | OSD (AT7456E) | PA5 | PA6 | PA7 | PC4 |
| SPI2 | IMU0 (MPU6500) | PB13 | PB14 | PB15 | PB12 |
| SPI3 | IMU1 (MPU6500) | PB3 | PB4 | PD6 | PD7 |

### I2C

| 总线 | SCL | SDA | 功能 |
|------|-----|-----|------|
| I2C1 | PB6 | PB7 | 气压计 (BMP280/SPL06) |
| I2C2 | PB10 | PB11 | 罗盘 (IST8310) |

### 串口

| 功能 | 硬件 | TX | RX |
|------|------|----|----|
| USB | OTG1 | PA11 (DM) | PA12 (DP) |
| GPS1 | USART1 | PA9 | PA10 |
| TEL1 | USART2 | PA2 | PA3 |
| GPS2 | USART3 | PD8 | PD9 |
| TEL2 | UART4 | PA0 | PA1 |
| RCIN | USART6 | PC6 | PC7 |
| URT6 | UART7 | PE8 | PE7 |
| TEL3 | UART8 | PE1 | PE0 |

> UART7 物理为 ESC 遥测口（PE8/PE7），PX4 中配置为通用串口 URT6（`/dev/ttyS6`）；如需 ESC 遥测：`tap_esc start -d /dev/ttyS6`。

### PWM

| 通道 | 定时器 | 引脚 |
|------|--------|------|
| 1-4 | TIM1 CH1-4 | PE9, PE11, PE13, PE14 |
| 5-8 | TIM4 CH1-4 | PD12, PD13, PD14, PD15 |
| 9-10 | TIM15 CH1-2 | PE5, PE6 |
| 11 | TIM3 CH4 | PB1 |
| 12 | TIM3 CH3 | PB0 |

### 其他

| 功能 | 引脚 |
|------|------|
| SD 卡 | SDMMC1: PC12(CK), PD2(CMD), PC8-11(D0-D3) |
| 安全开关 | PB2 |
| RGB LED | PE2(R), PE3(G), PE4(B) |
| CAN | PB8(RX), PB9(TX) |
| SWD | PA13, PA14 |
| ADC 电压 | PC0 (ADC1_IN10) |
| ADC 电流 | PC1 (ADC1_IN11) |

## Flash 布局

| 区域 | 地址 | 大小 |
|------|------|------|
| Bootloader | 0x08000000 | 128 KB |
| 固件 | 0x08020000 | 1920 KB |

## 编译

工具链需用 **GCC 10.2.1**（GCC 13.x 生成的 H7 代码无法启动）。

两个版本（flash 只有 1920KB，CAN 和 DDS 二选一）：

| 版本 | 命令 | 说明 |
|------|------|------|
| 主板（带 CAN） | `make openbl_h743_default` | UAVCAN 启用，UXRCE_DDS 关闭 |
| DDS 变体 | `make openbl_h743_dds` | UXRCE_DDS 启用，UAVCAN 关闭 |

```bash
cd PX4-Autopilot
make openbl_h743_default   # 或 make openbl_h743_dds
```

输出 `build/openbl_h743_<label>/openbl_h743_<label>.px4`。Board ID 18848，APM bootloader 通用。

完整编译/烧录流程（含 bootloader + 固件合并 hex）见 [编译与烧录说明.md](编译与烧录说明.md)。

## 传感器状态

当前已全部上线：IMU0（SPI2）、IMU1（SPI3）、SPL06 气压计（I2C1 0x77）、QMC5883P 罗盘（I2C1 0x2C）、电池 ADC、AT7456E OSD（SPI1，CS PC4）、CAN（UAVCAN，PB8/PB9）。

> 注：SPI2（IMU0）的 DMA 有冲突，目前临时关闭 DMA 走轮询模式；PWM 映射和 CAN 配置详见编译与烧录说明。

---

© 2026 洋洋柚 (@zhangyangyangyou) · OpenblH743VIT6

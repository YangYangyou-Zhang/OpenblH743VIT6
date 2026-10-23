# OpenblH743VIT6-FP — 参数存内部 flash 版

## 这个版本是什么

和 `../openblh743` 是**同一块硬件**（STM32H743VIT6，Board ID 18848，共用 APM bootloader）。
两个区别：

1. **参数存内部 flash，不依赖 SD 卡。**
2. **PB0 改回无源蜂鸣器，PB1 不再使用；PWM 输出从 12 路变成 10 路。**

### 为什么参数原本必须在 SD 卡上

原版 `openblh743` 用 `CONFIG_BOARD_PARAM_FILE=/fs/microsd/params`。
这不是设计偏好 —— 固件区 `FLASH ORIGIN=0x08020000 LENGTH=1920K` 把 2MB flash 占满，
**没有 128KB 扇区留给参数**，只能外挂到 SD。
代价：参数完全依赖 SD 卡 —— 卡不可用时就无法保存（dmesg 刷 `param export to /fs/microsd/params failed (-1)` /
`param auto save unavailable (-1)`），开机也一直报 `Missing FMU SD Card`。

本版启用 PX4 的 `FLASH_BASED_PARAMS` 后端，参数写在 flash 最后一个 128KB 扇区（0x081E0000）。
**没有 SD 卡也能正常读写和保存参数。**

> 注意：参数进 flash 只解决"丢参数"，**飞行日志仍然写 SD 卡**（`/fs/microsd/log`）。

## 硬件（与原版一致）

| 外设 | 型号 | 总线 | 说明 |
|------|------|------|------|
| IMU0 | MPU6500 或 ICM-42605 | SPI2，CS PB12 | 轮询模式；两种芯片均可贴装 |
| IMU1 | MPU6500 或 ICM-42605 | SPI3，CS PD7 | 两种芯片均可贴装 |
| IMU2（可选） | MPU6500 或 ICM-42605 | SPI3，CS PD5 | ICM 驱动已编译，**未启动**（见下文） |
| 气压计 | SPL06 | I2C（0x76/0x77 自动扫描） | |
| 罗盘 | QMC5883P | I2C1（0x2C） | 驱动默认旋转 `-R 10`（ROLL_180_YAW_90）；`CAL_MAG0_ROT` 保持 **-1**（见下文）|
| 电池 | ADC1 ch10/11 | PC0(电压)/PC1(电流) | 未接电池读数 ~0 正常 |
| OSD | AT7456E | SPI1（PA5/PA6/PA7，CS PC4） | 参数 `OSD_ATXXXX_CFG`：1=NTSC / 2=PAL |
| 蜂鸣器 | 无源 | **PB0（TIM3_CH3）** | `tone_alarm`，由 rcS 自动启动 |
| CAN | UAVCAN/DroneCAN | PB8(RX)/PB9(TX) | 仅 CAN 变体启用 |

### 可选的第二 IMU：ICM-42605（已编译，未启动）

`icm42605` 驱动已编入四个固件变体。MPU6500 和 ICM-42605（以及通过兼容补丁支持的 ICM-42688-P）
都注册在**全部三个 IMU 片选位置**：

| 总线 | CS | 已注册器件类型 |
|---|---|---|
| SPI2 | PB12 | MPU6500、ICM42605 |
| SPI3 | PD7 | MPU6500、ICM42605 |
| SPI3 | PD5 | MPU6500、ICM42605 |

`rc.board_sensors` 里原来的 MPU6500 启动保持不变。`icm42605` **没有**被启动（安装方向还需要实测）。
单独测试某个位置：

```
icm42605 start -s -b 3 -c 5  -R <旋转>   # PD5
icm42605 start -s -b 3 -c 7  -R <旋转>   # PD7
icm42605 start -s -b 2 -c 12 -R <旋转>   # PB12
```

不带 `-b`/`-c` 会遍历所有已注册的 ICM42605 设备；`-c` 按 CS 引脚号选择。

- 驱动接受 WHO_AM_I `0x42`（ICM-42605）和 `0x47`（ICM-42688-P，需下文所述的本地补丁）。插 42688P 时它运行在 **ICM-42605 兼容模式**（16-bit FIFO、复位默认滤波、不解析温度/FIFO 时间戳）；原生的 42688P 驱动未编译（约 +7.6 KB）。
- 不需要 DRDY 线：没有 DRDY 时驱动会退回定时读 FIFO。
- SPI3 是唯一启用 DMA 的 IMU 总线；SPI2（IMU0）为轮询模式。

## PWM 输出与蜂鸣器（与原版不同，重要）

**PWM 从 12 路变为 10 路**，PB0 让给蜂鸣器，PB1 不再使用：

| 输出 | 定时器 | 引脚 |
|------|--------|------|
| PWM 1-4 | TIM1 CH1-4 | PE9 / PE11 / PE13 / PE14 |
| PWM 5-8 | TIM4 CH1-4 | PD12 / PD13 / PD14 / PD15 |
| PWM 9-10 | TIM15 CH1-2 | PE5 / PE6 |
| （无 PWM 11/12） | — | PB1 不配任何功能，PB0 = 蜂鸣器 |

- **PB1**：保持复位默认状态，不分配给任何外设。
- **PB0**：无源蜂鸣器，`CONFIG_DRIVERS_TONE_ALARM=y`，`TONE_ALARM_TIMER 3` / `TONE_ALARM_CHANNEL 3`，
  `GPIO_TONE_ALARM = GPIO_TIM3_CH3OUT_1`（PB0, AF2）。静音用 QGC 参数 `CBRK_BUZZER`。
- **为什么必须动三处**：PX4 的 `ToneAlarmInterfacePWM.cpp` 有硬性检查 ——
  `TONE_ALARM_TIMER == 3` 时**必须不能开** NuttX 的 `CONFIG_STM32_TIM3`，否则直接 `#error`。
  这就是当初蜂鸣器和 PWM 抢 TIM3、最后放弃蜂鸣器的原因。本版的做法：
  1. `src/timer_config.cpp`：删掉 Timer3 的 io_timer 条目，以及 PB1(TIM3_CH4)/PB0(TIM3_CH3) 两个通道；
  2. `board_config.h`：`DIRECT_PWM_OUTPUT_CHANNELS` 12→10，加 `TONE_ALARM_*` / `GPIO_TONE_ALARM*` 定义，
     并把 `GPIO_TONE_ALARM_IDLE` 加进 `PX4_GPIO_INIT_LIST`；
  3. `nuttx-config/nsh/defconfig`：`CONFIG_STM32H7_TIM3=y` → 注释掉。
  这样 TIM3 整块归蜂鸣器独占。

> **改完 `timer_config.cpp` 必须 `rm -rf build/<target>` 干净重编**，否则 QGC 执行器页读到的
> 内嵌元数据（`actuators.json`，在 CMake configure 阶段生成）还是旧的路数。

## Flash 布局（和原版不同）

| 区域 | 地址 | 大小 |
|------|------|------|
| Bootloader | 0x08000000 | 128 KB |
| 固件 | 0x08020000 | **1792 KB**（原版是 1920 KB） |
| **参数扇区** | **0x081E0000** | **128 KB** |

- **固件上限从 1920KB 变成 1792KB**，加功能前先编译看余量。
- 参数扇区**不在合并 hex 的覆盖范围内**（固件只烧到约 0x081Dxxxx），
  所以**重刷固件不会擦掉已保存的参数**。

## 相比原版砍了什么

为容纳固定翼/VTOL 控制（`fw_mode_manager` + `fw_lat_lon_control`，约 45 KB）和可选的 ICM-42605 驱动（约 3.5 KB）并**保留全部 14 个磁罗盘驱动**，两个变体统一关闭：

| 砍掉 | 理由 |
|------|------|
| `SYSTEMCMDS_TESTS` | 台架自检命令集（约 112 KB） |
| `DRIVERS_GNSS_SEPTENTRIO` | Septentrio 专用 GPS 协议（通用 GPS 保留） |
| `MODULES_LOCAL_POSITION_ESTIMATOR` | 老旧 LPE（默认使用 EKF2） |
| `COMMON_TELEMETRY` | FrSky/HOTT/BST 专用遥测协议（MAVLink 不需要） |
| `DRIVERS_TAP_ESC` | TAP_ESC 专用电调协议（本机用 DShot） |
| `DRIVERS_SMART_BATTERY_BATMON` | SMBus 智能电池驱动（本机用模拟 ADC / INA238） |

- CAN（`default`）变体另关 `MODULES_UXRCE_DDS_CLIENT`（CAN 与 uXRCE-DDS 互斥）；dds 变体保留。
- `openbl_h743fp_default` 余量最小，额外关闭 `SYSTEMCMDS_SD_BENCH`、`SYSTEMCMDS_SD_STRESS`（SD 测速命令）。
- **全部 14 个磁罗盘驱动保留**；DShot + `ESC_BATTERY`（电调遥测）、UAVCAN、AT7456E OSD、SPL06、MPU6500、可选的 ICM-42605 驱动、VL53L1X、MS4525/SDP3X、INA238 均保留。
- 其余功能（airship、UWB、载荷投放、风速、云台+相机、ESC 电池、负载监控、`gyro_fft`、温度补偿、MC/FW 自整定、RPM、PPS 等）保留。着陆目标估计器（`landing_target_estimator`）保留，供后续机载电脑做精确着陆。

当前占用：

| 变体 | flash 占用 | 余量 |
|------|-----------|------|
| `openbl_h743fp_dds` | 1,757,264 B / 1792 KB = **95.76%** | ~76 KB |
| `openbl_h743fp_default`（CAN） | 1,832,760 B / 1792 KB = **99.88%** | **~2.2 KB** ⚠️ |

## 固定翼/VTOL 支持（2026-10-22）

固件已启用 PX4 固定翼控制链：

- `fw_mode_manager`：FW 模式管理（Position/Altitude/Manual/Stabilized/Acro）、起飞/降落、发射检测、跑道起飞
- `fw_lat_lon_control`：TECS、横纵向控制、NPFG 路径跟随

`rc.vtol_apps` 会自动启动这两个模块；VTOL 转换开关为 `RC_MAP_TRANS_SW`。
注意：本机**没有空速计**，尾座式前转换按姿态/时间完成（不等 `VT_ARSP_TRANS=10 m/s`），TECS 走无空速模式；首次固定翼飞行前建议加空速计，并设置 `VT_FW_MIN_ALT`、`VT_QC_ALT_LOSS` 等保护参数。

## LED 指示（2026-10-22）

硬件：PE2 = R、PE3 = G、PE4 = B（低电平点亮）。PX4 的颜色码是 `BLUE=0, RED=1, SAFETY=2, GREEN=3`，
`led.c` 的 `g_ledmap` 必须按这个顺序排列（旧版写成了 G/B/R，导致蓝色逻辑点亮了绿灯）。

当前行为（为兼容只有单颗红灯的硬件版本，已把解锁状态和过载的颜色对调）：

| 状态 | LED |
|---|---|
| 未通过预检（禁止解锁） | **红灯 10 Hz 快闪** |
| 可解锁 | **红灯 1 Hz 慢闪** |
| 已解锁、正常 | **红灯常亮**（`BOARD_ARMED_LED = LED_RED`） |
| 已解锁 + failsafe | 红灯 2 Hz |
| 过载/错误 | **蓝灯 10 Hz** |

## 精确着陆（2026-10-23）

固件保留了 `landing_target_estimator`（LTE，约 3.4 KB）和 navigator 的 `AUTO_PRECLAND` 状态机，
供后续加机载电脑使用。目前板上没有任何目标传感器，也没有接入目标源，所以该功能暂时是“已编译、无输入”。

| 目标输入 | 路径 | 是否经过 LTE |
|---|---|---|
| MAVLink `LANDING_TARGET`，`position_valid=1`、`frame=MAV_FRAME_LOCAL_NED` | 直接发布为 `landing_target_pose` | 不需要 |
| 角度式 `LANDING_TARGET`（`position_valid=0`）或 IRLock | `irlock_report` → LTE → `landing_target_pose` | 需要 |

- 触发方式：任务里的 Land 项设置 `land_precision=1`（机会式）/ `2`（强制式）；或 `RTL_PLD_MD=1/2`；
  或 `commander mode auto:precland`。
- 默认参数：`PLD_SRCH_ALT=10 m`、`PLD_SRCH_TOUT=10 s`、`PLD_BTOUT=5 s`、`PLD_HACC_RAD=0.2 m`、`PLD_FAPPR_ALT=0.1 m`。
- 没有目标时会先在搜索高度盘旋寻找，超时后回退普通降落。

## 本地对 PX4 公共代码的改动

除板级目录 `boards/openbl/` 外，这棵源码树还包含三处对 **PX4 公共代码**的本地修改。
它们不在上游，`git pull` / `git checkout` 会覆盖，升级前请先备份。
这三处修改后的文件已收录在 `../shared_src/`（目录结构与 PX4 源码树一致），可直接覆盖到对应位置。

### 1. VL53L1X 激光测距驱动

文件：`src/drivers/distance_sensor/vl53l1x/vl53l1x.cpp`

| 项 | 原值 | 本版 |
|---|---|---|
| `VL53L1X_DELAY`（轮询间隔） | 50000 µs | **20000 µs** |
| `VL53L1X_SAMPLE_RATE`（timing budget） | 200 ms | **100 ms** |
| `VL53L1X_INTER_MEAS_MS`（测量周期） | 250 ms | **110 ms** |
| 信号质量字段 | 把 ST range status 直接写入 `quality` | **`rangeStatus == 0 → 100`，否则 `0`** |

- **采样率**约 4 Hz → **约 9 Hz**；**量程** 4 m → **约 2.5~3 m**
- 本版修正了上游驱动的一处字段映射：I2C 通信状态（成功恒为 0）被写进了 `quality`，而 PX4 规定
  `quality == 0` 表示"信号无效"，会丢弃全部有效测距。修正后 EKF 才会真正使用测距数据。
- 启用方式：

  ```
  param set SENS_EN_VL53L1X 1     # 重启生效
  param set EKF2_RNG_CTRL 2       # 始终参与高度融合
  ```

### 2. uXRCE-DDS 的 microcdr 仓库路径

文件：`src/modules/uxrce_dds_client/Micro-XRCE-DDS-Client/cmake/SuperBuild.cmake`

`GIT_REPOSITORY` 由硬编码的 build 目录改为 `$ENV{HOME}/gitcache/microcdr`（Micro-CDR v2.0.1
的本地克隆），否则清理 build 后编译 dds 目标会因 `Failed to clone repository` 失败。
缓存可以这样创建：

```bash
git clone --branch v2.0.1 https://github.com/eProsima/Micro-CDR.git ~/gitcache/microcdr
```

或者把路径改成你自己的副本。**这是子模块内的改动，`git submodule update` 会冲掉。**

### 3. ICM-42605 驱动兼容 ICM-42688-P

文件：`src/drivers/imu/invensense/icm42605/ICM42605.cpp`、
`src/drivers/imu/invensense/icm42605/InvenSense_ICM42605_registers.hpp`

在 `probe()` 和复位后的检查中都额外接受 `WHO_AM_I = 0x47`（ICM-42688-P）。驱动其余部分不变，
因此 42688P 完全按 42605 的方式驱动：

- 16-bit FIFO（packet 3）、8 kHz ODR、±2000 dps / ±16 g；
- 不使用 20-bit hires，不配置 AAF/陷波滤波，不解析温度和 FIFO 时间戳；
- `INTF_CONFIG0/1` 和滤波寄存器依赖芯片复位默认值。

**这是兼容性 hack，不是原生驱动。** 飞行前必须台架验证加速度计/陀螺数据（重力轴 1 g、静止零偏、
旋转方向）。补丁文件已放入 `../shared_src/`；`git pull` / `git checkout` 会覆盖它们。

## 编译

1. 工具链必须用 **GCC 10.2.1**（`arm-none-eabi-10-2020-q4-major`）。GCC 13.x 生成的 H7 代码无法启动。
2. 命令与产物：

```bash
make openbl_h743fp_dds        # DDS 变体（uXRCE-DDS）
make openbl_h743fp_default    # CAN 变体（UAVCAN/DroneCAN）
# 产物：build/openbl_h743fp_<label>/openbl_h743fp_<label>.{px4,bin}
```

> 本地归档命名：`<target>_1.17.0-alpha1.hex`（已合并 bootloader）/ `<target>_1.17.0-alpha1.px4`。

3. 板级目录 `boards/openbl/h743fp/`，**独立于 `boards/openbl/h743/`**（改一个不影响另一个）。
   与原版的差异：

| 文件 | 改动 |
|------|------|
| `src/board_config.h` | `#define FLASH_BASED_PARAMS`；`DIRECT_PWM_OUTPUT_CHANNELS` 12→10；`TONE_ALARM_*`/`GPIO_TONE_ALARM*` 定义；`GPIO_TONE_ALARM_IDLE` 进 init list |
| `src/timer_config.cpp` | 移除 Timer3：PB1/PB0 两路 PWM 删掉 |
| `nuttx-config/scripts/script.ld` | `FLASH ... LENGTH = 1920K` → **1792K** |
| `nuttx-config/nsh/defconfig` | `CONFIG_STM32H7_TIM3` 关闭；`CONFIG_ARCH_BOARD_CUSTOM_DIR` 指向 `h743fp` |
| `default.px4board` / `dds.px4board` | 删除 `CONFIG_BOARD_PARAM_FILE`；加 `CONFIG_DRIVERS_TONE_ALARM=y`；CAN 版另有 3 项精简 |

## 合并 bootloader + 固件（merged hex）

```bash
O=arm-none-eabi-objcopy
$O -I binary -O ihex --change-addresses 0x08000000 openbl_h743_bootloader.bin /tmp/bl.hex
$O -I binary -O ihex --change-addresses 0x08020000 openbl_h743fp_dds.bin       /tmp/fw.hex
grep -v ':00000001FF' /tmp/bl.hex > openblh743fp_dds_merged.hex
cat /tmp/fw.hex >> openblh743fp_dds_merged.hex
```

（CAN 版把 `dds` 换成 `default`。）

## 烧录

- 用 **ST-Link** 刷 `openblh743fp_dds_merged.hex` 或 `openblh743fp_default_merged.hex`。
- **从原版（SD 存参数）首次切到本版时，建议先在 STM32CubeProgrammer 里做全片擦除**：
  原版固件一直用到 0x08200000，参数扇区里残留着旧固件字节，全清一遍最干净。
- 之后正常烧录即可，参数不会被擦。

## 验证

**参数落盘**：开机改一个参数（QGC 或 NSH `param set`）→ 断电重开 → 值还在。
（原版没有 SD 卡时这一步必丢。）

**蜂鸣器**：开机应有启动音（`tone_alarm` 的 startup tune）。没声音依次查：
`param show CBRK_BUZZER`（是否被静音）、NSH 里 `tone_alarm status`、PB0 是否确实接到无源蜂鸣器。

**PWM 路数**：QGC 执行器页应显示 `MAIN 1-4` / `MAIN 5-8` / `MAIN 9-10`（共 10 路），不再有 MAIN 11/12。

## 外设/接口状态

除 PWM/蜂鸣器外与原版一致，详见 `../openblh743/readme_zh.md`：罗盘驱动旋转 `-R 10`、`CAL_MAG0_ROT` 保持 -1、
串口分配（RC=ttyS5/USART6、GPS2=ttyS2、TEL2=ttyS3、URT6=ttyS6、TEL3=ttyS7）、
AT7456E 用法、SPI2 DMA 关闭等。GPIO 表见 `../openblh743/gpio_zh.md`（硬件相同，但 PB0/PB1 用途已变）。

## 已知注意事项

- **飞行日志仍依赖 SD 卡**（写 `/fs/microsd/log`）—— 参数搬进内部 flash 后，只有"参数存储"
  不再需要 SD 卡，日志照旧写在卡上。
- 固件区上限 **1792KB**：`openbl_h743fp_dds` 剩约 76 KB，`openbl_h743fp_default` 只剩约 2.2 KB（99.88%），加功能前先编译看余量。
- PX4 板级配置的坑：
  - `default.px4board` 是**基底**、`<变体>.px4board` **覆盖**它 —— 只改基底里的功能开关无效，
    必须改变体文件（或两个都改）。
  - `landing_target_estimator` 定义了 `LTEST_*` 参数并被 `local_position_estimator` 引用，
    要砍必须注意依赖顺序（可以只砍 LPE，不能只砍 landing_target_estimator）。
  - 动 `timer_config.cpp` 后必须 `rm -rf build/<target>` 重编，否则 QGC 执行器元数据是旧的。

---

© 2026 洋洋柚 (@zhangyangyangyou) · OpenblH743VIT6

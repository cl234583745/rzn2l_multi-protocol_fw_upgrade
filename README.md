# RZN2L 多协议固件升级系统

## 1. 开发环境要求

| 组件 | 版本要求 |
|------|----------|
| 操作系统 | Windows 10/11 |
| IDE | Renesas e² studio 2025-12 |
| FSP | RZN FSP 2.0.0 |
| EtherCAT主站 | TwinCAT v3.1.4024.65 |
| Python | 3.12.12 |
| 硬件平台 | CN032开发板 RSK/(RZN2L + KSZ8081 + W25Q128) |

## 2. 项目目录结构

```
RZN2L_Multi-protocol_FW_Upgrade/
├── .gitignore                         # Git忽略配置
├── .project                           # e2 studio项目文件
├── README.MD                          # 本文档
├── boot_config_tool.py                # Boot配置命令生成工具
├── boot_config_cmd.bin                # Boot配置命令文件
├── mytree.sh                          # 目录树生成脚本
│
├── common/                            # 公共模块 (SBL和APP1共享)
│   ├── app_config.h/c                 # APP配置管理
│   ├── bank_config.h/c                # Bank配置管理
│   ├── bank_detection.h/c             # Bank检测
│   ├── boot_config_cmd.h/c            # 启动配置命令
│   ├── bsp_r52_global_counter.h/c     # 全局计数器
│   ├── circular_queue.h/c             # 环形队列
│   ├── crc32_table.h/c                # CRC32表
│   ├── ecat_foe_data.h                # EtherCAT FoE固件头
│   ├── flash_config.h                 # Flash配置
│   ├── loader_table_manager.h/c       # Loader Table管理
│   ├── log.h                          # 日志输出
│   ├── progress.h/c                   # 升级进度显示
│   ├── sbl_boot_params.h/c            # SBL启动参数
│   └── sbl_params.h                   # SBL参数
│
├── RZN2L_xspi0_boot_sbl/              # SBL (Secondary Boot Loader) 项目
│   ├── .project                       # 项目文件
│   ├── .cproject                      # C项目配置
│   ├── configuration.xml              # FSP配置
│   ├── script/
│   │   └── fsp_xspi0_boot_loader.ld   # 链接脚本
│   └── src/
│       ├── APP1_BANK0_Flash_section.s # Bank0 Flash定义
│       ├── APP1_BANK1_Flash_section.s # Bank1 Flash定义
│       ├── app_jump.c                 # APP跳转模块
│       ├── hal_entry.c                # 入口函数
│       ├── loader_table.c             # Loader Table定义
│       └── loader_table.h
│
├── RZN2L_xspi0_app1/                  # APP1 (EtherCAT从站) 项目
│   ├── .project                       # 项目文件
│   ├── .cproject                      # C项目配置
│   ├── configuration.xml              # FSP配置
│   ├── attach_crc.py                  # CRC附加脚本
│   ├── script/
│   │   ├── fsp_xspi0_boot_app1_bank0.ld
│   │   └── fsp_xspi0_boot_app1_bank1.ld
│   ├── src/
│   │   ├── hal_entry.c                # APP入口函数
│   │   ├── syscall.c                  # 系统调用
│   │   └── ethercat/
│   │       ├── beckhoff/Src/           # Beckhoff SSC协议栈
│   │       │   ├── bootmode.c/h       # FoE升级功能
│   │       │   ├── ecatfoe.c/h
│   │       │   ├── foeappl.c/h
│   │       │   └── ...
│   │       └── renesas/               # Renesas示例
│   └── rzn/                           # FSP生成代码 [HIDDEN]
│
├── MULTI_APP_ARCHITECTURE.md          # 多APP架构说明
├── VERSION_GUIDE.md                   # 版本号使用说明
├── UPGRADE_IMPROVEMENT.md             # 升级改进方案
├── POST_BUILD_CONFIG.md               # Post-build配置说明
└── DOWNGRADE_GUIDE.md                  # 降级操作指南
```

## 3. Flash内存布局

```
══════════════════════════════════════════════════════════════════════════════
                           Flash 地址映射 (基地址: 0x60000000)
══════════════════════════════════════════════════════════════════════════════

┌────────────────────────────────┬──────────────┬───────────────────────────────┐
│ 区域                          │ 起始地址     │ 大小                          │
├────────────────────────────────┼──────────────┼───────────────────────────────┤
│ SBL区域                       │ 0x60000000   │ 1MB (到APP1_BANK0_START)     │
│  ├─ Loader Params             │ 0x60000000   │ 76B                          │
│  ├─ SBL Code + Data           │ 0x6000004C   │ 约1MB - 76B                  │
│  └─ SBL参数区域 (3x4KB)        │ 0x600FD000   │ 12KB                         │
│     ├─ Loader Table           │ 0x600FD000   │ 4KB                          │
│     ├─ 备份区                 │ 0x600FE000   │ 4KB                          │
│     └─ 主区                   │ 0x600FF000   │ 4KB                          │
├────────────────────────────────┼──────────────┼───────────────────────────────┤
│ APP1 Bank0                    │ 0x60100000   │ 1MB                          │
│  ├─ g_header (76B)            │ +0x0000      │ 76B                          │
│  ├─ g_identify (16B)         │ +0x004C      │ 16B                          │
│  ├─ 代码段                   │ +0x005C      │ 约1MB - 96B                  │
│  └─ CRC32 (4B)               │ -0x0004      │ 4B                           │
├────────────────────────────────┼──────────────┼───────────────────────────────┤
│ APP1 Bank1                    │ 0x60200000   │ 1MB                          │
│  ├─ g_header (76B)            │ +0x0000      │ 76B                          │
│  ├─ g_identify (16B)         │ +0x004C      │ 16B                          │
│  ├─ 代码段                   │ +0x005C      │ 约1MB - 96B                  │
│  └─ CRC32 (4B)               │ -0x0004      │ 4B                           │
├────────────────────────────────┼──────────────┼───────────────────────────────┤
│ 保留区域                      │ 0x60300000   │ 约61MB (扩展用)              │
└────────────────────────────────┴──────────────┴───────────────────────────────┘

宏定义参考 (flash_config.h):
  APP1_BANK0_START     = 0x60100000  SIZE = 1MB
  APP1_BANK1_START     = 0x60200000  SIZE = 1MB
  SBL_LOADER_TABLE_ADDR = 0x600FD000  SIZE = 4KB
  SBL_BACKUP_PARAMS_ADDR = 0x600FE000 SIZE = 4KB
  SBL_MAIN_PARAMS_ADDR   = 0x600FF000 SIZE = 4KB
```

### 3.1 g_header 结构 (76字节, 编译生成)

| 偏移 | 大小 | 字段 | 说明 |
|------|------|------|------|
| 0    | 3B   | "APP" | 魔数 |
| 3    | 1B   | Patch | 补丁版本号 (0-255) |
| 4    | 1B   | Minor | 次版本号 (0-255) |
| 5    | 1B   | Major | 主版本号 (0-255) |
| 6    | 1B   | 保留  | 0x00 |
| 7-75 | 69B  | 保留  | 保留字段 (由attach_crc.py写入目标Bank+长度，单固件方案: 自动检测Bank) |

### 3.2 g_identify 结构 (16字节, bootmode.c 定义)

| 偏移 | 大小 | 字段 | 说明 |
|------|------|------|------|
| 0    | 4B   | Vendor ID | 厂商ID |
| 4    | 4B   | Product Code | 产品代码 |
| 8    | 4B   | Revision Number | 修订号 |
| 12   | 4B   | Serial Number | 序列号 |

### 3.3 版本号存储格式

```
版本号: v1.2.3 -> 0x00010203
  └─ 小端序存储: [03][02][01][00]
     即: 偏移3=0x03(Patch), 偏移4=0x02(Minor), 偏移5=0x01(Major)
```

```c
BSP_DONT_REMOVE const uint32_t g_identify[4] = {
    VENDOR_ID,         // offset  0: Vendor ID
    PRODUCT_CODE,      // offset  4: Product Code
    REVISION_NUMBER,   // offset  8: Revision Number
    SERIAL_NUMBER      // offset 12: Serial Number
};
```

### 3.4 SBL Boot Params 双备份机制

| 区域 | 地址 | 大小 | 说明 |
|------|------|------|------|
| 主区 | 0x600FF000 | 4KB | 主要存储区域 |
| 备份区 | 0x600FE000 | 4KB | 备用区域，损坏时恢复 |

**读写策略:**
- **读取**: 先读主区，无效则读备份区
- **写入**: 写入主区成功后，同步写入备份区

### 3.2 SBL Boot Params 结构 (33字节)

```c
typedef struct {
    uint8_t header_app[3];          // "APP"
    uint32_t header_version;         // 版本号
    uint8_t target_app;             // 目标APP (1-5)
    uint8_t current_bank;           // 当前Bank (0/1)
    uint8_t target_bank;            // 目标Bank (0/1/0xFF自动)
    uint8_t version_check_enable;   // 版本号检查 (1/0)
    uint32_t vendor_id;             // Vendor ID
    uint32_t product_code;          // Product Code
    uint32_t revision_number;       // Revision Number
    uint32_t serial_number;         // Serial Number
    uint32_t crc32;                 // CRC校验
} sbl_boot_params_t;
```

## 4. SBL启动到APP1 FoE完整流程图

```mermaid
flowchart TB
    subgraph BootROM["【阶段1】Boot ROM (芯片内置)"]
        BR1["上电复位"]
        BR2["读取Loader Params<br/>(0x60000000)"]
        BR3["从Flash加载SBL<br/>到BTCM (0x20000000)"]
        BR4["跳转到SBL入口"]
    end

    subgraph SBL["【阶段2】SBL启动流程"]
        S1["hal_entry()"]
        S2["初始化QSPI<br/>R_XSPI_QSPI_Open()"]
        S3["初始化CRC<br/>CRC_Init()"]
        
        subgraph InitModules["模块初始化"]
            SM1["SblBootParams_Init()<br/>读取启动参数"]
            SM2["AppConfig_Init()<br/>APP配置管理"]
            SM3["BankConfig_Init()<br/>Bank配置管理"]
            SM4["LoaderTableManager_Init()<br/>Loader Table管理"]
            SM5["AppJump_Init()<br/>跳转模块初始化"]
        end
        
        S4["sblCheckBootParams()<br/>检查Boot Params"]
        S5["AppJump_ToNextApp()<br/>获取目标APP"]
        S6["AppConfig_IsEnabled()<br/>检查APP是否启用"]
        S7{"APP1是否启用?"}
        
        S8["BankConfig_SelectNextBank()<br/>自动选择有效Bank"]
        S9["BankConfig_IsBankValid()<br/>验证Bank有效性"]
        
        S10["LoaderTableManager_SelectEntry()<br/>选择Loader Table条目"]
        S11["AppJump_ValidateImage()<br/>验证APP镜像"]
        S12["从Flash复制到SRAM<br/>sbl_copy_multibyte()"]
        S13["关闭UART + 禁用中断"]
        S14["跳转到APP1入口<br/>app_prg()"]
    end

    subgraph APP1_INIT["【阶段3】APP1初始化"]
        A1["hal_entry()"]
        A2["__enable_irq() 启用中断"]
        A3["R_SCI_UART_Open() 初始化UART"]
        A4["R_XSPI_QSPI_Open() 初始化QSPI"]
        A5["RM_ETHERCAT_SSC_PORT_Open()<br/>初始化EtherCAT SSC"]
        A6["R_ETHER_PHY_StartAutoNegotiate()<br/> PHY自动协商"]
        A7["MainInit() 初始化EtherCAT栈"]
        A8["APPL_GenerateMapping()<br/>生成PDO映射"]
        A9["bRunApplication = TRUE<br/>设置运行标志"]
    end

    subgraph APP1_RUN["【阶段4】APP1主循环"]
        A10["MainLoop() 主循环"]
        A11{"是否有EtherCAT<br/>帧到达?"}
        A12["处理EtherCAT帧<br/>PDI/PDO更新"]
        A13{"是否是FoE请求?"}
    end

    subgraph FOE_UPGRADE["【阶段5】FoE升级流程"]
        F1["BL_Start()"]
        F1a["CRC_Init() 初始化CRC"]
        F1b["BankDetection_Init() 初始化Bank检测"]
        F1c["SblBootParams_Init() 初始化Boot Params"]
        F1d["读取版本号检查配置<br/>version_check_enabled"]
        
        F2["BL_StartDownload()"]
        F2a["获取当前Bank<br/>BankDetection_GetCurrentBank()"]
        F2b["等待固件头"]
        
        F3["BL_Data() - 首次接收"]
        F3a["验证固件魔数 'APP'"]
        F3b{"版本号检查<br/>新版本 > 当前版本?"}
        F3c{"确定目标Bank<br/>自动模式/强制模式"]
        F3d["擦除目标Bank<br/>R_XSPI_QSPI_Erase()"]
        
        F4["BL_Data() - 数据接收"]
        F4a["写入环形队列<br/>Queue_Wirte()"]
        F4b{"队列满256字节?"}
        F4c["写入Flash<br/>norFlashPageProgram()"]
        F4d["更新进度条<br/>Progress_Update()"]
        
        F5{"升级完成<br/>write_offset >= header_len?"}
        F6["CRC校验<br/>CRC_Calculate()"]
        F7["更新SII<br/>ESC_EepromAccess()"]
        F8["更新SBL Boot Params<br/>SblBootParams_Write()"]
        F9["BL_SetRebootFlag(TRUE)"]
        F10["BL_Reboot() 系统复位"]
    end

    BR1 --> BR2 --> BR3 --> BR4 --> S1
    S1 --> S2 --> S3
    S2 --> S3
    S3 --> InitModules
    SM1 --> SM2 --> SM3 --> SM4 --> SM5
    InitModules --> S4 --> S5 --> S6 --> S7
    S7 -->|"是"| S8
    S7 -->|"否"| S9
    S8 --> S9 --> S10 --> S11 --> S12 --> S13 --> S14
    
    S14 --> A1
    A1 --> A2 --> A3 --> A4 --> A5 --> A6 --> A7 --> A8 --> A9
    A9 --> A10
    A10 --> A11
    A11 -->|"否"| A10
    A11 -->|"是"| A12
    A12 --> A13
    A13 -->|"否"| A10
    A13 -->|"是"| F1
    
    F1 --> F1a --> F1b --> F1c --> F1d
    F1d --> F2 --> F2a --> F2b
    F2b --> F3
    F3 --> F3a --> F3b --> F3c --> F3d
    F3d --> F4
    F4 --> F4a --> F4b
    F4b -->|"否"| F4
    F4b -->|"是"| F4c --> F4d --> F5
    F5 -->|"否"| F4
    F5 -->|"是"| F6 --> F7 --> F8 --> F9 --> F10
    
    style BootROM fill:#E6FFE6,stroke:#00CC00
    style SBL fill:#FFF0E6,stroke:#FF9900
    style APP1_INIT fill:#E6F3FF,stroke:#0066CC
    style APP1_RUN fill:#E6F3FF,stroke:#0066CC
    style FOE_UPGRADE fill:#FFE6E6,stroke:#CC0000
```

### 4.1 功能模块说明

| 阶段 | 模块 | 功能 |
|------|------|------|
| **Boot ROM** | Loader Params | 指向SBL加载地址 |
| **SBL** | SblBootParams | 管理启动参数 (0x600FF000) |
| **SBL** | AppConfig | 管理APP1-5配置 |
| **SBL** | BankConfig | 管理Bank切换逻辑 |
| **SBL** | LoaderTableManager | 管理Loader Table |
| **SBL** | AppJump | 执行APP跳转 |
| **APP1** | EtherCAT SSC | EtherCAT从站协议栈 |
| **APP1** | BankDetection | Bank检测 |
| **APP1** | Circular Queue | 环形缓冲区 |
| **APP1** | CRC32 | 固件校验 |
| **APP1** | Progress | 升级进度显示 |

## 5. SBL模块详解

### 5.1 SBL主要模块

| 模块 | 功能 |
|------|------|
| **AppConfig** | 管理多个APP的配置信息，支持APP1-5 |
| **BankConfig** | 管理Bank切换和选择逻辑 |
| **LoaderTableManager** | 管理Loader Table配置 |
| **AppJump** | 执行APP跳转的核心模块 |
| **SblBootParams** | 存储启动参数（地址0x600FF000） |

### 5.2 SBL启动流程

```
1. hal_entry()
   ├─ 初始化QSPI
   ├─ 初始化CRC
   └─ 初始化所有模块
       ├─ SblBootParams_Init()
       ├─ AppConfig_Init()
       ├─ BankConfig_Init()
       ├─ LoaderTableManager_Init()
       └─ AppJump_Init()

2. AppJump_ToNextApp()
   ├─ 读取SBL Boot Params
   ├─ 获取目标APP
   ├─ AppConfig_IsEnabled() - 检查APP是否启用
   ├─ BankConfig_SelectNextBank() - 自动选择有效Bank
   └─ AppJump_ToApp()

3. AppJump_ToApp()
   ├─ 验证APP ID和Bank ID
   ├─ 检查APP是否启用
   ├─ 检查Bank是否有效
   ├─ LoaderTableManager_SelectEntry()
   ├─ AppJump_ValidateImage() - 验证镜像
   ├─ 从Flash复制到SRAM
   └─ 跳转到APP入口点
```

## 6. APP1 (EtherCAT从站) 详解

### 6.1 版本号定义

位置: `RZN2L_xspi0_app1/src/ethercat/beckhoff/Src/bootmode.c`

```c
// APP1版本号定义 (语义化版本规范)
#ifndef APP1_VERSION_MAJOR
#define APP1_VERSION_MAJOR  1
#endif

#ifndef APP1_VERSION_MINOR
#define APP1_VERSION_MINOR  0
#endif

#ifndef APP1_VERSION_PATCH
#define APP1_VERSION_PATCH  0
#endif

// 自动组合: v1.2.3 -> 0x00010203
#define APP1_VERSION ((APP1_VERSION_MAJOR << 16) | (APP1_VERSION_MINOR << 8) | APP1_VERSION_PATCH)
```

### 6.2 固件头结构

```c
typedef struct {
    uint8_t header_app[3];          // "APP" 魔数
    uint32_t header_version;        // 版本号
    uint8_t header_target_bank;    // 目标Bank: 0=Bank0, 1=Bank1, 0xFF=自动
    uint32_t header_len;            // 固件长度
    uint8_t header_reserved[64];   // 保留
    uint32_t vendor_id;            // Vendor ID
    uint32_t product_code;         // Product Code
    uint32_t revision_number;       // Revision Number
    uint32_t serial_number;         // Serial Number
} app_header_t;
```

### 6.3 FoE升级流程

```
1. BL_Start(State)
   ├─ 初始化CRC
   ├─ 初始化Bank检测
   ├─ 初始化Boot Params
   └─ 读取版本号检查配置

2. BL_StartDownload(password)
   ├─ 获取当前Bank
   └─ 等待固件头

3. BL_Data() - 首次接收
   ├─ 验证固件魔数 "APP"
   ├─ 版本号检查 (如果启用)
   │   └─ 新版本 > 当前版本 ?
   ├─ 确定目标Bank
   │   ├─ 自动模式: 写入另一个Bank
   │   └─ 强制模式: 检查有效性
   └─ 擦除目标Bank

4. BL_Data() - 数据接收
   ├─ 写入环形队列
   ├─ 累积256字节写入Flash
   └─ 显示进度

5. 升级完成
   ├─ CRC校验
   ├─ 更新SII (Revision Number)
   ├─ 更新SBL Boot Params
   └─ 设置重启标志 -> BL_Reboot()
```

## 7. 升级功能模块

### 7.1 Bank自动检测

```c
// 优先级:
// 1. 从SBL Boot Params读取
// 2. 通过链接符号判断
// 3. 默认Bank0
uint8_t BankDetection_GetCurrentBank(void);
uint8_t BankDetection_GetTargetBank(uint8_t current_bank);
```

### 7.2 版本号检查

- 默认启用版本号检查
- 新版本号必须大于当前版本号
- 可通过SBL Boot Params的`version_check_enable`字段配置

### 7.3 升级安全机制

| 机制 | 说明 |
|------|------|
| **Bank检测** | 防止写入当前运行的Bank |
| **版本号检查** | 防止降级攻击 |
| **CRC校验** | 固件完整性校验 |
| **Boot Params保护** | 启动参数CRC校验 |

## 8. 单固件升级方案

### 8.1 原理

只需生成**一个固件文件**，运行时自动检测当前Bank，写入另一个Bank。

### 8.2 固件头关键字段

| 字段 | 说明 |
|------|------|
| `header_app` | "APP" 魔数 |
| `header_version` | 版本号 (0x00MMmmpp) |
| `header_target_bank` | 0xFF=自动模式 |

### 8.3 Python脚本用法

```bash
# 自动读取C代码中的版本号
python attach_crc.py

# 指定版本号
python attach_crc.py --version 1.2.3

# 强制写入Bank0
python attach_crc.py --version 1.2.3 --target-bank 0

# 强制写入Bank1
python attach_crc.py --version 1.2.3 --target-bank 1
```

## 9. 降级操作

### 9.1 方法1: 修改C代码

```c
// common/flash_config.h
#define BOOT_PARAMS_VERSION_CHECK_ENABLE    0  // 禁用
```

重新编译SBL并烧录。

### 9.2 方法2: 通过FoE发送配置命令

```bash
# 禁用版本号检查
python boot_config_tool.py --disable-version-check

# 启用版本号检查
python boot_config_tool.py --enable-version-check
```

## 10. 多APP支持 (扩展)

当前支持单个APP (APP1)，通过修改可扩展支持APP1-5：

```c
// 多APP Loader Table配置示例
const loader_table table[TABLE_ENTRY_NUM] = {
    // APP1 Bank0
    { app1_bank0_flash_addr, app1_bank0_ram_addr, app1_bank0_size, TABLE_ENABLE },
    // APP1 Bank1
    { app1_bank1_flash_addr, app1_bank1_ram_addr, app1_bank1_size, TABLE_DISABLE },
    // APP2 Bank0
    { app2_bank0_flash_addr, app2_bank0_ram_addr, app2_bank0_size, TABLE_DISABLE },
    // APP2 Bank1
    { app2_bank1_flash_addr, app2_bank1_ram_addr, app2_bank1_size, TABLE_DISABLE },
};
```

## 11. 常见问题

### Q1: 找不到Python

解决: 使用完整路径 `C:\Python312\python.exe`

### Q2: 降级后无法启动

解决: 检查Boot Params，必要时重新烧录SBL

### Q3: 如何查看当前版本号

```bash
python -c "
import struct
with open('firmware.bin', 'rb') as f:
    data = f.read(8)
    print(f'Version: {data[5]}.{data[4]}.{data[3]}')
"
```

## 12. 作者

Jerry.Chen

## 13. 日期

2026-04-18
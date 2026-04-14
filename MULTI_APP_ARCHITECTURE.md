# 多APP支持架构说明

## 概述

SBL (Secondary Boot Loader) 支持加载多个APP，每个APP都有独立的版本号管理。

---

## 命名规范

### 版本号宏命名

```
APP1_VERSION_MAJOR  APP1主版本号
APP1_VERSION_MINOR  APP1次版本号
APP1_VERSION_PATCH  APP1补丁版本号
APP1_VERSION        APP1组合版本号

APP2_VERSION_MAJOR  APP2主版本号
APP2_VERSION_MINOR  APP2次版本号
APP2_VERSION_PATCH  APP2补丁版本号
APP2_VERSION        APP2组合版本号

...
```

### Flash区域命名

```
APP1_BANK0_ADDR     APP1 Bank0起始地址
APP1_BANK1_ADDR     APP1 Bank1起始地址

APP2_BANK0_ADDR     APP2 Bank0起始地址
APP2_BANK1_ADDR     APP2 Bank1起始地址

...
```

---

## Flash布局（多APP示例）

```
起始地址: 0x60000000
├─ SBL区域 (0x60000000 - 0x60100000)
│  ├─ Loader Params
│  ├─ SBL Code
│  ├─ Loader Table
│  └─ Boot Params
│
├─ APP1区域 (0x60100000 - 0x60300000, 2MB)
│  ├─ APP1 Bank0 (0x60100000 - 0x60200000, 1MB)
│  │  ├─ Header: "APP" + APP1_VERSION
│  │  ├─ Identify
│  │  ├─ Code
│  │  └─ CRC32
│  └─ APP1 Bank1 (0x60200000 - 0x60300000, 1MB)
│
├─ APP2区域 (0x60300000 - 0x60500000, 2MB)
│  ├─ APP2 Bank0 (0x60300000 - 0x60400000, 1MB)
│  └─ APP2 Bank1 (0x60400000 - 0x60500000, 1MB)
│
└─ APP3区域 (0x60500000 - 0x60700000, 2MB)
   ├─ APP3 Bank0
   └─ APP3 Bank1
```

---

## Loader Table配置

### 单APP配置（当前）

```c
// loader_table.c
const loader_table table[TABLE_ENTRY_NUM] = {
    // APP1 Bank0
    { (uint32_t *)app1_bank0_prg_flash_addr, 
      (uint32_t *)app1_bank0_prg_start_addr, 
      (uint32_t)app1_bank0_prg_size, 
      TABLE_ENABLE },
    
    // APP1 Bank1 (禁用)
    { ..., TABLE_DISABLE },
    { ..., TABLE_DISABLE },
    { ..., TABLE_DISABLE }
};
```

---

### 多APP配置（扩展）

```c
// loader_table.c
const loader_table table[TABLE_ENTRY_NUM] = {
    // APP1 Bank0
    { (uint32_t *)app1_bank0_prg_flash_addr, 
      (uint32_t *)app1_bank0_prg_start_addr, 
      (uint32_t)app1_bank0_prg_size, 
      TABLE_ENABLE },
    
    // APP1 Bank1
    { (uint32_t *)app1_bank1_prg_flash_addr, 
      (uint32_t *)app1_bank1_prg_start_addr, 
      (uint32_t)app1_bank1_prg_size, 
      TABLE_DISABLE },
    
    // APP2 Bank0
    { (uint32_t *)app2_bank0_prg_flash_addr, 
      (uint32_t *)app2_bank0_prg_start_addr, 
      (uint32_t)app2_bank0_prg_size, 
      TABLE_DISABLE },
    
    // APP2 Bank1
    { (uint32_t *)app2_bank1_prg_flash_addr, 
      (uint32_t *)app2_bank1_prg_start_addr, 
      (uint32_t)app2_bank1_prg_size, 
      TABLE_DISABLE }
};
```

---

## Boot Params扩展

### 当前结构（单APP）

```c
typedef struct {
    uint8_t header_app[3];          // "APP"
    uint32_t header_version;        // APP1版本号
    
    uint8_t current_bank;           // 当前Bank
    uint8_t target_bank;            // 目标Bank
    
    uint8_t version_check_enable;   // 版本号检查使能
    
    uint32_t vendor_id;
    uint32_t product_code;
    uint32_t revision_number;
    uint32_t serial_number;
    
    uint32_t crc32;
} boot_params_t;
```

---

### 扩展结构（多APP）

```c
typedef struct {
    // APP1配置
    uint8_t app1_header[3];         // "APP"
    uint32_t app1_version;          // APP1版本号
    uint8_t app1_current_bank;      // APP1当前Bank
    uint8_t app1_target_bank;       // APP1目标Bank
    
    // APP2配置
    uint8_t app2_header[3];         // "APP"
    uint32_t app2_version;          // APP2版本号
    uint8_t app2_current_bank;      // APP2当前Bank
    uint8_t app2_target_bank;       // APP2目标Bank
    
    // APP3配置
    uint8_t app3_header[3];         // "APP"
    uint32_t app3_version;          // APP3版本号
    uint8_t app3_current_bank;      // APP3当前Bank
    uint8_t app3_target_bank;       // APP3目标Bank
    
    // 全局配置
    uint8_t current_app;            // 当前运行的APP (1, 2, 3)
    uint8_t version_check_enable;   // 版本号检查使能
    
    // Identify信息
    uint32_t vendor_id;
    uint32_t product_code;
    uint32_t revision_number;
    uint32_t serial_number;
    
    uint32_t crc32;
} boot_params_t;
```

---

## 多APP使用场景

### 场景1：功能分离

```
APP1: EtherCAT从站功能
APP2: CANopen从站功能
APP3: Modbus从站功能
```

**优势：**
- 功能独立，互不干扰
- 可以单独升级某个APP
- 降低单个APP复杂度

---

### 场景2：安全隔离

```
APP1: 安全关键功能 (Safety)
APP2: 非安全功能 (Non-Safety)
```

**优势：**
- 安全功能独立验证
- 非安全功能不影响安全
- 符合功能安全标准

---

### 场景3：协议切换

```
APP1: EtherCAT协议
APP2: PROFINET协议
APP3: EtherNet/IP协议
```

**优势：**
- 支持多种工业协议
- 现场可切换协议
- 一机多用

---

## 版本号管理

### APP1版本号定义

```c
// RZN2L_xspi0_app1/src/ethercat/beckhoff/Src/bootmode.c

#ifndef APP1_VERSION_MAJOR
#define APP1_VERSION_MAJOR  1
#endif

#ifndef APP1_VERSION_MINOR
#define APP1_VERSION_MINOR  0
#endif

#ifndef APP1_VERSION_PATCH
#define APP1_VERSION_PATCH  0
#endif

#define APP1_VERSION ((APP1_VERSION_MAJOR << 16) | (APP1_VERSION_MINOR << 8) | APP1_VERSION_PATCH)
```

---

### APP2版本号定义

```c
// RZN2L_xspi0_app2/src/canopen/.../bootmode.c

#ifndef APP2_VERSION_MAJOR
#define APP2_VERSION_MAJOR  1
#endif

#ifndef APP2_VERSION_MINOR
#define APP2_VERSION_MINOR  0
#endif

#ifndef APP2_VERSION_PATCH
#define APP2_VERSION_PATCH  0
#endif

#define APP2_VERSION ((APP2_VERSION_MAJOR << 16) | (APP2_VERSION_MINOR << 8) | APP2_VERSION_PATCH)
```

---

## Python脚本适配

### APP1脚本

```python
# RZN2L_xspi0_app1/attach_crc.py

# 读取APP1版本号
major_match = re.match(r'#define\s+APP1_VERSION_MAJOR\s+(\d+)', stripped)
minor_match = re.match(r'#define\s+APP1_VERSION_MINOR\s+(\d+)', stripped)
patch_match = re.match(r'#define\s+APP1_VERSION_PATCH\s+(\d+)', stripped)
```

---

### APP2脚本

```python
# RZN2L_xspi0_app2/attach_crc.py

# 读取APP2版本号
major_match = re.match(r'#define\s+APP2_VERSION_MAJOR\s+(\d+)', stripped)
minor_match = re.match(r'#define\s+APP2_VERSION_MINOR\s+(\d+)', stripped)
patch_match = re.match(r'#define\s+APP2_VERSION_PATCH\s+(\d+)', stripped)
```

---

## 升级流程

### 单APP升级（当前）

```
1. 通过EtherCAT FoE发送APP1固件
2. APP1验证版本号
3. APP1写入另一个Bank
4. 重启运行新APP1
```

---

### 多APP升级（扩展）

```
1. 选择目标APP (APP1/APP2/APP3)
2. 通过对应协议发送固件
   - APP1: EtherCAT FoE
   - APP2: CANopen SDO
   - APP3: Modbus Write
3. 目标APP验证版本号
4. 目标APP写入另一个Bank
5. 重启运行新APP
```

---

## 配置示例

### flash_config.h（多APP）

```c
// APP1配置
#define APP1_ENABLE             1
#define APP1_BANK0_ADDR         (0x60100000)
#define APP1_BANK1_ADDR         (0x60200000)
#define APP1_TOTAL_SIZE         (2 * 1024 * 1024)  // 2MB

// APP2配置
#define APP2_ENABLE             1
#define APP2_BANK0_ADDR         (0x60300000)
#define APP2_BANK1_ADDR         (0x60400000)
#define APP2_TOTAL_SIZE         (2 * 1024 * 1024)  // 2MB

// APP3配置
#define APP3_ENABLE             0  // 禁用
#define APP3_BANK0_ADDR         (0x60500000)
#define APP3_BANK1_ADDR         (0x60600000)
#define APP3_TOTAL_SIZE         (2 * 1024 * 1024)  // 2MB
```

---

## 注意事项

### 1. Flash空间规划

- 每个APP需要足够空间（建议≥1MB）
- 预留升级空间
- 考虑未来扩展

### 2. RAM资源分配

- 每个APP独立RAM区域
- 避免RAM冲突
- 共享资源需互斥

### 3. 外设资源分配

- 每个APP独立外设
- 避免外设冲突
- 共享外设需仲裁

### 4. 版本号管理

- 每个APP独立版本号
- 版本号检查独立配置
- 记录每个APP的版本历史

---

## 作者

Jerry.Chen

## 日期

2026-04-13

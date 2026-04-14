# RZN2L固件升级系统改进方案

## 改进概述

本次改进实现了**单固件自适应Bank升级**方案，主要特性：

1. **单固件生成** - 只需生成一个固件文件
2. **版本号检查** - 升级固件版本号必须大于当前版本（可配置）
3. **Bank自动检测** - 运行时自动检测当前Bank，写入另一个Bank
4. **双保险机制** - Bank检测 + 版本号检查双重保护

---

## 主要修改

### 1. 固件头结构改进

**修改文件:** `common/ecat_foe_data.h`

**旧结构:**
```c
typedef struct {
    uint8_t header_app[9];      // "APP1BANK0" 或 "APP1BANK1"
    uint32_t header_len;        // 固件长度
    uint8_t header_reserved[...];
    union { ... };              // Identify信息
} app_header_t;
```

**新结构:**
```c
typedef struct {
    uint8_t header_app[3];      // "APP" (固定魔数)
    uint32_t header_version;    // 版本号 (例如: 0x00010203 = v1.2.3)
    uint8_t header_target_bank; // 目标Bank: 0=Bank0, 1=Bank1, 0xFF=自动
    uint32_t header_len;        // 固件总长度
    uint8_t header_reserved[...];
    union { ... };              // Identify信息
} app_header_t;
```

**改进点:**
- ✅ 固件魔数统一为"APP"
- ✅ 增加4字节版本号字段
- ✅ 增加目标Bank字段（支持自动模式）
- ✅ 单固件适配所有Bank

---

### 2. Boot Params结构改进

**新增文件:** `common/boot_params.h`, `common/boot_params.c`

**结构定义:**
```c
typedef struct {
    // 固件标识 (7字节)
    uint8_t header_app[3];          // "APP"
    uint32_t header_version;        // 当前运行固件版本号

    // Bank信息 (2字节)
    uint8_t current_bank;           // 当前运行的Bank
    uint8_t target_bank;            // 下次启动的Bank

    // 配置选项 (1字节)
    uint8_t version_check_enable;   // 版本号检查使能: 1=启用, 0=禁用

    // Identify信息 (16字节)
    uint32_t vendor_id;
    uint32_t product_code;
    uint32_t revision_number;
    uint32_t serial_number;

    // CRC校验 (4字节)
    uint32_t crc32;
} boot_params_t;
```

**存储位置:** Flash地址 `0x600FF000` (4KB)

**功能:**
- 记录当前运行的Bank和版本号
- 配置版本号检查选项（默认启用）
- CRC32校验保护数据完整性

---

### 3. Bank自动检测机制

**新增文件:** `common/bank_detection.h`, `common/bank_detection.c`

**API接口:**
```c
// 初始化Bank检测
void BankDetection_Init(void);

// 获取当前运行的Bank
uint8_t BankDetection_GetCurrentBank(void);

// 获取目标写入Bank (另一个Bank)
uint8_t BankDetection_GetTargetBank(uint8_t current_bank);

// 检查固件头的目标Bank是否有效
bool BankDetection_IsTargetBankValid(uint8_t target_bank, uint8_t current_bank);

// 获取Bank的起始地址
uint32_t BankDetection_GetBankAddress(uint8_t bank);
```

**检测方法:**
1. **优先方法:** 从Boot Params读取
2. **备用方法:** 通过链接符号判断
3. **最终默认:** Bank0

---

### 4. 升级逻辑改进

**修改文件:** `RZN2L_xspi0_app1/src/ethercat/beckhoff/Src/bootmode.c`

**升级流程:**

```
1. BL_Start()
   ├─ 初始化CRC
   ├─ 初始化Bank检测
   ├─ 初始化Boot Params
   └─ 读取版本号检查配置

2. BL_StartDownload()
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
   ├─ 累积到256字节写入Flash
   └─ 显示进度

5. BL_Data() - 升级完成
   ├─ CRC校验
   ├─ 更新SII (Revision Number)
   ├─ 更新Boot Params
   │   ├─ 记录新版本号
   │   ├─ 记录目标Bank
   │   └─ CRC校验
   └─ 设置重启标志
```

**关键改进:**
- ✅ 首次接收时验证固件头
- ✅ 版本号检查（可配置）
- ✅ 动态确定目标Bank
- ✅ 升级完成后更新Boot Params

---

### 5. Python脚本改进

**新增文件:** `RZN2L_xspi0_app1/attach_crc.py` (合并版本)

**使用方法:**
```bash
# 默认版本1.0.0, 自动模式
python attach_crc.py

# 指定版本号
python attach_crc.py --version 1.2.3

# 强制写入Bank0
python attach_crc.py --version 1.2.3 --target-bank 0

# 强制写入Bank1
python attach_crc.py --version 1.2.3 --target-bank 1
```

**功能:**
- 解析版本号字符串 (major.minor.patch)
- 修改固件头 ("APP" + 版本号 + 目标Bank)
- 计算并附加CRC32
- 输出单个固件文件

---

## 使用示例

### 1. 编译固件

```bash
# 在e2 studio中编译APP1项目
# 输出: BUILD/RZN2L_xspi0_app1.bin
```

### 2. 生成带CRC的固件

```bash
# 生成v1.2.3固件，自动模式
python attach_crc.py --version 1.2.3

# 输出: BUILD/RZN2L_xspi0_app1_with_crc.bin
```

### 3. 通过EtherCAT FoE升级

1. 将固件发送到设备
2. 设备自动检测当前Bank
3. 验证版本号 (1.2.3 > 当前版本)
4. 写入另一个Bank
5. 重启后运行新固件

---

## 配置选项

### 版本号检查

**默认:** 启用

**配置位置:** `common/flash_config.h`
```c
#define BOOT_PARAMS_VERSION_CHECK_ENABLE    1   // 1=启用, 0=禁用
```

**运行时修改:** 通过Boot Params的`version_check_enable`字段

**检查逻辑:**
- 启用时: 新版本号 > 当前版本号
- 禁用时: 允许降级或相同版本

---

## 安全机制

### 1. 双重保护

- **Bank检测:** 防止写入当前运行的Bank
- **版本号检查:** 防止降级攻击

### 2. CRC校验

- 固件完整性校验
- Boot Params完整性校验
- 升级前后双重校验

### 3. 自动回滚

- Boot Params损坏时自动恢复
- Bank损坏时选择另一个有效Bank
- 两个Bank都损坏时停止启动

---

## 文件清单

### 新增文件

```
common/
├── boot_params.h          # Boot Params结构定义
├── boot_params.c          # Boot Params实现
├── bank_detection.h       # Bank检测接口
└── bank_detection.c       # Bank检测实现

RZN2L_xspi0_app1/
└── attach_crc.py          # 合并的CRC附加脚本
```

### 修改文件

```
common/
├── ecat_foe_data.h        # 固件头结构修改
├── flash_config.h         # Bank定义修改
└── sbl_params.c           # Boot Params支持修改

RZN2L_xspi0_app1/src/ethercat/beckhoff/Src/
└── bootmode.c             # 升级逻辑修改
```

---

## 测试建议

### 1. Bank切换测试

```
Bank0 (v1.0.0) --升级--> Bank1 (v1.0.1)
Bank1 (v1.0.1) --升级--> Bank0 (v1.0.2)
Bank0 (v1.0.2) --升级--> Bank1 (v1.0.3)
```

### 2. 版本号检查测试

```
✅ v1.0.0 -> v1.0.1 (允许)
✅ v1.0.1 -> v1.1.0 (允许)
✅ v1.1.0 -> v2.0.0 (允许)
❌ v1.0.1 -> v1.0.0 (拒绝，版本号检查启用)
✅ v1.0.1 -> v1.0.0 (允许，版本号检查禁用)
```

### 3. 异常情况测试

```
- Boot Params损坏
- Bank0损坏
- Bank1损坏
- 两个Bank都损坏
- 固件CRC错误
- 固件魔数错误
```

---

## 优势总结

### 改进前

- ❌ 需要生成2个固件
- ❌ 用户需要知道当前Bank
- ❌ 手动选择对应固件
- ❌ 编译时固定Bank
- ❌ 无版本号检查

### 改进后

- ✅ 只生成1个固件
- ✅ 自动检测当前Bank
- ✅ 自动写入另一个Bank
- ✅ 支持强制指定目标Bank
- ✅ 运行时动态判断
- ✅ 版本号检查（可配置）
- ✅ 更安全（双保险机制）

---

## 作者

Jerry.Chen

## 日期

2026-04-13

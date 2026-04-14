# 固件降级操作指南

## 概述

默认情况下，版本号检查是**启用**的，不允许降级。但可以通过以下方法临时禁用版本号检查，实现降级。

---

## 方法1：修改C代码重新编译SBL（最简单）

### 步骤：

1. **修改配置**
   ```c
   // 文件: common/flash_config.h
   #define BOOT_PARAMS_VERSION_CHECK_ENABLE    0  // 禁用
   ```

2. **重新编译SBL**
   - 编译 `RZN2L_xspi0_boot_sbl` 项目
   - 烧录新的SBL

3. **降级固件**
   - 现在可以发送任何版本的固件
   - 版本号检查已禁用

4. **恢复版本号检查**（可选）
   ```c
   #define BOOT_PARAMS_VERSION_CHECK_ENABLE    1  // 启用
   ```
   - 重新编译并烧录SBL

---

## 方法2：通过Boot配置命令（推荐）

### 步骤：

#### 1. 生成禁用版本号检查的命令

```bash
cd RZN2L_Multi-protocol_FW_Upgrade
python boot_config_tool.py --disable-version-check --output disable_version_check.bin
```

**输出：**
```
============================================================
Generating: Disable Version Check Command
============================================================
This command will:
  - Disable version number check
  - Allow firmware downgrade
  - Use with caution!
============================================================

[SUCCESS] Command generated: disable_version_check.bin
[SUCCESS] Size: 20 bytes
```

#### 2. 通过EtherCAT FoE发送命令

- 将 `disable_version_check.bin` 通过FoE发送到设备
- 设备会处理命令并禁用版本号检查

#### 3. 降级固件

- 现在可以发送旧版本固件
- 版本号检查已禁用，允许降级

#### 4. 恢复版本号检查（重要！）

```bash
python boot_config_tool.py --enable-version-check --output enable_version_check.bin
```

- 发送 `enable_version_check.bin` 到设备
- 版本号检查恢复启用

---

## 方法3：通过EtherCAT CoE（对象字典）

### 添加配置对象

可以在EtherCAT对象字典中添加配置对象：

```c
// 对象0x8000: 版本号检查使能
// 类型: UINT8
// 访问: RW
// 值: 0=禁用, 1=启用

case 0x8000:
    if (command == ESC_WR)
    {
        uint8_t enable = *(uint8_t *)data;
        if (enable == 0)
            BootConfig_DisableVersionCheck();
        else
            BootConfig_EnableVersionCheck();
    }
    else
    {
        boot_params_t params;
        BootParams_Read(&params);
        *(uint8_t *)data = params.version_check_enable;
    }
    break;
```

**使用方法：**
- 通过TwinCAT写入对象0x8000
- 值为0：禁用版本号检查
- 值为1：启用版本号检查

---

## 降级操作流程

### 完整流程示例：

```
当前版本: v2.1.0
目标版本: v1.0.0 (降级)

步骤1: 禁用版本号检查
  ├─ 方法1: 修改C代码重新编译SBL
  ├─ 方法2: 发送配置命令
  └─ 方法3: 写入CoE对象

步骤2: 发送旧版本固件
  └─ 通过EtherCAT FoE发送 v1.0.0 固件

步骤3: 设备重启
  └─ 运行 v1.0.0 固件

步骤4: 恢复版本号检查 (可选)
  └─ 重新启用版本号检查
```

---

## 安全建议

### ⚠️ 降级风险

1. **配置不兼容**
   - 旧版本可能不支持新配置
   - Boot Params格式可能不兼容

2. **功能缺失**
   - 旧版本缺少新功能
   - 可能影响系统运行

3. **安全漏洞**
   - 旧版本可能有已知漏洞
   - 建议保持最新版本

### ✅ 安全措施

1. **备份当前配置**
   - 记录当前Boot Params
   - 记录当前版本号

2. **测试降级**
   - 先在测试环境验证
   - 确认旧版本可正常运行

3. **快速回滚**
   - 保留新版本固件
   - 随时可以升级回来

4. **记录操作**
   - 记录降级原因
   - 记录降级时间和版本

---

## 配置命令详解

### 命令结构（20字节）

```
偏移0-6:   魔数 "BOOTCFG" (7字节)
偏移7:     命令类型 (1字节)
偏移8-15:  参数 (8字节)
偏移16-19: CRC32校验 (4字节)
```

### 命令类型

| 值 | 名称 | 说明 |
|----|------|------|
| 0x01 | DISABLE_VERSION_CHECK | 禁用版本号检查 |
| 0x02 | ENABLE_VERSION_CHECK | 启用版本号检查 |
| 0x03 | SET_TARGET_BANK | 设置目标Bank |
| 0x04 | GET_CURRENT_CONFIG | 获取当前配置 |

---

## 使用示例

### 示例1：从v2.1.0降级到v1.0.0

```bash
# 1. 生成禁用命令
python boot_config_tool.py --disable-version-check

# 2. 通过FoE发送 disable_version_check.bin

# 3. 通过FoE发送 v1.0.0 固件

# 4. 设备重启，运行v1.0.0

# 5. 恢复版本号检查
python boot_config_tool.py --enable-version-check
# 通过FoE发送 enable_version_check.bin
```

---

### 示例2：强制切换到Bank0

```bash
# 生成设置Bank0命令
python boot_config_tool.py --set-target-bank 0

# 通过FoE发送，下次启动使用Bank0
```

---

## 常见问题

### Q1: 降级后无法启动怎么办？

**答：**
1. 检查Boot Params是否正确
2. 检查Bank0/Bank1固件是否有效
3. 使用SBL恢复机制
4. 重新烧录固件

---

### Q2: 如何查看当前版本号检查状态？

**答：**
```bash
# 方法1: 查看Boot Params
python -c "
import struct
with open('boot_params.bin', 'rb') as f:
    data = f.read()
    version_check = data[10]  # 偏移10
    print('Version Check:', 'Enabled' if version_check else 'Disabled')
"

# 方法2: 通过CoE读取对象0x8000
```

---

### Q3: 降级后Boot Params损坏怎么办？

**答：**
- SBL会自动检测并恢复Boot Params
- 选择版本号更高的Bank启动
- 如果两个Bank都损坏，停止启动

---

### Q4: 是否可以永久禁用版本号检查？

**答：**
可以，但不推荐：
```c
// 永久禁用
#define BOOT_PARAMS_VERSION_CHECK_ENABLE    0
```

**风险：**
- 失去版本保护
- 可能误降级到不兼容版本
- 建议仅在开发阶段禁用

---

## 最佳实践

1. **✅ 仅在必要时降级**
   - 优先考虑修复问题
   - 而不是回退到旧版本

2. **✅ 临时禁用版本号检查**
   - 降级后立即恢复
   - 不要长期禁用

3. **✅ 记录降级原因**
   - 记录问题和解决方案
   - 便于后续分析

4. **✅ 测试验证**
   - 降级前充分测试
   - 确认旧版本可用

5. **✅ 保留回滚路径**
   - 保存新版本固件
   - 随时可以升级回来

---

## 作者

Jerry.Chen

## 日期

2026-04-13

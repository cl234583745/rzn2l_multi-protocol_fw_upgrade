# Post-build配置说明

## 原配置（双固件方案）

```
cmd /c copy RZN2L_xspi0_app1.bin RZN2L_xspi0_app1_bank0.bin && python "${ProjDirPath}\attach_crc_bank0.py"
```

## 新配置（单固件方案）

### 方式1：自动读取C代码中的版本号（推荐）

```
python "${ProjDirPath}\attach_crc.py"
```

**说明：**
- 自动从 `src/ethercat/beckhoff/Src/bootmode.c` 读取 `APP_VERSION` 宏
- 无需手动指定版本号
- 修改C代码中的版本号后，编译时自动使用新版本号

**C代码中的版本号定义：**
```c
// 在 bootmode.c 中
#ifndef APP_VERSION
#define APP_VERSION 0x00010000  // 默认v1.0.0
#endif
```

**版本号格式：** `0x00MMmmpp`
- MM = 主版本号 (Major)
- mm = 次版本号 (Minor)
- pp = 补丁版本号 (Patch)

**示例：**
- `0x00010000` = v1.0.0
- `0x00010203` = v1.2.3
- `0x00020100` = v2.1.0

---

### 方式2：命令行指定版本号

```
python "${ProjDirPath}\attach_crc.py" --version 1.2.3
```

**说明：**
- 优先级最高，覆盖C代码中的版本号
- 适用于临时测试或特殊版本

---

### 方式3：指定目标Bank

```
python "${ProjDirPath}\attach_crc.py" --version 1.2.3 --target-bank 0
```

**参数说明：**
- `--target-bank 0`: 强制写入Bank0
- `--target-bank 1`: 强制写入Bank1
- `--target-bank 255`: 自动模式（默认）

---

## 配置步骤

### 在e² studio中配置：

1. 右键项目 `RZN2L_xspi0_app1` → Properties
2. C/C++ Build → Settings
3. Build Steps → Post-build steps
4. 在 Command 中输入：

```
python "${ProjDirPath}\attach_crc.py"
```

5. 点击 Apply and Close

---

## 版本号修改方法

### 方法1：修改C代码（推荐）

**文件：** `RZN2L_xspi0_app1/src/ethercat/beckhoff/Src/bootmode.c`

```c
// 修改版本号为 v1.2.3
#define APP_VERSION 0x00010203
```

**优点：**
- 版本号在代码中可见
- Git可追踪版本变化
- 编译时自动使用

---

### 方法2：通过构建配置

**在e² studio中：**

1. 右键项目 → Properties
2. C/C++ Build → Settings
3. Tool Settings → Cross ARM C Compiler → Preprocessor
4. 在 Defined symbols 中添加：

```
APP_VERSION=0x00010203
```

**优点：**
- 不修改源代码
- 不同构建配置可使用不同版本号

---

### 方法3：Post-build命令行

**Post-build步骤：**

```
python "${ProjDirPath}\attach_crc.py" --version 1.2.3
```

**优点：**
- 灵活性最高
- 可用于CI/CD自动化

---

## 版本号优先级

```
1. 命令行参数 --version        (最高优先级)
2. C代码中的 APP_VERSION 宏
3. 默认值 1.0.0                (最低优先级)
```

---

## 输出文件

**输入：** `BUILD/RZN2L_xspi0_app1.bin` (编译器生成)

**输出：** `BUILD/RZN2L_xspi0_app1_with_crc.bin` (最终固件)

**固件结构：**
```
偏移0-2:   "APP" (魔数)
偏移3-6:   版本号 (例如: 0x00010203)
偏移7:     目标Bank (0xFF=自动)
偏移8-11:  固件总长度
偏移12-75: 保留字段
偏移76-91: Identify信息
偏移92-N:  代码段
偏移N-3-N: CRC32校验码
```

---

## 测试验证

### 1. 查看固件头信息

```bash
# 使用hexdump查看前100字节
hexdump -C RZN2L_xspi0_app1_with_crc.bin | head -n 10

# 或使用Python
python -c "
import struct
with open('BUILD/RZN2L_xspi0_app1_with_crc.bin', 'rb') as f:
    data = f.read(100)
    print('Magic:', data[0:3])
    print('Version:', hex(struct.unpack('<I', data[3:7])[0]))
    print('Target Bank:', data[7])
    print('Total Size:', struct.unpack('<I', data[8:12])[0])
"
```

### 2. 验证CRC

```bash
python -c "
import zlib
with open('BUILD/RZN2L_xspi0_app1_with_crc.bin', 'rb') as f:
    data = f.read()
    calc_crc = zlib.crc32(data[:-4]) & 0xFFFFFFFF
    stored_crc = int.from_bytes(data[-4:], 'little')
    print('Calculated CRC:', hex(calc_crc))
    print('Stored CRC:', hex(stored_crc))
    print('CRC Valid:', calc_crc == stored_crc)
"
```

---

## 常见问题

### Q1: 找不到Python

**错误：** `'python' 不是内部或外部命令`

**解决：**
1. 确保Python已安装并添加到PATH
2. 或使用完整路径：`C:\Python312\python.exe "${ProjDirPath}\attach_crc.py"`

---

### Q2: 找不到输入文件

**错误：** `Input file not found!`

**解决：**
1. 检查编译是否成功
2. 检查输出目录是否为 `BUILD`
3. 检查输出文件名是否为 `RZN2L_xspi0_app1.bin`

---

### Q3: 版本号读取失败

**错误：** `APP_VERSION not found in bootmode.c`

**解决：**
1. 检查 `bootmode.c` 中是否定义了 `APP_VERSION`
2. 或使用命令行参数：`--version 1.0.0`

---

## 示例输出

```
============================================================
RZN2L Firmware CRC Attachment Script (Single Firmware)
============================================================
[INFO] Project Root: E:\RS_workspace\RZN2L_Multi-protocol_FW_Upgrade\RZN2L_xspi0_app1
[INFO] Found APP_VERSION in C code: 0x00010203 -> 0x00010203
[INFO] Version from C code: 1.2.3 -> 0x00010203
[INFO] Target Bank: Auto (0xFF)
[INFO] Input:  BUILD/RZN2L_xspi0_app1.bin
[INFO] Output: BUILD/RZN2L_xspi0_app1_with_crc.bin
[INFO] Original size: 131072 bytes
[INFO] Firmware header:
       Magic: APP
       Version: 0x00010203 (1.2.3)
       Target Bank: 255 (Auto)
       Total Size: 131076 bytes
[INFO] CRC-32: 0xA1B2C3D4
[SUCCESS] Output: BUILD/RZN2L_xspi0_app1_with_crc.bin
[SUCCESS] Size: 131076 bytes (131072 original + 4 CRC)
============================================================
```

---

## 作者

Jerry.Chen

## 日期

2026-04-13

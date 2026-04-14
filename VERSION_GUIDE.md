# 版本号使用说明

## 版本号格式

采用**语义化版本规范** (Semantic Versioning)：`Major.Minor.Patch`

```
APP_VERSION_MAJOR  主版本号 (不兼容的API修改)
APP_VERSION_MINOR  次版本号 (向下兼容的功能性新增)
APP_VERSION_PATCH  补丁版本号 (向下兼容的问题修正)
```

---

## 修改方法

### 文件位置
`RZN2L_xspi0_app1/src/ethercat/beckhoff/Src/bootmode.c`

### 修改示例

```c
// 发布 v1.0.0 版本
#define APP_VERSION_MAJOR  1
#define APP_VERSION_MINOR  0
#define APP_VERSION_PATCH  0

// 发布 v1.0.1 版本 (修复bug)
#define APP_VERSION_MAJOR  1
#define APP_VERSION_MINOR  0
#define APP_VERSION_PATCH  1

// 发布 v1.1.0 版本 (新增功能)
#define APP_VERSION_MAJOR  1
#define APP_VERSION_MINOR  1
#define APP_VERSION_PATCH  0

// 发布 v2.0.0 版本 (重大更新)
#define APP_VERSION_MAJOR  2
#define APP_VERSION_MINOR  0
#define APP_VERSION_PATCH  0
```

---

## 版本号范围

每个字段范围：**0 ~ 255**

- 主版本号：0-255 (足够使用)
- 次版本号：0-255 (足够使用)
- 补丁版本号：0-255 (足够使用)

**总版本数：** 256 × 256 × 256 = **16,777,216** 个不同版本

---

## 版本号比较规则

升级时，新版本号必须**大于**当前版本号：

```
比较优先级: Major > Minor > Patch

示例:
v1.0.0 < v1.0.1  ✅ (Patch增加)
v1.0.1 < v1.1.0  ✅ (Minor增加)
v1.1.0 < v2.0.0  ✅ (Major增加)
v1.2.3 < v1.2.4  ✅ (Patch增加)
v1.2.3 < v1.3.0  ✅ (Minor增加)
v1.2.3 < v2.0.0  ✅ (Major增加)

v1.0.1 > v1.0.0  ❌ (降级，拒绝)
v2.0.0 > v1.9.9  ❌ (降级，拒绝)
```

---

## 内部存储格式

### C代码中的组合

```c
// 自动组合，无需手动计算
#define APP_VERSION ((APP_VERSION_MAJOR << 16) | (APP_VERSION_MINOR << 8) | APP_VERSION_PATCH)

// 示例:
// v1.2.3 -> (1 << 16) | (2 << 8) | 3 = 0x00010203
// v2.1.0 -> (2 << 16) | (1 << 8) | 0 = 0x00020100
```

### Flash存储格式

固件头中的版本号（4字节，小端序）：

```
偏移3:  Patch    (0-255)
偏移4:  Minor    (0-255)
偏移5:  Major    (0-255)
偏移6:  保留     (0x00)
```

**示例：v1.2.3**

```
偏移3: 0x03  (Patch = 3)
偏移4: 0x02  (Minor = 2)
偏移5: 0x01  (Major = 1)
偏移6: 0x00  (保留)
```

---

## 版本号修改建议

### 何时增加Major？

- 不兼容的API修改
- 重大架构调整
- 破坏性变更

**示例：**
```
v1.x.x -> v2.0.0
- 修改了EtherCAT对象字典结构
- 修改了固件头格式
- 修改了Boot Params结构
```

---

### 何时增加Minor？

- 新增功能
- 功能增强
- 向下兼容的改进

**示例：**
```
v1.0.x -> v1.1.0
- 新增诊断功能
- 新增LED控制
- 优化升级速度
```

---

### 何时增加Patch？

- Bug修复
- 小改进
- 文档更新

**示例：**
```
v1.0.0 -> v1.0.1
- 修复CRC计算错误
- 修复Flash写入问题
- 优化日志输出
```

---

## 版本号历史记录建议

建议在项目文档中记录版本历史：

```markdown
## 版本历史

### v2.1.0 (2026-04-13)
- 新增: 版本号检查功能
- 新增: Bank自动检测
- 改进: 单固件升级方案

### v2.0.0 (2026-04-01)
- 重大更新: 重构固件头结构
- 重大更新: 新Boot Params格式

### v1.2.3 (2026-03-15)
- 修复: CRC校验错误
- 修复: Flash写入问题

### v1.2.0 (2026-03-01)
- 新增: EtherCAT FoE升级
- 新增: 双Bank支持

### v1.0.0 (2026-01-01)
- 初始版本
```

---

## 常见问题

### Q1: 版本号不够用怎么办？

**答：** 每个字段0-255，总共16,777,216个版本，足够使用。

如果真的不够：
- Major达到255时，考虑重置为v1.0.0（新项目）
- Minor达到255时，增加Major
- Patch达到255时，增加Minor

---

### Q2: 如何查看当前版本号？

**方法1：查看C代码**
```bash
grep "APP_VERSION_MAJOR" src/ethercat/beckhoff/Src/bootmode.c
grep "APP_VERSION_MINOR" src/ethercat/beckhoff/Src/bootmode.c
grep "APP_VERSION_PATCH" src/ethercat/beckhoff/Src/bootmode.c
```

**方法2：查看固件头**
```bash
python -c "
import struct
with open('Debug/RZN2L_xspi0_app1_with_crc.bin', 'rb') as f:
    data = f.read(8)
    patch = data[3]
    minor = data[4]
    major = data[5]
    print(f'Version: {major}.{minor}.{patch}')
"
```

**方法3：通过EtherCAT主站**
- 读取Slave Information Interface (SII)
- 查看Revision Number字段

---

### Q3: 如何禁用版本号检查？

**方法1：修改Boot Params**
```c
// 在 flash_config.h 中
#define BOOT_PARAMS_VERSION_CHECK_ENABLE  0
```

**方法2：运行时修改**
- 通过EtherCAT写入Boot Params
- 设置 `version_check_enable = 0`

**注意：** 不建议禁用，版本号检查是重要的安全机制。

---

### Q4: 如何降级固件？

**默认：** 不允许降级（版本号检查启用）

**如果需要降级：**
1. 临时禁用版本号检查
2. 升级到旧版本
3. 重新启用版本号检查

**风险：** 降级可能导致配置不兼容，谨慎操作。

---

## 版本号最佳实践

1. **✅ 遵循语义化版本规范**
   - Major: 不兼容变更
   - Minor: 功能新增
   - Patch: Bug修复

2. **✅ 记录版本历史**
   - 每个版本记录变更内容
   - 标注发布日期

3. **✅ 保持版本号递增**
   - 不要跳过版本号
   - 不要重复版本号

4. **✅ 使用Git标签**
   ```bash
   git tag -a v2.1.0 -m "Release v2.1.0"
   git push origin v2.1.0
   ```

5. **✅ 测试版本升级**
   - 测试升级流程
   - 测试版本号检查
   - 测试回滚机制

---

## 作者

Jerry.Chen

## 日期

2026-04-13

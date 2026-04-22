#!/usr/bin/env python3
"""
RZN2L固件CRC附加脚本 (单固件版本)

功能:
1. 读取编译生成的bin文件
2. 修改固件头: "APP" + 版本号 + 目标Bank
3. 计算并附加CRC32校验码
4. 输出带CRC的固件文件

使用方法:
    python attach_crc.py [--version VERSION] [--target-bank BANK]

参数:
    --version: 固件版本号 (格式: major.minor.patch, 例如: 1.2.3)
               如果不指定，则从C代码中读取APP_VERSION宏
    --target-bank: 目标Bank (0=Bank0, 1=Bank1, 255=自动)
                   默认: 255 (自动模式)

作者: Jerry.Chen
日期: 2026-04-13
"""

import sys
import zlib
import os
import struct
import argparse
import re

# 配置
BUILD_DIR_NAME = "Debug"  # 编译输出目录
BIN_BASE_NAME = "RZN2L_xspi0_app1"

def parse_version(version_str):
    """
    解析版本号字符串
    输入: "major.minor.patch.revision" (例如: "1.2.3.4") 或 "major.minor.patch" (例如: "1.2.3")
    输出: 32位版本号 (例如: 0x01020304)
    """
    parts = version_str.split('.')
    if len(parts) not in [3, 4]:
        raise ValueError(f"Invalid version format: {version_str}. Expected: major.minor.patch or major.minor.patch.revision")

    major = int(parts[0])
    minor = int(parts[1])
    patch = int(parts[2])
    revision = int(parts[3]) if len(parts) == 4 else 0

    if not (0 <= major <= 255 and 0 <= minor <= 255 and 0 <= patch <= 255 and 0 <= revision <= 255):
        raise ValueError(f"Version numbers must be 0-255: {version_str}")

    if len(parts) == 4:
        version = (major << 24) | (minor << 16) | (patch << 8) | revision
    else:
        version = (major << 16) | (minor << 8) | patch
    return version

def read_app1_str_from_flash_config(project_root):
    """
    从flash_config.h中读取APP1_STR
    """
    flash_config_h = os.path.join(os.path.dirname(project_root), "common", "flash_config.h")

    if not os.path.exists(flash_config_h):
        print(f"[WARNING] Cannot find flash_config.h: {flash_config_h}")
        return None

    try:
        with open(flash_config_h, 'r', encoding='utf-8') as f:
            content = f.read()

        pattern = r'#define\s+APP1_STR\s+"(\w+)"'
        match = re.search(pattern, content)
        if match:
            result = match.group(1)
            print(f"[INFO] Found APP1_STR in flash_config.h: '{result}'")
            return result

        print(f"[WARNING] APP1_STR not found in {flash_config_h}")
        return None

    except Exception as e:
        print(f"[ERROR] Failed to read flash_config.h: {e}")
        return None

def read_version_from_flash_config(project_root):
    """
    从flash_config.h中读取APP1版本号
    支持两种格式:
    1. APP1_VERSION_STR "X.Y.Z.W"
    2. APP1_VERSION ((APP1_VERSION_MAJOR << 24) | ...)
    """
    flash_config_h = os.path.join(os.path.dirname(project_root), "common", "flash_config.h")

    if not os.path.exists(flash_config_h):
        print(f"[WARNING] Cannot find flash_config.h: {flash_config_h}")
        return None

    try:
        with open(flash_config_h, 'r', encoding='utf-8') as f:
            content = f.read()

        # 方法1: 读取APP1_VERSION_STR字符串
        str_match = re.search(r'#define\s+APP1_VERSION_STR\s+"(\d+)\.(\d+)\.(\d+)\.(\d+)"', content)
        if str_match:
            major = int(str_match.group(1))
            minor = int(str_match.group(2))
            patch = int(str_match.group(3))
            revision = int(str_match.group(4))
            version = (major << 24) | (minor << 16) | (patch << 8) | revision
            version_str = f"{major}.{minor}.{patch}.{revision}"
            print(f"[INFO] Found APP1_VERSION in flash_config.h: {version_str} -> 0x{version:08X}")
            return version

        # 方法2: 读取组合定义的APP1_VERSION表达式
        expr_match = re.search(r'#define\s+APP1_VERSION\s+\(\(APP1_VERSION_MAJOR\s+<<\s+24\)\s+\|\s+\(APP1_VERSION_MINOR\s+<<\s+16\)\s+\|\s+\(APP1_VERSION_PATCH\s+<<\s+8\)\s+\|\s+APP1_VERSION_REVISION\)', content)
        if expr_match:
            major_match = re.search(r'#define\s+APP1_VERSION_MAJOR\s+(\d+)', content)
            minor_match = re.search(r'#define\s+APP1_VERSION_MINOR\s+(\d+)', content)
            patch_match = re.search(r'#define\s+APP1_VERSION_PATCH\s+(\d+)', content)
            revision_match = re.search(r'#define\s+APP1_VERSION_REVISION\s+(\d+)', content)

            if major_match and minor_match and patch_match and revision_match:
                major = int(major_match.group(1))
                minor = int(minor_match.group(1))
                patch = int(patch_match.group(1))
                revision = int(revision_match.group(1))
                version = (major << 24) | (minor << 16) | (patch << 8) | revision
                version_str = f"{major}.{minor}.{patch}.{revision}"
                print(f"[INFO] Found APP1_VERSION in flash_config.h: {version_str} -> 0x{version:08X}")
                return version

        print(f"[WARNING] APP1_VERSION not found in {flash_config_h}")
        return None

    except Exception as e:
        print(f"[ERROR] Failed to read flash_config.h: {e}")
        return None

def main():
    # 解析命令行参数
    parser = argparse.ArgumentParser(description='RZN2L Firmware CRC Attachment Script')
    parser.add_argument('--version', type=str, default=None,
                        help='Firmware version (format: major.minor.patch, e.g., 1.2.3). If not specified, read from C code.')
    args = parser.parse_args()

    print("=" * 60)
    print("RZN2L Firmware CRC Attachment Script (Single Firmware)")
    print("=" * 60)

    # 确定项目根目录
    PROJECT_ROOT = os.path.dirname(os.path.abspath(__file__))
    os.chdir(PROJECT_ROOT)
    print(f"[INFO] Project Root: {PROJECT_ROOT}")

    # APP1_STR 优先级:
    # 1. flash_config.h中的 APP1_STR
    # 2. 默认值 "APP1"
    app1_str = read_app1_str_from_flash_config(PROJECT_ROOT)
    if app1_str is None:
        app1_str = "APP1"
        print(f"[INFO] APP1_STR from default: '{app1_str}'")

    # 版本号优先级:
    # 1. 命令行参数 --version
    # 2. C代码中的 APP_VERSION 宏
    # 3. 默认值 1.0.0
    version = None
    version_str = None

    if args.version:
        # 优先使用命令行参数
        try:
            version = parse_version(args.version)
            version_str = args.version
            print(f"[INFO] Version from command line: {version_str} -> 0x{version:08X}")
        except ValueError as e:
            print(f"[ERROR] {e}")
            sys.exit(1)
    else:
        # 尝试从flash_config.h读取
        version = read_version_from_flash_config(PROJECT_ROOT)
        if version is not None:
            major = (version >> 24) & 0xFF
            minor = (version >> 16) & 0xFF
            patch = (version >> 8) & 0xFF
            revision = version & 0xFF
            if revision != 0:
                version_str = f"{major}.{minor}.{patch}.{revision}"
            else:
                version_str = f"{major}.{minor}.{patch}"
            print(f"[INFO] Version from C code: {version_str} -> 0x{version:08X}")
        else:
            version_str = "1.0.0.0"
            version = parse_version(version_str)
            print(f"[INFO] Version from default: {version_str} -> 0x{version:08X}")

    # 输入输出文件路径
    input_bin = os.path.join(BUILD_DIR_NAME, BIN_BASE_NAME + ".bin")
    output_bin = os.path.join(BUILD_DIR_NAME, BIN_BASE_NAME + "_with_crc.bin")

    print(f"[INFO] Input:  {input_bin}")
    print(f"[INFO] Output: {output_bin}")

    # 检查输入文件
    if not os.path.exists(input_bin):
        print(f"[ERROR] Input file not found!")
        print(f"        Full path: {os.path.abspath(input_bin)}")
        sys.exit(1)

    # 读取原始数据
    with open(input_bin, 'rb') as f:
        data = bytearray(f.read())

    print(f"[INFO] Original size: {len(data)} bytes")

    # 构建固件头
    # 偏移0-3: APP1_STR (4字节)
    app1_str_bytes = app1_str.encode('ascii')
    if len(app1_str_bytes) != 4:
        print(f"[ERROR] APP1_STR must be 4 bytes, got: {app1_str}")
        sys.exit(1)
    data[0:4] = app1_str_bytes

    # 偏移4-7: 版本号 (小端序)
    data[4:8] = struct.pack('<I', version)

    # 偏移8-11: 固件总长度 (原始数据 + 4字节CRC)
    total_size = len(data) + 4
    data[8:12] = struct.pack('<I', total_size)

    print(f"[INFO] Firmware header:")
    print(f"       APP1_STR: '{app1_str}'")
    print(f"       Version: 0x{version:08X} ({version_str})")
    print(f"       Total Size: {total_size} bytes")

    # 计算CRC-32
    crc_initial = zlib.crc32(data) & 0xFFFFFFFF
    print(f"[INFO] CRC-32: 0x{crc_initial:08X}")

    # 将CRC追加到末尾
    crc_bytes = struct.pack('<I', crc_initial)

    # 写入输出文件
    with open(output_bin, 'wb') as f:
        f.write(data)
        f.write(crc_bytes)

    # 复制为.efw文件
    output_efw = output_bin.replace('.bin', '.efw')
    import shutil
    shutil.copy(output_bin, output_efw)

    print(f"[SUCCESS] Output: {output_bin}")
    print(f"[SUCCESS] Output: {output_efw}")
    print(f"[SUCCESS] Size: {total_size} bytes ({len(data)} original + 4 CRC)")
    print("=" * 60)

if __name__ == "__main__":
    main()

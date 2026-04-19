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

def read_app1_str_from_c_code(project_root):
    """
    从C代码中读取APP1_STR_1-4
    """
    bootmode_c = os.path.join(project_root, "src", "ethercat", "beckhoff", "Src", "bootmode.c")

    if not os.path.exists(bootmode_c):
        print(f"[WARNING] Cannot find bootmode.c: {bootmode_c}")
        return None

    try:
        with open(bootmode_c, 'r', encoding='utf-8') as f:
            content = f.read()

        str_values = []
        for i in range(1, 5):
            pattern = rf'#define\s+APP1_STR_{i}\s+(\'.\')'
            match = re.search(pattern, content)
            if match:
                char = match.group(1).strip("'")
                str_values.append(char)
            else:
                print(f"[WARNING] APP1_STR_{i} not found in {bootmode_c}")
                return None

        if len(str_values) == 4:
            result = ''.join(str_values)
            print(f"[INFO] Found APP1_STR in C code: '{result}'")
            return result

    except Exception as e:
        print(f"[ERROR] Failed to read C code: {e}")
        return None

    return None

def read_version_from_c_code(project_root):
    """
    从C代码中读取APP1版本号
    支持两种格式:
    1. 分开定义: APP1_VERSION_MAJOR, APP1_VERSION_MINOR, APP1_VERSION_PATCH, APP1_VERSION_REVISION
    2. 组合定义: APP1_VERSION 0xXXXXXXXX
    """
    bootmode_c = os.path.join(project_root, "src", "ethercat", "beckhoff", "Src", "bootmode.c")

    if not os.path.exists(bootmode_c):
        print(f"[WARNING] Cannot find bootmode.c: {bootmode_c}")
        return None

    try:
        with open(bootmode_c, 'r', encoding='utf-8') as f:
            lines = f.readlines()

        # 方法1: 读取分开定义的版本号 (优先, 支持3或4分量)
        major_value = None
        minor_value = None
        patch_value = None
        revision_value = None

        for line in lines:
            stripped = line.strip()
            if stripped.startswith('//') or stripped.startswith('/*'):
                continue

            major_match = re.match(r'#define\s+APP1_VERSION_MAJOR\s+(\d+)', stripped)
            minor_match = re.match(r'#define\s+APP1_VERSION_MINOR\s+(\d+)', stripped)
            patch_match = re.match(r'#define\s+APP1_VERSION_PATCH\s+(\d+)', stripped)
            revision_match = re.match(r'#define\s+APP1_VERSION_REVISION\s+(\d+)', stripped)

            if major_match:
                major_value = int(major_match.group(1))
            if minor_match:
                minor_value = int(minor_match.group(1))
            if patch_match:
                patch_value = int(patch_match.group(1))
            if revision_match:
                revision_value = int(revision_match.group(1))

        if major_value is not None and minor_value is not None and patch_value is not None:
            if revision_value is not None:
                version = (major_value << 24) | (minor_value << 16) | (patch_value << 8) | revision_value
                version_str = f"{major_value}.{minor_value}.{patch_value}.{revision_value}"
            else:
                version = (major_value << 16) | (minor_value << 8) | patch_value
                version_str = f"{major_value}.{minor_value}.{patch_value}"
            print(f"[INFO] Found APP1 version in C code: {version_str} -> 0x{version:08X}")
            return version

        # 方法2: 读取组合定义的版本号 (备用)
        for line in lines:
            stripped = line.strip()
            if stripped.startswith('//') or stripped.startswith('/*'):
                continue

            match = re.match(r'#define\s+APP1_VERSION\s+(0x[0-9A-Fa-f]+)', stripped)
            if match:
                version_hex = match.group(1)
                version = int(version_hex, 16)
                print(f"[INFO] Found APP1_VERSION in C code: {version_hex} -> 0x{version:08X}")
                return version

        print(f"[WARNING] APP1 version not found in {bootmode_c}")
        return None

    except Exception as e:
        print(f"[ERROR] Failed to read C code: {e}")
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
    # 1. C代码中的 APP1_STR_1-4
    # 2. 默认值 "APP1"
    app1_str = read_app1_str_from_c_code(PROJECT_ROOT)
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
        # 尝试从C代码读取
        version = read_version_from_c_code(PROJECT_ROOT)
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

    print(f"[SUCCESS] Output: {output_bin}")
    print(f"[SUCCESS] Size: {total_size} bytes ({len(data)} original + 4 CRC)")
    print("=" * 60)

if __name__ == "__main__":
    main()

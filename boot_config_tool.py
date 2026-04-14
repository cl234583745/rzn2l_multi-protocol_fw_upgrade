#!/usr/bin/env python3
"""
Boot配置命令生成工具

功能:
1. 生成禁用版本号检查的命令
2. 生成启用版本号检查的命令
3. 生成设置目标Bank的命令

使用方法:
    # 禁用版本号检查
    python boot_config_tool.py --disable-version-check

    # 启用版本号检查
    python boot_config_tool.py --enable-version-check

    # 设置目标Bank
    python boot_config_tool.py --set-target-bank 0

作者: Jerry.Chen
日期: 2026-04-13
"""

import sys
import struct
import zlib
import argparse

# 配置命令魔数
BOOT_CONFIG_CMD_MAGIC = b"BOOTCFG"

# 命令类型
BOOT_CONFIG_CMD_DISABLE_VERSION_CHECK = 0x01
BOOT_CONFIG_CMD_ENABLE_VERSION_CHECK  = 0x02
BOOT_CONFIG_CMD_SET_TARGET_BANK       = 0x03
BOOT_CONFIG_CMD_GET_CURRENT_CONFIG    = 0x04

def generate_config_command(cmd_type, param=None):
    """
    生成配置命令
    """
    # 构建命令结构
    magic = BOOT_CONFIG_CMD_MAGIC
    param_bytes = param if param else b'\x00' * 8

    # 组合数据（不包括CRC）
    data = magic + bytes([cmd_type]) + param_bytes[:8]

    # 计算CRC
    crc = zlib.crc32(data) & 0xFFFFFFFF

    # 组合完整命令
    cmd = data + struct.pack('<I', crc)

    return cmd

def main():
    parser = argparse.ArgumentParser(description='Boot Config Command Generator')
    parser.add_argument('--disable-version-check', action='store_true',
                        help='Disable version check (allow downgrade)')
    parser.add_argument('--enable-version-check', action='store_true',
                        help='Enable version check (prevent downgrade)')
    parser.add_argument('--set-target-bank', type=int, choices=[0, 1],
                        help='Set target bank (0 or 1)')
    parser.add_argument('--output', type=str, default='boot_config_cmd.bin',
                        help='Output file name')

    args = parser.parse_args()

    cmd = None
    description = ""

    if args.disable_version_check:
        cmd = generate_config_command(BOOT_CONFIG_CMD_DISABLE_VERSION_CHECK)
        description = "Disable Version Check"
        print("=" * 60)
        print("Generating: Disable Version Check Command")
        print("=" * 60)
        print("This command will:")
        print("  - Disable version number check")
        print("  - Allow firmware downgrade")
        print("  - Use with caution!")
        print("=" * 60)

    elif args.enable_version_check:
        cmd = generate_config_command(BOOT_CONFIG_CMD_ENABLE_VERSION_CHECK)
        description = "Enable Version Check"
        print("=" * 60)
        print("Generating: Enable Version Check Command")
        print("=" * 60)
        print("This command will:")
        print("  - Enable version number check")
        print("  - Prevent firmware downgrade")
        print("  - Require higher version for upgrade")
        print("=" * 60)

    elif args.set_target_bank is not None:
        param = bytes([args.set_target_bank]) + b'\x00' * 7
        cmd = generate_config_command(BOOT_CONFIG_CMD_SET_TARGET_BANK, param)
        description = f"Set Target Bank to Bank{args.set_target_bank}"
        print("=" * 60)
        print(f"Generating: Set Target Bank to Bank{args.set_target_bank}")
        print("=" * 60)
        print(f"This command will:")
        print(f"  - Set target bank to Bank{args.set_target_bank}")
        print(f"  - Next boot will use Bank{args.set_target_bank}")
        print("=" * 60)

    else:
        parser.print_help()
        return

    if cmd:
        # 写入文件
        with open(args.output, 'wb') as f:
            f.write(cmd)

        print(f"\n[SUCCESS] Command generated: {args.output}")
        print(f"[SUCCESS] Size: {len(cmd)} bytes")
        print(f"[SUCCESS] Description: {description}")

        # 显示十六进制
        print(f"\n[INFO] Hex dump:")
        for i in range(0, len(cmd), 16):
            hex_str = ' '.join(f'{b:02X}' for b in cmd[i:i+16])
            print(f"  {i:04X}: {hex_str}")

        print("\n" + "=" * 60)
        print("How to use:")
        print("  1. Send this file via EtherCAT FoE")
        print("  2. Device will process the command")
        print("  3. Configuration will be updated")
        print("=" * 60)

if __name__ == "__main__":
    main()

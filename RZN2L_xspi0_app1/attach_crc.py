#!/usr/bin/env python3
"""
RZN2L固件CRC附加脚本

功能:
1. 读取编译生成的bin文件
2. 读取头部信息用于打印（不修改）
3. 在第9-12字节填入固件总长度 (原始数据 + 4字节CRC)
4. 文件末尾附加CRC32校验码
5. 输出带CRC的固件文件

作者: Jerry.Chen
日期: 2026-04-22
"""

import sys
import zlib
import os
import struct

BUILD_DIR_NAME = "Debug"
BIN_BASE_NAME = "RZN2L_xspi0_app1"

def main():
    print("=" * 60)
    print("RZN2L Firmware CRC Attachment Script")
    print("=" * 60)

    PROJECT_ROOT = os.path.dirname(os.path.abspath(__file__))
    os.chdir(PROJECT_ROOT)
    print(f"[INFO] Project Root: {PROJECT_ROOT}")

    input_bin = os.path.join(BUILD_DIR_NAME, BIN_BASE_NAME + ".bin")
    output_bin = os.path.join(BUILD_DIR_NAME, BIN_BASE_NAME + "_with_crc.bin")

    print(f"[INFO] Input:  {input_bin}")
    print(f"[INFO] Output: {output_bin}")

    if not os.path.exists(input_bin):
        print(f"[ERROR] Input file not found!")
        print(f"        Full path: {os.path.abspath(input_bin)}")
        sys.exit(1)

    with open(input_bin, 'rb') as f:
        data = bytearray(f.read())

    print(f"[INFO] Original size: {len(data)} bytes")

    app1_str = data[0:4].decode('ascii')
    version = struct.unpack('<I', data[4:8])[0]

    print(f"[INFO] Firmware header:")
    print(f"       APP1_STR: '{app1_str}'")
    print(f"       Version: 0x{version:08X}")

    total_size = len(data) + 4
    print(f"       Total Size: 0x{total_size:08X} ({total_size} bytes)")

    data[8:12] = struct.pack('<I', total_size)

    crc = zlib.crc32(data) & 0xFFFFFFFF
    print(f"[INFO] CRC-32: 0x{crc:08X}")

    with open(output_bin, 'wb') as f:
        f.write(data)
        f.write(struct.pack('<I', crc))

    output_efw = output_bin.replace('.bin', '.efw')
    import shutil
    shutil.copy(output_bin, output_efw)

    print(f"[SUCCESS] Output: {output_bin}")
    print(f"[SUCCESS] Output: {output_efw}")
    print(f"[SUCCESS] Size: {total_size} bytes ({len(data)} original + 4 CRC)")
    print("=" * 60)

if __name__ == "__main__":
    main()
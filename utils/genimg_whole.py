#!/usr/bin/env python3

import sys
import struct
import hashlib

def genimgfile(loader_bin):
    # AES Cipher flag, 0x01 for AES encryption, 0x00 for none
    aes_cipher_flag = b'\x00'

    firmware_bin = open(loader_bin, 'rb').read()
    firmware_len = len(firmware_bin)

    # 64bytes align
    pad_len = (firmware_len + 37) % 64
    if pad_len != 0:
        pad_len = 64 - pad_len
        firmware_bin += bytearray(pad_len)
        firmware_len += pad_len

    data = aes_cipher_flag + struct.pack('I', firmware_len) + firmware_bin
    sha256_hash = hashlib.sha256(data).digest()
    firmware_with_header = data + sha256_hash
    return firmware_with_header



if len(sys.argv) != 4:
    print(sys.argv[0] + " <loader.img> <app.bin> <out.img>")
    sys.exit()

loader_img = open(sys.argv[1], 'rb').read()
loader_len = len(loader_img)
app_img = genimgfile(sys.argv[2])
app_len = len(app_img)

out_file = open(sys.argv[3], 'wb')
out_file.write(loader_img + bytearray(b'\xff') * (3 * 64 * 1024 - loader_len) + app_img + bytearray(b'\xff') * (320 * 1024 - app_len) + app_img)
out_file.close()


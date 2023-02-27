#!/usr/bin/env python3

import sys
import struct
import hashlib

if len(sys.argv) != 3:
    print(sys.argv[0] + " <infile> <outfile>")
    sys.exit()


def genimgfile(bin_file):
    # AES Cipher flag, 0x01 for AES encryption, 0x00 for none
    aes_cipher_flag = b'\x00'

    firmware_bin = open(bin_file, 'rb').read()
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


out_file = open(sys.argv[2], 'wb')
out_file.write(genimgfile(sys.argv[1]))
out_file.close()

sys.exit()


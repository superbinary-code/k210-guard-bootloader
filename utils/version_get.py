#!/usr/bin/env python3

import sys
import struct

if len(sys.argv) != 3:
    print(sys.argv[0] + " <input.bin>  <out.bin>")
    sys.exit()

with open(sys.argv[1], 'rb') as f:
    loader_img = f.read()

loader_len = len(loader_img)

offset = struct.unpack_from("<L", loader_img, 2)[0]
print("Offset value is", hex(offset))
offset -= 0x80000000

version = struct.unpack_from("<Q", loader_img, offset)[0]
print("Version value is", hex(version))

with open(sys.argv[2], 'wb') as f:
	version=struct.pack("<Q", version)
	f.write(version)

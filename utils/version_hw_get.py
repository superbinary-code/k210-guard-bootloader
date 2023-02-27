#!/usr/bin/env python3

import sys
import struct

if len(sys.argv) != 3:
    print(sys.argv[0] + " <input.bin>  <out.bin>")
    sys.exit()

with open(sys.argv[1], 'rb') as f:
    loader_img = f.read()

version_hw = struct.unpack_from("<L", loader_img, 32)[0]
print("Hardware Version value is", hex(version_hw))

with open(sys.argv[2], 'wb') as f:
	version=struct.pack("<Q", version_hw)
	f.write(version)

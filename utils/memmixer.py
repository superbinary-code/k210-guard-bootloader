#!/usr/bin/env python2
import sys

"""
@brief      Convert ECC ROM to FPGA format from stdin
@param      Convert mode
@return     The result
"""

"""
@brief      72 bit reverse by 2 bit
@param      data  The 72 bit data
@return     72 bit data result
"""
def reverse72by2(data):
    data = ((data & 0xccccccccccccccccccL) >> 2) | ((data & 0x333333333333333333L) << 2)
    data = ((data & 0xf0f0f0f0f0f0f0f0f0L) >> 4) | ((data & 0x0f0f0f0f0f0f0f0f0fL) << 4)
    data = ((data & 0xff0000ff0000ff0000L) >> 16) | ((data & 0x00ff0000ff0000ff00L)) | ((data & 0x0000ff0000ff0000ffL) << 16)
    data = ((data & 0xffffff000000000000L) >> 48) | ((data & 0x000000ffffff000000L)) | ((data & 0x000000000000ffffffL) << 48)
    return data

"""
@brief      72 bit reverse by 2 bit
@param      data  The 72 bit data
@return     72 bit data result
"""
def reverse72by2_special_verify(data):
    data = ((data & 0xccccccccccccccccccL) >> 2) | ((data & 0x333333333333333333L) << 2)
    data = ((data & 0xf0f0f0f0f0f0f0f0f0L) >> 4) | ((data & 0x0f0f0f0f0f0f0f0f0fL) << 4)
    msb = data & 0xff0000000000000000L
    data = ((data & 0xff00ff00ff00ff00L) >> 8) | ((data & 0x00ff00ff00ff00ffL) << 8)
    data = ((data & 0xffff0000ffff0000L) >> 16) | ((data & 0x0000ffff0000ffffL) << 16)
    data = ((data & 0xffffffff00000000L) >> 32) | ((data & 0x00000000ffffffffL) << 32)
    data = (data << 8) | (msb >> 64)
    return data

"""
@brief      72 bit reverse by 2 bit
@param      data  The 72 bit data
@return     72 bit data result
"""
def reverse72by2_normal_verify(data):
    result = 0L;
    for i in range(0, 71, 2):
        result = result | (((data & (3L << i)) >> i) << (70 - i))
    return result

i = 0
for line in sys.stdin:
    if line.startswith('@'):
        sys.stdout.write(line)
        continue
    data_orig = long(line, 16)
    data_new = reverse72by2(data_orig)
    data_normal_verify = reverse72by2_normal_verify(data_orig)
    data_special_verify = reverse72by2_special_verify(data_orig)
    if (data_new != data_normal_verify) or (data_new != data_special_verify):
        sys.stderr.write("Data verify error\n")
        sys.exit(-1)

    formated_str = '%018x' % data_new
    for ch in formated_str:
        i = i + 1
        sys.stdout.write(ch)
        if i % 16 == 0:
            sys.stdout.write('\n')

sys.exit(0)


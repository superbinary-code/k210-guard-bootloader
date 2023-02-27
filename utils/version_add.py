#!/usr/bin/env python3

import sys

if len(sys.argv) != 3:
    print(sys.argv[0] + " <a> <append>")
    sys.exit()

with open(sys.argv[1], 'ab') as a, open(sys.argv[2], 'rb') as b:
	a.write(b.read())

print("version add Finish")

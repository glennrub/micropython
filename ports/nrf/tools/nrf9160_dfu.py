# This file is part of the MicroPython project, http://micropython.org/
#
# The MIT License (MIT)
#
# Copyright (c) 2019 Glenn Ruben Bakke
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import sys, serial, struct, time

usage = """
Usage:

    nrf9160_dfu.py <binfile> <dev> <baudrate> 

Example:

    nrf9160_dfu.py build-pca10090/firmware.bin /dev/ttyACM0 1000000")
"""
if len(sys.argv) < 4:
    print(usage)
    sys.exit(1)

f = open(sys.argv[1], "rb")
c = f.read()
f.close()
image_len = len(c)
fragment_size = 4096 #8192
print(len(c))
packed_image_len = struct.pack("<I", image_len)
packed_fragment_size = struct.pack("<h", fragment_size)

s = serial.Serial(port=sys.argv[2], baudrate=int(sys.argv[3]), rtscts=True)
print("Waiting for DFU")
while not b'Secureboot TDFU\r\n' in s.readline():
    pass
print("DFU Started")
s.write(b'S')
echo = s.read(1)
if echo == b'S':
    s.write(packed_image_len)
    s.write(packed_fragment_size)
    print(s.read())
    num = image_len // fragment_size
    for i in range(0, num):
        s.write(c[i * fragment_size:(i * fragment_size)+fragment_size])
        print(i, "acked: ", s.read(1))
    s.write(c[num * fragment_size:])
    print(i, "acked: ", s.read(1))
    print(s.read())

s.close()


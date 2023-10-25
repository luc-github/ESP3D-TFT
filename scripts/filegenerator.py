#!/usr/bin/env python3

import os

output_file = "testdir/file"
output_ext = ".txt"

for i in range(1, 1000):
    filename = output_file + str(i).zfill(4) + output_ext
    f = open(filename, "w")
    f.write(filename)
    f.close()

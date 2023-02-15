#!/bin/python3
from PIL import Image
import sys, os

if __name__=='__main__':
    if len(sys.argv)>1:
        file = sys.argv[1]
        img = Image.open(file).convert('RGB').getdata()
        arr = [((x[0]>>3)<<11)|((x[1]>>2)<<5)|(x[2]>>3) for x in list(img)]
        f = open(f'{file}.c', 'w')
        print(f"unsigned char {file.replace('.','_')}[{len(arr)*2}] = {{\n", end='\t',file=f)
        for ix ,b in enumerate(arr):
            print(f"0x{b&0xff:02x}, 0x{b>>8:02x}, ", end='' if (ix+1) % 8 else '\n\t', file=f)
        print("};", file=f)
        f.close()

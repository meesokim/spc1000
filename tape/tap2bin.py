#!/bin/python3

import os, sys
from struct import *

def cntbit(x):
    LOOKUP = \
    [
        0, 1, 1, 2, 1, 2, 2, 3, 
        1, 2, 2, 3, 2, 3, 3, 4
    ]
    return LOOKUP[(x>>4)&0xf] + LOOKUP[x&0xf]
def printhex(bs, w):
    for ix, b in enumerate(bs):
        print(f'{b:02x}',end='')
        if ix % w == w-1 or ix == len(bs)-1:
            print()
        else:
            print(',',end='')
        
if __name__=='__main__':
    files = sys.argv[1:]
    for file in files:
        filename = file.lower()
        if '*' in filename:
            break
        if '.cas' in filename:
            data = ''.join(['0'*(8-len(bin(d)[2:]))+bin(d)[2:] for d in open(file, 'rb').read()])
            type = 'CAS'
            # print(data)
        elif '.tap' in filename:
            data = open(file, 'r').read()
            type = 'TAP'
        print(f'filename:{file}')
        print(f'type: {type}')
        index = 0
        offset = 0
        while(True):
            mark = '1'*40+'0'*40+'11'
            mark1 = '1'*20+'0'*20+'11'
            if mark in data[offset:]:
                index = data[offset:].index(mark)
                # print(f'start({index+offset:07d}):{data[index+offset:index+offset+82]}')
                begin = index+offset+82
                b = []
                cnt = 0
                for ix in range(0, 130):
                    s = begin+ix*9
                    binstr = data[s:s+9]
                    # print(binstr,end='')
                    if binstr[-1] == '1':
                        c = int(binstr[:8], 2)
                        if ix < 128:
                            # print(ix)
                            cnt += cntbit(c)                       
                        b.append(c)
                    else:
                        b.append(c)
                        print(s)
                        print(ix, data[s-10:s+20])
                        # sys.exit()
                print()
                # printhex(b, 16)
                f = unpack('<b17sHHHH102b', bytes(b[:-2])) + unpack('>H', bytes(b[-2:]))
                # print(bytes(b), f)
                print('type:','basic' if f[0] == 2 else 'machine')
                print('name:',f[1].decode('ascii',errors="ignore"))
                print('size:',f[2],'addr:',f'{f[3]:04x}~{f[2]+f[3]:04x}','exec:',f'{f[4]:04x}','prot:',f'{f[5]:04x}')
                print('checksum:',f[-1],cnt)
                offset += index + 81
                if mark1 in data[offset:]:
                    cnt = 0
                    index = data[offset:].index(mark1)
                    # print(f'mark1({index+offset:07d}):{data[index+offset:index+offset+42]}')
                    begin = index+offset+42
                    b = []
                    size = (f[2]+2)
                    for ix in range(0, 9 * size, 9):
                        s = begin+ix
                        binstr = data[s:s+9]
                        if binstr[-1] == '1':
                            c = int(binstr[:8], 2)
                            if ix < f[2]:
                                cnt += cntbit(c)
                                # print(cnt,end=',')                       
                            b.append(c)
                        else:
                            print(s)
                            print(ix, int(ix/9), binstr)                                
                            sys.exit()
                    printhex(b[-20:], 20)
                    print('size:', len(b)-2)
                    print('checksum:',unpack('>H', bytes(b[-2:]))[0],cnt,hex(cnt))                                
            else:
                break
            # print(f'size: {len(data)}')

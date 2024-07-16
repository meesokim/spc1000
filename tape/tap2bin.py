#!/bin/python3

import os, sys
from struct import *
from io import StringIO

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
        print(file)
        if '*' in filename:
            break
        if '.cas' == filename[-4:]:
            data = ''.join(['0'*(8-len(bin(d)[2:]))+bin(d)[2:] for d in open(file, 'rb').read()][16*8:])
            fmt = 'CAS'
            # print(data)
        elif '.tap' in filename:
            data = open(file, 'r').read()
            fmt = 'TAP'
        print(f'filename:{file}')
        print(f'format: {fmt}')
        tap = 'tap/Jet+set+willy.cas'
        if tap == file:
        # print('file compare', file == 'tap/Jet+set+willy.cas')
            data = data[:74908] + '00' + data[74909:]
            header = 'SPC-1000.CASfmt '
            conv = [int(data[i:i+8],2) for i in range(0, len(data), 8)]
            file = f'{file}.1'
            f = open(file, 'wb')
            f.write(header.encode())
            f.write(bytes(conv))
            f.close()
        #     # sys.exit()
        if 'tap.1' in file:
            ff = StringIO()
        else:
            ff = open(f'{file}.tap.1', 'w')
        index = 0
        offset = 0
        while(True):
            mark = '1'*40+'0'*40+'11'
            mark1 = '1'*20+'0'*20+'11'
            if mark in data[offset:]:
                index = data[offset:].index(mark)
                smark = offset+index                
                # print(f'start({index+offset:07d}):{data[index+offset:index+offset+82]}')
                # print(mark, end=' ')
                begin = index+offset+82
                sbuf = StringIO()
                sbuf.write('0')
                sbuf.write(mark)
                b = []
                cnt = 0
                for ix in range(0, 130):
                    s = begin+ix* 9
                    binstr = data[s:s+9]
                    # print(binstr,end=' ')
                    if binstr[-1] == '1':
                        c = int(binstr[:8], 2)
                        sbuf.write(binstr)
                        if ix < 128:
                            # print(ix)
                            cnt += cntbit(c)                       
                        b.append(c)
                    else:
                        b.append(c)
                        print(s)
                        print(ix, data[s-9:s], data[s:s+9], data[s+9:s+18])
                        # sys.exit()
                # printhex(b, 16)
                print()
                sbuf.write("\n")
                # print(sbuf.getvalue())
                f = unpack('<b17sHHHH102b', bytes(b[:-2])) + unpack('>H', bytes(b[-2:]))
                # print(bytes(b), f)
                print('type:','basic' if f[0] == 2 else 'machine')
                print('name:',f[1].decode('ascii',errors="ignore"))
                print('size:',f[2],'addr:',f'{f[3]:04x}~{f[2]+f[3]:04x}','exec:',f'{f[4]:04x}','prot:',f'{f[5]:04x}')
                print('checksum:',f[-1],cnt, 'OK' if f[-1] == cnt else 'FAIL')
                offset += index + 90 + 130 * 9
                # print(data[smark:s+9])
                if mark1 in data[offset:]:
                    cnt = 0
                    index = data[offset:].index(mark1)
                    smark = offset+index 
                    sbuf.write('0')
                    sbuf.write(mark1)
                    # print(mark1, end=' ')
                    # print(f'mark1({index+offset:07d}):{data[index+offset:index+offset+42]}')
                    begin = index+offset+42
                    b = []
                    size = (f[2]+2)
                    for ix in range(0, size):
                        s = begin+ix*9
                        binstr = data[s:s+9]
                        # print(binstr,end=' ')
                        if binstr[-1] == '1':
                            c = int(binstr[:8], 2)
                            sbuf.write(binstr)
                            if ix < f[2]:
                                cnt += cntbit(c)
                                # print(cnt,end=',')                         
                            b.append(c)
                        else:
                            print(s)
                            print(ix, int(ix/9), binstr)                                
                            sys.exit()
                    printhex(b[-20:], 20)
                    print()
                    sbuf.write("\n")
                    # print(sbuf.getvalue())
                    ff.write(sbuf.getvalue())
                    # print('size:', len(b)-2)
                    chksum = unpack('>H', bytes(b[-2:]))[0]
                    print('checksum:',chksum,cnt,hex(cnt), 'OK' if chksum == cnt else 'FAIL')  
                    # print()
                    # print(data[smark:s+9])
            else:
                for i in range(39, 20, -1):
                    m = mark[:i]
                    if m in data[offset:]:
                        ix = data[offset:].index(m)
                        # print(i, data[offset+ix:offset+ix+2000])
                        break
                break
        ff.close()
            # print(f'size: {len(data)}')

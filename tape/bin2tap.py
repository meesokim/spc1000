#!/bin/python3

import os, sys
from struct import *
from io import StringIO
import numpy as np

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

class Tap:
    def __init__(self):
        self.ftype = 2
        self.fname = 'UNITITLED'
        self.size = 0
        self.checksum = 0
        self.addr = 0x7c9d
        self.exec = 0
        self.prot = 0
        # self.bin = bytes('\1'*10, 'ascii')
        # self.set_bin(self.bin, 0x8000)

    def basic_to_bin(self, basic):
        return '\0'

    def set_basic(self, basic):
        self.ftype = 2
        self.addr = 0x7c9d
        self.bin = basic_to_bin(basic)        

    def set_bin(self, bin, addr, title='', auto_start=0, prot=0):
        self.ftype = 1
        self.bin = bin
        self.size = len(bin)
        if auto_start:
            self.exec = addr
        self.prot = prot
        self.addr = addr
        if len(title):
            self.fname = title
        print(self.size, self.fname)
        # print(self.bin)

    def make_header(self):
        self.header = self.ftype.to_bytes(1, byteorder='little') \
                    + bytes((self.fname+'\0'*16)[:17], 'ascii') \
                    + self.size.to_bytes(2, byteorder='little') \
                    + self.addr.to_bytes(2, byteorder='little') \
                    + self.exec.to_bytes(2, byteorder='little') \
                    + self.prot.to_bytes(2, byteorder='little') 
        self.checksum = np.sum([cntbit(x) for x in self.header])
        # print(bytes('\0'* 103, 'ascii'), int(self.checksum).to_bytes(2, byteorder='big'))
        self.header += (bytes('\0'* 102, 'ascii') + int(self.checksum).to_bytes(2, byteorder='big'))
        # print(self.header)    

    def header_decode(self, b=None):
        if b == None:
            b = self.header
        f = unpack('<b17sHHHH102b', bytes(b[:-2])) + unpack('>H', bytes(b[-2:]))
        cnt = np.sum([cntbit(x) for x in b[:-2]])
        print('type:','basic' if f[0] == 2 else 'machine')
        print('name:',f[1].decode('ascii',errors="ignore"))
        print('size:',f[2],'addr:',f'{f[3]:04x}~{(f[2]+f[3]-10):04x}','exec:',f'{f[4]:04x}','prot:',f'{f[5]:04x}')
        print('checksum:',f[-1],cnt, 'OK' if f[-1] == cnt else 'FAIL')      

    def tap_decode(self, data):
        offset=0
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
                print('size:',f[2],'addr:',f'{f[3]:04x}~{f[2]+f[3]-1:04x}','exec:',f'{f[4]:04x}','prot:',f'{f[5]:04x}')
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
                    # ff.write(sbuf.getvalue())
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
        # ff.close()        
    
    def tap_encode(self, b):
        s = ''
        self.checksum = 0
        for x in b:
            s += "{:08b}".format(x) + '1'
            self.checksum += cntbit(x)
        for x in int(self.checksum).to_bytes(2, byteorder='big'):
            s += "{:08b}".format(x) + '1'
        return s

    def encode(self):
        b = StringIO()
        mark0 = '1'*40+'0'*40+'11'
        mark1 = '1'*20+'0'*20+'11'
        b.write('0'*20)
        b.write(mark0)
        b.write(self.tap_encode(self.header[:-2]))
        b.write('1')
        b.write('0'*20)
        b.write(mark1)
        b.write(self.tap_encode(self.bin))
        b.write('0'*10)
        return b

import sys
if 'bin2tap' in sys.argv[0]:
    if len(sys.argv) < 4:
        print('bin2tap file title start_address [auto_start,]')
        exit()
    fname = sys.argv[1]
    if os.path.exists(fname):
        b = open(fname, 'rb').read()
    else:
        printf(f'no {fname} file exists')
        exit()
    if len(sys.argv) > 2:
        title = sys.argv[2]
    if len(sys.argv) > 3:
        addr = int(sys.argv[3], 16)
    auto_start = 0
    if len(sys.argv) > 4:
        auto_start = int(sys.argv[4])
    tap = Tap()
    # print(len(b), b)
    tap.set_bin(b, addr, title, addr, auto_start)
    tap.make_header()
    s = tap.encode().getvalue()
    open(f'{title}.tap', 'w').write(s)
    tap.tap_decode(s)
    exit()
        
if __name__=='__main__':
    files = sys.argv[1:]
    for file in files:
        filename = file.lower()
        print(file)
        if '*' in filename:
            break
        if '.cas' in filename and not '.tap' in filename:
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
                    print(binstr,end=' ')
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

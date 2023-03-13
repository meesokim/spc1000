import wave
import sys
import numpy as np
import pandas as pd
import struct
from matplotlib import pyplot as plt
import time

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
def tapinfo(data, offset=0):
    from struct import unpack
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
        info = {}
        info['type']='basic' if f[0] == 2 else 'machine'
        info['name']=f[1].decode('ascii',errors="ignore").split('\0')[0]  
        info['size']=f[2]
        info['addr']=f[3]
        info['exec']=f[4]
        info['prot']=f[5]
        info['chksum0']=f[-1]
        info['calcsum0']=cnt
        print('type:',info['type'])
        print('name:',info['name'])
        print('size:',info['size'],'addr:',f'{info["addr"]:04x}~{info["size"]+info["addr"]:04x}','exec:',f"{info['exec']:04x}",'prot:',f"{info['prot']:04x}")
        print('checksum:',info['chksum0'],info['calcsum0'])
        offset += index + 81
        if mark1 in data[offset:]:
            cnt = 0
            index = data[offset:].index(mark1)
            # print(f'mark1({index+offset:07d}):{data[index+offset:index+offset+42]}')
            begin = index+offset+42
            b = []
            size = f[2]+2
            for ix in range(0, size):
                s = begin+ix*9
                binstr = data[s:s+9]
                if binstr[-1] == '1':
                    c = int(binstr[:8], 2)
                    if ix < f[2]:
                        cnt += cntbit(c)
                        # print(cnt,end=',')                       
                    b.append(c)
                else:
                    print(s, ix, binstr)                                
            printhex(b[-20:], 20)
            print('size:', len(b)-2)
            info['chksum']=unpack('>H', bytes(b[-2:]))[0]
            info['calcsum']=cnt
            print('checksum:',info['chksum'],info['calcsum'],hex(cnt))
        return info
    return None

if __name__=='__main__':
    args = sys.argv[1:]
    argc = len(args)
    if argc < 1:
        print(sys.argv[0], "file.wav prefix")
        sys.exit()
    filename = args[0]
    # filename = 'contest_9_size_a.wav'
    if argc > 1:
        prefix=args[1]
    else:
        prefix='c9a'
    wf = wave.open(filename)
    samplewidth = wf.getsampwidth()
    nchannels = wf.getnchannels()
    rate = wf.getframerate()
    frames = bytearray(wf.readframes(wf.getnframes()))
    length = len(frames)
    print ("sample width = ",samplewidth)
    print ("frame rates = ", rate)
    print ("channels = ", nchannels)
    print ("length = ", time.strftime('%H:%M:%S', time.gmtime(len(frames)/rate)))
    size = int(rate/10)
    vars = []
    for ix in range(0, length, size):
        fb = frames[ix:ix+size]
        a = np.array(struct.unpack('B'*int(len(fb)), fb))
        vars.append(np.var(a))
    v = pd.Series(vars)
    vx = pd.Series(v[v<50].index)
    dxpos = [v[v>50].index[0]*size]
    dxpos.extend(vx[vx.diff()>200]*size)
    dxpos.append(length)    
    i = 0
    from scipy.signal import find_peaks
    s = dxpos[i]
    for ix in dxpos[i+1:]:
        fb = frames[s:ix]
        print(f'{i}.time:', time.strftime('%H:%M:%S', time.gmtime(s/rate)),'~', time.strftime('%H:%M:%S', time.gmtime(ix/rate)))
        height = 160
        if samplewidth == 1:
            a = np.array(struct.unpack('B'*int(len(fb)), fb))
        elif samplewidth == 2:
            a = np.array(struct.unpack('h'*int(len(fb)/2), fb))
        peak = find_peaks(a, height=height, distance=70)[0]
        tap = ''.join(['0' if a[p+40] < 110 else '1' for p in peak])
        if '001' in tap:
            tap = tap[tap.index('001')+2:]
        info = tapinfo(tap)
        if info is not None:
            i += 1
            file = f'{prefix}-{i:02d}.{info["name"].strip(" ")}.tap'
            print(file)
            open(file,'w').write(tap)
            s = ix
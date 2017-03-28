#!/usr/bin/python
# wavtest.py
#
# Author : Miso Kim (http://github.com/meesokim)
# Copyright (C) 2017
#
from __future__ import print_function

def normalized(a, axis=-1, order=2):
    l2 = np.atleast_1d(np.linalg.norm(a, order, axis))
    l2[l2==0] = 1
    return a / np.expand_dims(l2, axis)

if __name__ == '__main__':
    import wave
    import sys
    import pylab
    import numpy as np
    import struct
    if len(sys.argv) < 2:
        print("Usage: %s infile" % sys.argv[0])
        raise SystemExit(1)

    f = wave.open(sys.argv[1])
    channels = f.getnchannels()
    samples = f.getsampwidth()
    frames = f.getnframes()
    print ("channels =", samples)
    print ("sample width=", samples)
    print ("frames =", frames)
    nframes = 0
    if len(sys.argv) > 2:
        f.setpos(int(sys.argv[2]))
        nframes = int(sys.argv[2])/samples
    rev = 0
    st = np.zeros(100).astype(int)
    while True:   
        draw = False
        if rev > 0:
            f.setpos(f.tell()-rev)     
        rawdata = bytearray(f.readframes(800))
        nframes = nframes + len(rawdata)/samples
        if nframes > frames:
            break
        if channels == 2:
            del rawdata[2::4]
            del rawdata[2::3]
        msdata = np.array(struct.unpack('h'*(len(rawdata)/2), rawdata)) /100.0
        a = msdata[2:-1]-msdata[1:-2]*2-msdata[0:-3]
        b = np.where(np.abs(np.sign(a[1:-1])-np.sign(a[0:-2]))>1)[0]+1
        c = np.zeros(len(b)).astype(int)
        for i in range(1,len(b)):
           c[i] = np.argmax(np.absolute(msdata[b[i-1]:b[i]])) + b[i-1]
        m = normalized(msdata,0)[0]
        #print (m)
        
        d = c[2:-1:2]-c[1:-2:2]
        d2 = c[2:-1:2]-c[0:-3:2]
        d3 = d+d2
        e = m[c[2:-1:2]]-m[c[1:-2:2]]
        g = d>12
        for i in range(0,len(d)):
            g[i] = False if d2[i] < 27 and d[i] >12 else g[i]
        if any(d==13) or any(d2==14) or any(d2==15):
            draw = False
        if msdata[c[-2]] - msdata[c[-3]] < 0:
            endl = c[-2]
        else:
            endl = c[-3]
        rev = len(msdata)-endl
        for i in d3:
            if i < 100:
                st[i] = st[i] + 1
        #print (d)
        #print (e)
        s = ''.join(chr(i+ord('0')) for i in g*1)
        print (s,end='')
        #pylab.plot(m)
        if draw == True:
            _, ax = pylab.subplots(1, 1, figsize=(12, 6)) 
            #ax.plot(msdata)
            ax.plot(c, msdata[c])
            ind = c[2:-1:2]
            for i in range(0,len(ind)):
                ax.text(ind[i]-d[i]/2, 0, '%d' % g[i], ha='center', va='bottom')
                ax.text(ind[i]-d[i]/2, -50, '%d' % d[i], ha='center', va='bottom')
                ax.text(ind[i]-d2[i]/2, -100, '%d' % d2[i], ha='center', va='bottom')
            ax.plot((c[2:-1:2],c[1:-2:2]),(0,0),'k-')
            ax.plot((end,end),(100,-100),'c-')
            pylab.show()
    print (st)
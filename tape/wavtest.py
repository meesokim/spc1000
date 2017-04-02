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
    pos = 0
    if len(sys.argv) > 2:
        pos = int(sys.argv[2])
        if (pos > 0):
            f.setpos(int(pos))
            nframes = int(pos)/samples
        else:
            pos = -pos
    rev = 0
    st = np.zeros(100).astype(int)
    n = 0
    length = 0
    while True:   
        draw = False
        if rev > 0:
            f.setpos(f.tell()-rev)     
        rawdata = bytearray(f.readframes(1200))
        nframes = nframes + len(rawdata)/samples
        if nframes > frames:
            break
        msdata = np.array(struct.unpack('h'*(len(rawdata)/2), rawdata)) /100.0
        if channels == 2:
            lframes = msdata[::2]
            rframes = msdata[1::2]
            msdata = lframes/2 + rframes/2
        for i in range(0,len(msdata)):
            msdata[i] = 0 if abs(msdata[i]) < 2 else msdata[i]
        a = msdata[1:-1]-msdata[0:-2]
        r = np.average(np.abs(msdata))
        if r < 10.:
            continue
       # b = np.where(np.abs(np.sign(msdata[1:-1]-0.1)-np.sign(msdata[0:-2]+0.1))>1)[0]+1
        e = np.sign(msdata[1:-1]*msdata[0:-2])
        b = np.where(np.diff(np.signbit(msdata)))[0]
        c = np.zeros(len(b)+1).astype(int)
        k = 0
        for i in range(1,len(b)):
            if msdata[b[i-1]+1] - msdata[b[i-1]] > 0:
                c[k] = np.argmax(msdata[b[i-1]:b[i]]) + b[i-1]
            else:
                c[k] = np.argmin(msdata[b[i-1]:b[i]]) + b[i-1]
            k = k + 1
        msdata = msdata - ((np.max(msdata[c])+np.min(msdata[c]))/2)
        b = np.where(np.diff(np.signbit(msdata)))[0]
        b1 = np.zeros(len(b)+100).astype(int)
#        for i in range(0, len(b1)):
            
        c1 = c[1:-1]-c[0:-2]
        c2 = np.zeros(len(b)+100).astype(int)
        k = 0
        k = c[0]
        c2[0] = c[0]
        j = 1
        for i in range(1, len(c1)):
            if (msdata[c[i-1]]*msdata[c[i+1]] >=0 or abs(msdata[c[i]]) > 10) and c1[i-1] > 4 and c1[i-1] < 60:
                if (abs(msdata[c[i]]-msdata[c[i-1]])>30 and abs(msdata[c[i+1]]-msdata[c[i]])>30):
                    c2[j] = c[i]
                    j = j + 1
            elif (c1[i-1] == 4 and msdata[c[i]] * msdata[c[i-1]] < 0):
                c2[j] = c[i]
                j = j + 1
        c2[j] = c[len(c1)-1]
        c2 = np.resize(c2, j+1)
        c3 = c
        c = c2
        num = 0
        mc = np.diff(msdata[c])
        c1 = c[1:-1]-c[0:-2]
        for i in range(0, len(c)-4):
            if msdata[c[i+1]] - msdata[c[i]] > 30 and c1[i] > 4:
                if  (c[i] + (c[i+1] - c[i]) / 2 + 18 < len(msdata)):
                    b1[num] = c[i] + (c[i+1] - c[i]) / 2
                    num = num + 1
        b1 = np.resize(b1, num)
#        print (c, b1, len(msdata))
        if len(b1) > 0:
            g1 = msdata[b1+18] > 0
        #if any(b1<6):
        #print ("*",b[1:-1]-b[0:-2])
        #print ("*",e)
        #c = c2        
        #print (m)
        
        d = c[2:-1:2]-c[1:-2:2]
        d2 = c[2:-1:2]-c[0:-3:2]
        d3 = d+d2
        e = msdata[c[2:-1:2]]-msdata[c[1:-2:2]]
        g = d>11
        if len(c) > 3 and len(b1) > 0:
            for i in range(0,len(d)):
                g[i] = False if d2[i] < 30 and d[i] >10 else g[i]
                if d[i] == 11 and d2[i] > 30:
                    g[i] = True
            if any(d==13) or any(d2==14) or any(d2==15):
                draw = False
            if any(g) == True:
                draw = False
            if msdata[c[-2]] - msdata[c[-3]] < 0:
                endl = c[-3]
            else:
                endl = c[-4]
            if any(d<5):
                draw = False
            endl = b1[-1] + 5
            rev = len(msdata)-endl
            for i in d3:
                if i < 100:
                    st[i] = st[i] + 1
            #print (d)
            #print (e)
            s = ''.join(chr(i+ord('0')) for i in g1*1)
            if any(g1) == True:
                print (s,end='')
                length = length + len(g1)
                if pos > 0 and length > pos-200 or (pos == -1 and s == '1'*len(s)):
                    draw = True
                    #print (c1)
                    #print (' ')
                    #print (c3)
                    #print (c1)
            #else:
            #    print ('*',end='')
            sys.stdout.flush()
            if draw == True:
                #print (b[1:-1]-b[0:-2])
                _, ax = pylab.subplots(1, 1, figsize=(16, 6)) 
                ax.plot(msdata)
                #ax.plot(b, msdata[b], 'o')
                ax.plot(c, msdata[c], '*')
                ax.plot(c3, msdata[c3], '|')
                ax.plot(b1, msdata[b1], "*")
                ind = c[2:-1:2]
                for i in range(0,len(ind)):
                    if (len(g1)>i):
                        ax.text(ind[i]-d[i]/2, 30, '%d' % g1[i], ha='center', va='bottom')
                    ax.text(ind[i]-d[i]/2, 0, '%d' % g[i], ha='center', va='bottom')
                    ax.text(ind[i]-d[i]/2, -50, '%d' % d[i], ha='center', va='bottom')
                    ax.text(ind[i]-d2[i]/2, -100, '%d' % d2[i], ha='center', va='bottom')
                    ax.text(ind[i]-d2[i]/2, -150, '%d' % mc[i], ha='center', va='bottom')
                ax.plot((c[2:-1:2],c[1:-2:2]),(0,0),'k-')
                ax.plot((endl,endl),(100,-100),'c-')
                pylab.show()
    print (st)
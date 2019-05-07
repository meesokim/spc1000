#!/usr/bin/python
# kcs_decode.py
#
# Author : David Beazley (http://www.dabeaz.com)
# Copyright (C) 2010
#
# Requires Python 3

"""
Converts a WAV file containing Kansas City Standard data and
extracts text data from it. See:

http://en.wikipedia.org/wiki/Kansas_City_standard
"""

from __future__ import print_function
from collections import deque
from itertools import islice
from detect_peaks import detect_peaks
import struct
import numpy as np
import pylab
import matplotlib.pyplot as plt
import array
import sys

st = np.ndarray(100, dtype=int)
st[0:100] = 0

def hyst(x, th_lo, th_hi, initial = False):
    hi = x >= th_hi
    lo_or_hi = (x <= th_lo) | hi
    ind = np.nonzero(lo_or_hi)[0]
    if not ind.size: # prevent index error if ind is empty
        return np.zeros_like(x, dtype=bool) | initial
    cnt = np.cumsum(lo_or_hi) # from 0 to len(x)
    return np.where(cnt, hi[ind[cnt-1]], initial)

def zero_crossing(data):
    signs             = np.sign(data)
    signs[signs == 0] = 1
    signs = np.where(np.diff(signs))[0]
    for i in np.arange(len(signs)):
        try:
            if data[signs[i]-1] < 0 or data[signs[i]+1] >= 0:
                signs[i] = -1
            elif signs[i] - signs[i-1] > 0 and min(data[signs[i-1]:signs[i]]) >= 0 or (signs[i+1] - signs[i] + 1 > 0 and max(data[signs[i]+1:signs[i+1]]) <= 0):
                signs[i] = -1
        except:
            print
    a = np.where(signs==-1)
    signs = np.delete(signs, a)
    signs = np.delete(signs, np.where(signs[1:]-signs[:-1] < 14))
    return signs[0]
    
def zero_check(data):
    signs = np.sign(data)
    signs[signs == 0] = 1
    signs = np.where(np.diff(signs)==2)[0]
    return signs
    
# Generate a sequence representing sign bits
def generate_tap(wavefile):
    samplewidth = wavefile.getsampwidth()
    nchannels = wavefile.getnchannels()
    rate = wavefile.getframerate()
    previous = 0
    max = 0
    rev = 0
    det = 0
    msdata = np.zeros(1);
    dataheader = [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1]
    di = 0
    d = 0
    checkbit = 0
    dx = 0
    while True:
        if rev > 0:
            wavefile.setpos(wavefile.tell()-rev)
        pos = wavefile.tell()
        if pos + 50 > wavefile.getnframes():
            return
        fpos = wavefile.tell()
        frames = bytearray(wavefile.readframes(1500))
        if not frames or len(frames) == 0:
            break
        # Extract most significant bytes from left-most audio channel
#        if nchannels > 1:
#            frames = frames[0:][::2]
        #    del frames[samplewidth::1]    # Delete the left stereo channel    
        #    del frames[samplewidth::2]
        if samplewidth == 2:
            msdata = np.append(msdata,np.array(struct.unpack('h'*(int(len(frames)/2)), frames)) /100.0)
        else:
            msdata = np.append(msdata,(128 - np.array(struct.unpack('B'*(int(len(frames))), frames))) / 10.0)
        if nchannels > 1:
            msdata = msdata[1::2]
        x = np.where(msdata[1:-1] - msdata[0:-2]>=0,1,-1)
        size = len(x)
        x = np.zeros(size)
        y = np.zeros(size)
        mx = np.zeros(size)
        mn = np.zeros(size)
        df = np.zeros(size)
        p = 1
        ww = 10
        wx = 14
        for i in range(0, size-wx):
            mx[i] = np.max(msdata[i-wx if i > wx else 0:i+wx])
            mn[i] = np.min(msdata[i-wx if i > wx else 0:i+wx])
            df[i] = np.sign(msdata[i] - msdata[i-1])
        xk = -ww
        nk = -ww
        for i in range(0, len(y)):
            if mx[i] == msdata[i] and (xk + ww <= i  or (xk > 0 and xk + ww > i and msdata[xk] != msdata[i])):
                xk = i
                y[i] = 1
            elif mn[i] == msdata[i] and (nk + ww <= i or (nk > 0 and nk + ww > i and msdata[nk] != msdata[i])):
                nk = i
                y[i] = -1
        p = 1
        cnt = 0
        max = 0
        hw = np.max(msdata)
        #flg = np.where(y[13:-12]-y[12:-13]<-1)
        val = np.ndarray((100,2))        
        min = 20
        rev = 0
        p = y[0]
        error = 0
        p = np.sign (y[11])
        num = 0
        sum0 = 0
        i = 0
        k = 0
        idx = 0
        last = 0
        erridx = 0
        for i in range(0, size-20):
            if k > i:
                i = k
            if i < size-20 and y[i] > 0:
                j = i + 2
                while j < size-ww and y[j] != -1:
                    j = j + 1
                if y[j] != -1:
                    last = j
                    break
                cnt = cnt + 1
                sum0 = j - i
                #print (i,j,sum(msdata[i:j]), sum(np.abs(msdata[i:j])))
                if sum(np.abs(msdata[i:j]))/sum0 < 8:
                    #print ("rejected:[%d:%d],%d,%d %f" % (i,j,sum(msdata[i:j]), sum(np.abs(msdata[i:j])), sum(np.abs(msdata[i:j]))/sum0), file=sys.stderr)
                    checkbit = 0
                    di = 0
                    continue
                if j >= size:
                    break
                last = j + 1
                if y[j] < 0:
                    idx = int((j+i)/2)
                    if sum0 > 12:
                        x[idx] = 1
                        d = 1
                    else:
                        x[idx] = -1
                        d = 0
                else:
                    idx = int(i+(j-i)/4)
                    if sum0 > 22:
                        x[idx] = 1
                        d = 1
                    else:
                        x[idx] = -1
                        d = 0
                #print (',',i,j)
                k = j
                yield d
                if checkbit == 0:
                    if dataheader[di] == d:
                        di = di + 1
                        if di == 42:
                            checkbit = 1
                            dx = 0
                    else:
                        di = 0
                else:
                    dx = dx + 1
                    if dx % 9 == 0:
                        if d != 1:
                            error = 1
                            erridx = i
                            x[idx] = -2
                        else:
                            x[idx] = 2
#        print (st, min, rev)
        t = (fpos+erridx)
        if error == 1 and num < 4:
            _, ax = plt.subplots(1, 1, figsize=(12, 6))
            #ax = plt.plot(figsize=(12, 6))
            #ax.plot(x[0:last-1]*50, 'b', lw=1)
            ax.plot(y[0:last-1]*2, 'b', lw=2)
            ax.plot(msdata[0:last-1], 'k', lw=1)
            ax.plot(x[0:last-1]*20, 'r', lw=1)
            ax.set_title('%d %.2f %d:%.2f' % (t, t/float(rate), t/60/rate, ((t-(int(t/60/rate))*60*rate)/float(rate))))
            if num > 0:
                for x, v in val[0:num-1]:
                    ax.text(x-v/2, -50, '%d' % v, ha='center', va= 'bottom')
            plt.show()
            error = 0
#        else:
#            print ('%f %d:%f' % (t/float(rate), t/60/rate, ((t-(int(t/60/rate))*60*rate)/float(rate))))
            #plt.savefig("f%d.png" % (pos))
        if last > 0:
            last0 = last
            while last < len(y) and y[last] != -1:
                last = last + 1
            if last == len(y):
                last = last0
                while y[last] != -1:
                    last = last - 1
            msdata = msdata[last:]
        else:
            msdata = [];

if __name__ == '__main__':
    import wave
    import sys
    length = len(sys.argv)
    print ("args=%d" % length)
    if length < 2:
        print("Usage: %s infile" % sys.argv[0])
        raise SystemExit(1)

    wf = wave.open(sys.argv[1])
    if len(sys.argv) > 2 and sys.argv[2] != '':
        wf.setpos(int(sys.argv[2]))
    print ("sample width = ", wf.getsampwidth())
    print ("frame rates = ", wf.getframerate())
    print ("channels = ", wf.getnchannels())
    byte_stream = generate_tap(wf)
    # Output the byte stream in 80-byte chunks
    outf = sys.stdout
    #buffer = islice(byte_stream,80)
    #print ( sum(1 for _ in byte_stream))
    for c in byte_stream:
        if c == '':
            break
        print (c,end='')
        sys.stdout.flush()        

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
    while True:
        if rev > 0:
            wavefile.setpos(wavefile.tell()-rev)
        pos = wavefile.tell()
        if pos + 50 > wavefile.getnframes():
            return
        frames = bytearray(wavefile.readframes(1000))
        if not frames or len(frames) == 0:
            break
        # Extract most significant bytes from left-most audio channel
        if nchannels > 1:
            del frames[samplewidth::4]    # Delete the right stereo channel    
            del frames[samplewidth::3]
        if samplewidth == 2:
            msdata = np.append(msdata,np.array(struct.unpack('h'*(len(frames)/2), str(frames))) /100.0)
        else:
            msdata = np.append(msdata,(128 - np.array(struct.unpack('B'*(len(frames)), str(frames)))) / 10.0)
        x = np.where(msdata[1:-1] - msdata[0:-2]>=0,1,-1)
        size = len(x)
        x = np.zeros(size)
        y = np.zeros(size)
        mx = np.zeros(size)
        mn = np.zeros(size)
        df = np.zeros(size)
        p = 1
        for i in range(0, size):
            mx[i] = np.max(msdata[i-12 if i > 12 else 0:i+12])
            mn[i] = np.min(msdata[i-12 if i > 12 else 0:i+12])
            df[i] = np.sign(msdata[i] - msdata[i-1])
        for i in range(0, len(y)):
            if mx[i] == msdata[i]:
                y[i] = 1
            elif mn[i] == msdata[i]:
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
        for i in range(0, size-20):
            if k > i:
                i = k
            if i < size-20 and y[i] > 0:
                j = i + 3
                while j < size and y[j] >= 0:
                    j = j + 1
                cnt = cnt + 1
                sum0 = j - i
                if sum0 > 12:
                    x[(j+i)/2] = 1
                    yield 1
                else:
                    x[(j+i)/2] = -1
                    yield 0
                #print (',',i,j)
                k = j
        last = j + 1
#        print (st, min, rev)
        if error == 1 and num < 4:
            _, ax = plt.subplots(1, 1, figsize=(12, 6))
            #ax = plt.plot(figsize=(12, 6))
            #ax.plot(x[0:last-1]*50, 'b', lw=1)
            ax.plot(y[0:last-1]*2, 'b', lw=2)
            ax.plot(msdata[0:last-1], 'k', lw=1)
            ax.plot(x[0:last-1]*20, 'r', lw=1)
            ax.set_title('%d' % pos)
            if num > 0:
                for x, v in val[0:num-1]:
                    ax.text(x-v/2, -50, '%d' % v, ha='center', va= 'bottom')
            plt.show()
            #plt.savefig("f%d.png" % (pos)) 
        msdata = msdata[last:]

if __name__ == '__main__':
    import wave
    import sys
    if len(sys.argv) < 2:
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
    while True:
        #buffer = islice(byte_stream,80)
        #print ( sum(1 for _ in byte_stream))
        if not byte_stream:
            break
        for c in byte_stream:
            if c == '':
                break
            print (c,end='')
            sys.stdout.flush()        
    print (st)

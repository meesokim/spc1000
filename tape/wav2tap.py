#!/mingw32/bin/python
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
    while True:
        if rev > 0:
            wavefile.setpos(wavefile.tell()-rev)
        pos = wavefile.tell()
        frames = bytearray(wavefile.readframes(8000))
        if not frames:
            break
        # Extract most significant bytes from left-most audio channel
        if nchannels > 1:
            del frames[samplewidth::4]    # Delete the right stereo channel    
            del frames[samplewidth::3]
        if samplewidth == 2:    
            msdata = np.array(struct.unpack('h'*(len(frames)/2), frames)) /100.0
        else:
            msdata = (128 - np.array(struct.unpack('B'*(len(frames)), frames))) / 10.0
        x = np.where(msdata[1:-1] - msdata[0:-2]>=0,1,-1)
        y = np.zeros(len(x))
        mx = np.zeros(len(x))
        p = 1
        for i in range(12, len(x)-12):
            mx[i] = np.max(msdata[i-12:i+12]) if msdata[i] > 0 else np.min(msdata[i-12:i+12])
        for i in range(0, len(y)):
            if abs(mx[i]) > 50:
                if msdata[i] == mx[i]: 
                    if msdata[i] > 0:
                        p = -1
                    else:
                        p = 1                
                y[i] = p
            else:
                y[i] = 0
        p = -1 
        cnt = 0
        max = 0
        hw = np.max(msdata)
                
        min = 20
        rev = 0
        p = y[0]

        for i in range(0,len(x)):
            if y[i] > 0 and p < 0:
                rev = len(x) - i
                if cnt < min:
                    min = cnt
                #yield cnt
                if cnt < 100 and cnt > 15:
                    st[cnt] = st[cnt] + 1
                if cnt > max:
                    max = cnt
                cnt = 1
            else:
                cnt = cnt + 1
            p = y[i]
        if min < 12:
            _, ax = plt.subplots(1, 1, figsize=(16, 8))
            ax.plot(x*10, 'b', lw=1)
            ax.plot(y*20, 'r', lw=1)
            ax.plot(msdata, 'k', lw=1)
            ax.plot(mx, 'c', lw=1)
            plt.show()            
        

if __name__ == '__main__':
    import wave
    import sys
    if len(sys.argv) < 2:
        print("Usage: %s infile" % sys.argv[0])
        raise SystemExit(1)

    wf = wave.open(sys.argv[1])
    if len(sys.argv) > 2:
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
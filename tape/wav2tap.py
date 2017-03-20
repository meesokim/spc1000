#!/usr/bin/env python3
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
        frames = bytearray(wavefile.readframes(800))
        if not frames:
            break
        # Extract most significant bytes from left-most audio channel
        if nchannels > 1:
            del frames[samplewidth::4]    # Delete the right stereo channel    
            del frames[samplewidth::3]
        if samplewidth == 2:    
            msdata = np.array(struct.unpack('h'*(len(frames)/2), frames)) / 1000
        else:
            msdata = (128 - np.array(struct.unpack('B'*(len(frames)), frames))) / 10.0
        ind = zero_crossing(msdata)
        if len(ind) > 3:
			det = 0
			strs = np.chararray(len(ind))  
			for i in range(1,len(ind)-1):
				x = msdata[ind[i-1]:ind[i]]
				w = ind[i] - ind[i-1]
				area = np.sum(np.abs(x))
				mn = np.min(x)
				mx = np.max(x)
				r = np.argmin(msdata[ind[i]:ind[i+1]])+ind[i]
				l = np.argmin(msdata[ind[i-1]:ind[i]])+ind[i-1]
				lw = r - l
				if lw > 16 and lw < 31:
					s = '0'
				elif lw > 30 and lw < 56:
					s = '1'
				else:
					s = '*'
					det = 0
				if s == '*':
					print (s, w, area, mn, mx, lw, l+pos, r+pos, end='')
				else:
					print (s, end='')
				strs[i] = s
			rev = len(msdata) - ind[-3] + 5
			if det > 0:
				d = np.arange(len(msdata))
				_, ax = plt.subplots(1, 1, figsize=(8, 4))
				ax.plot(msdata, 'b', lw=1)
				ax.plot((l,l),(20,-20), 'k-', linestyle='--')
				ax.plot((r,r),(20,-20), 'k-', linestyle='--')
				ax.plot(ind[i], -20, )
				ax.set_ylim(-20,20)
				for i in range(1,len(ind)):
					if strs[i] == '1':
						ax.plot(ind[i], -10, '|')
					elif strs[i] == '0':
						ax.plot(ind[i], -10, '.')
					elif strs[i] == '*':
						ax.plot(ind[i], -10, '*')
					#print (strs[i])
				ax.set_title('output')
				plt.show()

if __name__ == '__main__':
    import wave
    import sys
    if len(sys.argv) == 1:
        print("Usage: %s infile" % sys.argv[0])
        raise SystemExit(1)

    wf = wave.open(sys.argv[1])
    print ("sample width = ", wf.getsampwidth())
    print ("frame rates = ", wf.getframerate())
    byte_stream = generate_tap(wf)
    # Output the byte stream in 80-byte chunks
    outf = sys.stdout
    while True:
        #buffer = islice(byte_stream,80)
        #print ( sum(1 for _ in byte_stream))
        if not byte_stream:
            break
        for c in byte_stream:
            print (c,end='')
        

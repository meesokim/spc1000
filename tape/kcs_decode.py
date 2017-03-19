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

# Generate a sequence representing sign bits
def generate_wav_sign_change_bits(wavefile):
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
        frames = bytearray(wavefile.readframes(4000))
        if not frames:
            break

        # Extract most significant bytes from left-most audio channel
        if nchannels > 1:
            del frames[samplewidth::4]    # Delete the right stereo channel    
            del frames[samplewidth::3]
            msdata = np.array(struct.unpack('h'*(len(frames)/2), frames)) / 1000
            minp = 15
            rate = 1
            mph = 1
            indexes = np.array(detect_peaks(msdata, mph=mph, mpd=minp))
            indexll = np.array(detect_peaks(msdata * -1, mph=mph, mpd=minp))
        else:
            msdata = (128 - np.array(struct.unpack('B'*(len(frames)), frames))) / 10.0
            minp = 2 if rate == 8000 else 15
            mph = 1.0
            rate = 5 if rate == 8000 else 1
            #print (45/rate, 25/rate, 35/rate)
            #print (45/rate, 25/rate, 35/rate)
            indexes = np.array(detect_peaks(msdata, mph=mph, mpd=minp))
            indexll = np.array(detect_peaks(msdata * -1, mph=mph, mpd=minp))
        #msbytes = bytearray(frames[samplewidth-1::samplewidth*nchannels])
            #print("index=",indexes[-1], len(msdata))
        #if rev > 0:
            #print ("rev=", rev)
        if len(indexes) > 1:
            rev = (len(msdata) - indexes[-3]) + 10/rate
            b = indexes[1:len(indexes)] - indexes[0:len(indexes)-1]
            str = []
            ch = np.zeros((len(b)))
            chs = ["" for x in range(len(b))]
            det = det + 1
            for i in range(1,len(b)-1):
                v = b[i]
                s = ''
                l = np.argmin(msdata[indexes[i-1]:indexes[i]]) + indexes[i-1]
                r = np.argmin(msdata[indexes[i]:indexes[i+1]]) + indexes[i]             
                a = np.sum(msdata[l:r] - msdata[l])
                #print (indexes[i-1:i+2], l,r,a)
                h = np.max(msdata[indexes[i]-1:indexes[i+1]])-np.min(msdata[indexes[i]:indexes[i+1]])
                ch[i] = indexes[i]
                if v >= 15/rate and v <= 50/rate:
                       #print (msdata[indexes[i]:indexes[i+1]], min(msdata[indexes[i]:indexes[i+1]]))
                    #if min(msdata[indexes[i]:indexes[i+1]]) < 0:
                    if v < 28/rate and r-l < 30:
                        s = '0'
                    elif v <= 35/rate:
                        if a > 60/rate:
                            s = '1'
                        elif a <= 60/rate:
                            s = '0'
                        else:
                            s = '?'
                        s = '?'
                        if r-l > 30:
                            s = '1'
                        else:
                            s = '0'
                    elif v > 35/rate:
                        s = '1'
                    else:
                        s = '*'
                        #det = det + 1
                        #print (v, a/v, h, s)
                    #if det > 0:
                    #    print (v, r-l, a/v, h, s)
                else:
                    s = '#'
                str.append(s)
                chs[i] = s
            if ''.join(str) != '0' * len(str):
                #print (str)
                #detect_peaks(msdata, mph=2000, mpd=15, show=True)
                #print (b)
                i = 0
                for s in str:
                    # Emit a stream of sign-change bits
                    #print ("%d>%c "%(b[i],s), end='')
                    i = i + 1
                    yield s
                if det > 1000:
                    #print ("rev=", rev)
                    np.array(detect_peaks(msdata, mph=mph, mpd=minp, edge='both', show=True, cut=(rev-b[-2]), check=ch, strs=chs))
                #det = det + 1
            else:
                yield '-'
            #rev = 0 if rev > 90 else rev
            #print("rev=", rev, wavefile.tell(), wavefile.tell()-rev+1200, len(msdata), indexes[-2])
            #print(msdata[0:rev])
            #print(msdata[len(msdata)-rev:len(msdata)-1])
        else:
            rev = 0
        

# Base frequency (representing a 1)
BASE_FREQ = 1200

# Generate a sequence of data bytes by sampling the stream of sign change bits
def generate_bytes(bitstream,framerate):
    bitmasks = [0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80]

    # Compute the number of audio frames used to encode a single data bit
    frames_per_bit = int(round(float(framerate)*8/BASE_FREQ))

    # Queue of sampled sign bits
    sample = deque(maxlen=frames_per_bit)     

    # Fill the sample buffer with an initial set of data
    sample.extend(islice(bitstream,frames_per_bit-1))
    sign_changes = sum(sample)

    # Look for the start bit
    for val in bitstream:
        if val:
            sign_changes += 1
        if sample.popleft():
            sign_changes -= 1
        sample.append(val)

        # If a start bit detected, sample the next 8 data bits
        if sign_changes <= 9:
            byteval = 0
            for mask in bitmasks:
                if sum(islice(bitstream,frames_per_bit)) >= 12:
                    byteval |= mask
            yield byteval
            # Skip the final two stop bits and refill the sample buffer 
            sample.extend(islice(bitstream,2*frames_per_bit,3*frames_per_bit-1))
            sign_changes = sum(sample)

if __name__ == '__main__':
    import wave
    import sys
    if len(sys.argv) != 2:
        print("Usage: %s infile" % sys.argv[0])
        raise SystemExit(1)

    wf = wave.open(sys.argv[1])
    print ("sample width = ", wf.getsampwidth())
    print ("frame rates = ", wf.getframerate())
    byte_stream = generate_wav_sign_change_bits(wf)
    #byte_stream  = generate_bytes(sign_changes, wf.getframerate())
    # Output the byte stream in 80-byte chunks
    outf = sys.stdout
    while True:
        #buffer = islice(byte_stream,80)
        #print ( sum(1 for _ in byte_stream))
        if not byte_stream:
            break
        for c in byte_stream:
            print (c,end='')
        

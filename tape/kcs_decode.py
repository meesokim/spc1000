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
    previous = 0
    max = 0
    rev = 0
    while True:
        if rev > 0:
            wavefile.setpos(wavefile.tell()-rev)
        frames = bytearray(wavefile.readframes(1024))
        if not frames:
            break

        # Extract most significant bytes from left-most audio channel
        if nchannels > 1:
            del frames[samplewidth::4]    # Delete the right stereo channel    
            del frames[samplewidth::3]
        msdata = np.array(struct.unpack('h'*(len(frames)/2), frames)) * -1
        #msbytes = bytearray(frames[samplewidth-1::samplewidth*nchannels])
        indexes = np.array(detect_peaks(msdata, mph=1000, mpd=15))
            #print("index=",indexes[-1], len(msdata))
        #if rev > 0:
            #print ("rev=", rev)
        if len(indexes) > 1:
            rev = (len(msdata) - indexes[-2]) + 5
            b = indexes[1:len(indexes)-1] - indexes[0:len(indexes)-2]
            str = []
            for i, v in enumerate(b):
                if v < 45:
                    #print (msdata[indexes[i]:indexes[i+1]], min(msdata[indexes[i]:indexes[i+1]]))
                    if min(msdata[indexes[i]:indexes[i+1]]) < 0:
                        if v < 25:
                            str.append('0')
                        elif v> 35 and v < 45:
                            str.append('1')
                        else:
                            pylab.plot(msdata[indexes[i]:indexes[i+1]])
            if ''.join(str) != '0' * len(str):
                #print (str)
                #detect_peaks(msdata, mph=2000, mpd=15, show=True)
                for s in str:
                    # Emit a stream of sign-change bits
                    yield s
            rev = 0 if rev > 90 else rev
            #print("rev-", rev)
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
    byte_stream = generate_wav_sign_change_bits(wf)
    #byte_stream  = generate_bytes(sign_changes, wf.getframerate())
    # Output the byte stream in 80-byte chunks
    outf = sys.stdout
    while True:
        #buffer = islice(byte_stream,80)
        if not byte_stream:
            break
        for c in byte_stream:
            print (c,end='')
        

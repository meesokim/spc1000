from __future__ import print_function
from collections import deque
from itertools import islice
import struct
import numpy as np
import pylab
import matplotlib.pyplot as plt
import array
import os


if __name__ == '__main__':
    import wave
    import sys
    import time
    if len(sys.argv) != 2:
        print("Usage: %s infile outfile" % sys.argv[0])
        raise SystemExit(1)

    wf = wave.open(sys.argv[1])
    filename, extension = os.path.splitext(sys.argv[1])
    #wf2 = wave.open(sys.argv[2], "w")
    (nchannels, sampwidth, framerate, nframes, comptype, compname) = wf.getparams ()
    print ("sample width = ", sampwidth)
    print ("frame rates = ", framerate)
    print ("channels = ", nchannels)
    print ("nframes = ", nframes)
    print ("play time = ", time.strftime('%H:%M:%S.', time.gmtime(nframes/framerate)) )
    #wf2.setparams((1, sampwidth, framerate, nframes, comptype, compname))
    nosound = 0
    frms = int(framerate / 10)
    starttime = -1
    splitno = 0
    wf2 = None
    rectime = 0
    while True:
        moment = wf.tell()
        ms = str((wf.tell() % framerate) * 10 / framerate)
        tt = time.strftime('%H:%M:%S.', time.gmtime(wf.tell()/framerate)) 
        tt0 = time.strftime('%H:%M:%S.', time.gmtime((wf.tell()-frms)/framerate))
        ms0 = str(((wf.tell()-frms) % framerate) * 10 / framerate)
        frames = bytearray(wf.readframes(frms))
        #print (tt)
        if len(frames) == 0:
            break
#        print (len(frames))
        # msdata = np.array(struct.unpack('h'*(len(frames)/2), frames))
        if nchannels > 1:
            lframes = msdata[::2]
            rframes = msdata[1::2]
            msdata = lframes/2 + rframes/2
        if sampwidth == 2:    
            msdata = np.array(struct.unpack('h'*int(len(frames)/2), frames))
        elif sampwidth == 1:
            msdata = (128 - np.array(struct.unpack('B'*(len(frames)), frames))) - 2            
        noise = int(np.std(msdata))
        #print (noise, ' ',end='')
        mx = len(np.where(msdata > 4))
        if noise < 0.05 * 256 ** sampwidth:
            if nosound == 0:
                starttime = wf.tell()
                print ("no sound from:", tt.join(' ' + ms), noise)
                nosound = 1
                if wf2 is not None:
                    wf2.writeframes(frames)
            else:
                noise0 = noise
        elif nosound == 1:
            d = (moment - starttime)*1000/framerate
            print ("no sound   to:", tt0.join(' ' + ms), noise0, str(d),  (moment - rectime)/framerate)
            if wf2 is not None and d  >= 500 and (moment - rectime)/framerate > 20:
                wf2.close()
                #print ("wf2.close()", d)
                wf2 = None

            nosound = 0
            if wf2 is None:
                splitno += 1
                print ("%s-%d.wav" % (filename,  splitno))
                rectime = moment
                wf2 = wave.open("%s-%d.wav" % (filename,  splitno), "w")
                wf2.setparams((nchannels, sampwidth, framerate, nframes, comptype, compname))
            if wf2 is not None:
                wf2.writeframes(frames)
        else: 
            noise0 = noise;
            if wf2 is not None:
                wf2.writeframes(frames)
        # print(nosound, noise, tt, end="\r")
        # print(noise,end=",")
     #       print ("sound:", tt.join(' ' + ms), noise, )
        #wf2.writeframes(struct.pack('h'*(len(monoframes)), *monoframes))
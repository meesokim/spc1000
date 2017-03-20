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
    #wf2.setparams((1, sampwidth, framerate, nframes, comptype, compname))
    nosound = 0
    frms = framerate / 10
    starttime = -1
    splitno = 0
    wf2 = None
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
        msdata = np.array(struct.unpack('h'*(len(frames)/2), frames))
        if nchannels > 1:
            lframes = msdata[::2]
            rframes = msdata[1::2]
            msdata = lframes/2 + rframes/2
        noise = int(np.average(np.abs(msdata)))
        mx = len(np.where(msdata > 16384 * 0.3))
        if noise < 16384 * 0.2 and mx < 15:
            if nosound == 0:
                starttime = wf.tell()
                print ("no sound from:", tt.join(' ' + ms), noise)
                nosound = 1
				#endtime = moment
                #splitno += 1
				#if starttime > -1:
                #	writewave(wf, starttime, endtime, "%s%d" % (filename,  splitno))
            else:
                noise0 = noise
        elif nosound == 1:
            print ("no sound   to:", tt0.join(' ' + ms), noise0, str((moment - starttime)*1000/framerate))
            if wf2 is not None and (moment - starttime)*1000/framerate >= 500 and (moment - rectime)/framerate > 20:
                wf2.close()
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
                
     #       print ("sound:", tt.join(' ' + ms), noise, )
        #wf2.writeframes(struct.pack('h'*(len(monoframes)), *monoframes))
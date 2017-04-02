from __future__ import print_function
from collections import deque
from itertools import islice
from detect_peaks import detect_peaks
import struct
import numpy as np
import pylab
import matplotlib.pyplot as plt
import array
import scipy.signal as sps


if __name__ == '__main__':
    import wave
    import sys
    if len(sys.argv) < 2 or sys.argv[1] == '':
        print("Usage: %s infile outfile" % sys.argv[0])
        raise SystemExit(1)

    wf = wave.open(sys.argv[1])
    wf2 = wave.open(sys.argv[2], "w")
    (nchannels, sampwidth, framerate, nframes, comptype, compname) = wf.getparams ()
    print ("sample width = ", sampwidth)
    print ("frame rates = ", framerate)
    print ("channels = ", nchannels)
    print ("nframes = ", nframes)
    if nchannels == 1 and framerate <= 8000:
        print ("no changes.")
        exit()
    div = 16000. / framerate
    wf2.setparams((1, 1, 16000, nframes, comptype, compname))
    while True:
        frames = bytearray(wf.readframes(32000))
        if len(frames) == 0:
            break
#        print (len(frames))
        if sampwidth == 2:
            msdata = np.array(struct.unpack('h'*(len(frames)/2), frames))
            #msdata = sps.resample(np.fromstring(frames, np.uint8, nroutsamples)
#            print (msdata[0:10])
            msdata = np.uint8((msdata/256.+128))
#            print (msdata[0:20])
        else:
            msdata = np.array(struct.unpack('c'*(len(frames)/2), frames))
            #msdata = sps.resample(np.fromstring(frames, np.uint8, nroutsamples)
        if nchannels == 2:
            lframes = msdata[::2]
            rframes = msdata[1::2]
            msdata = lframes/2 + rframes/2
        outdata = np.uint8(sps.resample(msdata, int(round(len(msdata) * div))))    
#        print (msdata[0:20])
        #        exit()
        wf2.writeframes(outdata.copy(order='C'))
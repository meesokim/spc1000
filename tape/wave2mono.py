from __future__ import print_function
from collections import deque
from itertools import islice
from detect_peaks import detect_peaks
import struct
import numpy as np
import pylab
import matplotlib.pyplot as plt
import array


if __name__ == '__main__':
    import wave
    import sys
    if len(sys.argv) != 3:
        print("Usage: %s infile outfile" % sys.argv[0])
        raise SystemExit(1)

    wf = wave.open(sys.argv[1])
    wf2 = wave.open(sys.argv[2], "w")
    (nchannels, sampwidth, framerate, nframes, comptype, compname) = wf.getparams ()
    print ("sample width = ", sampwidth)
    print ("frame rates = ", framerate)
    print ("channels = ", nchannels)
    print ("nframes = ", nframes)
    wf2.setparams((1, sampwidth, framerate, nframes, comptype, compname))
    while True:
        frames = bytearray(wf.readframes(32000))
        if len(frames) == 0:
            break
#        print (len(frames))
        msdata = np.array(struct.unpack('h'*(len(frames)/2), frames))
        lframes = msdata[::2]
        rframes = msdata[1::2]
        monoframes = lframes/2 + rframes/2
        wf2.writeframes(struct.pack('h'*(len(monoframes)), *monoframes))
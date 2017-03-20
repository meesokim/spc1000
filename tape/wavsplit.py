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
    import time
    if len(sys.argv) != 2:
        print("Usage: %s infile outfile" % sys.argv[0])
        raise SystemExit(1)

    wf = wave.open(sys.argv[1])
    #wf2 = wave.open(sys.argv[2], "w")
    (nchannels, sampwidth, framerate, nframes, comptype, compname) = wf.getparams ()
    print ("sample width = ", sampwidth)
    print ("frame rates = ", framerate)
    print ("channels = ", nchannels)
    print ("nframes = ", nframes)
    #wf2.setparams((1, sampwidth, framerate, nframes, comptype, compname))
    nosound = 0
    frms = framerate / 10
	starttime = 0
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
                print ("no sound from:", tt.join(' ' + ms), noise)
                nosound = 1
				endtime = moment
				if starttime > -1:
					writewave(wf, starttime, endtime, )
            else:
                noise0 = noise
        elif nosound == 1:
            print ("no sound   to:", tt0.join(' ' + ms), noise0)
            nosound = 0
        else: 
            noise0 = noise;
			if starttime != 0
     #       print ("sound:", tt.join(' ' + ms), noise, )
        #wf2.writeframes(struct.pack('h'*(len(monoframes)), *monoframes))
#!/mingw32/bin/python
# wavtest.py
#
# Author : Miso Kim (http://github.com/meesokim)
# Copyright (C) 2017
#

def normalized(a, axis=-1, order=2):
    l2 = np.atleast_1d(np.linalg.norm(a, order, axis))
    l2[l2==0] = 1
    return a / np.expand_dims(l2, axis)

if __name__ == '__main__':
    import wave
    import sys
    import pylab
    import numpy as np
    import struct
    if len(sys.argv) < 2:
        print("Usage: %s infile" % sys.argv[0])
        raise SystemExit(1)

    f = wave.open(sys.argv[1])
    channels = f.getnchannels()
    samples = f.getsampwidth()
    frames = f.getnframes()
    print ("channels =", samples)
    print ("sample width=", samples)
    print ("fames =", frames)
    if len(sys.argv) > 1:
        f.setpos(int(sys.argv[2]))
    rawdata = bytearray(f.readframes(400))
    if channels == 2:
        del rawdata[2::4]
        del rawdata[2::3]
    msdata = np.array(struct.unpack('h'*(len(rawdata)/2), rawdata)) /100.0
    a = msdata[2:-1]-msdata[1:-2]*2-msdata[0:-3]
    b = np.where(np.abs(np.sign(a[1:-1])-np.sign(a[0:-2]))>1)[0]+1
    c = np.zeros(len(b)).astype(int)
    for i in range(1,len(b)):
       c[i] = np.argmax(np.absolute(msdata[b[i-1]:b[i]])) + b[i-1]
    m = normalized(msdata,0)[0]
    #print (m)
    print (c[3:-1:2]-c[2:-2:2])
    print (msdata[c[3:-1:2]]-msdata[c[2:-2:2]])
    #pylab.plot(m)
    pylab.plot(c, m[c])
    pylab.show()

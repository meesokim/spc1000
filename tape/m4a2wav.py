#!/bin/python3
import os,sys, glob
from pydub import AudioSegment
import argparse
file = "*.m4a"
if len(sys.argv) > 1:
	file = sys.argv[1]
i = 0
for m4afile in glob.glob(file):
    i = i + 1
    print("file:%s" % m4afile)
    chunk = AudioSegment.from_file(m4afile, "m4a")
    chunk = chunk.set_channels(1)
    chunk.export("%s.wav"%(m4afile), format="wav")
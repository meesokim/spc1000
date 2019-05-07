import os,sys, glob
from pydub import AudioSegment
i = 0
for m4afile in glob.glob("c:\\Users\\meesokim\\Downloads\\*.m4a"):
    i = i + 1
    chunk = AudioSegment.from_file(m4afile, "m4a")
    chunk.export("%s_%d.wav"%(m4afile,i), format="wav")
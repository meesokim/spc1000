#!/bin/python3

import contextlib, datetime, math, os, time, wave, glob, sys
from scipy.io.wavfile import read
from numpy import log10, sqrt, mean
import numpy as np

def nextcheck(wavdata, i):
    p0 = 1
    while i < len(wavdata):
        if wavdata[i] < 0 and wavdata[i-1] >=0:
            return i
        i = i + 1
    return 0

path = "*.wav"
if len(sys.argv) > 1:
    path = sys.argv[1]
for fname in glob.glob(path):
    # print ("process_wav is processing - " + fname)
    outfname = fname + ".txt"
    mtime = datetime.datetime.fromtimestamp(os.path.getmtime(fname))
    tm=0.000000
    f = wave.open(fname,'r')
    channels = f.getnchannels()
    sampwidth = f.getsampwidth()
    comptype = f.getcomptype()
    frames = f.getnframes()
    rate = f.getframerate()
    wav_duration = frames / float(rate)
    frame_duration = round((wav_duration / frames),6)
    # print (mtime)
    hdr = (
    "# audio file processed - " + fname +
    "\n# this file name - " + outfname +    
    "\n# audio file mod time-" + str(mtime) +
    "\n# channels - " + str(channels) +
    "\n# sampwidth - " + str(sampwidth) +
    "\n# comptype - " + str(comptype) +
    "\n# frames - " + str(frames) +
    "\n# rate - " + str(rate) +
    "\n# wav_duration - " + str(wav_duration) +
    "\n# frame_dutation - " + "{0:0.6f}".format(frame_duration) +
    "\n# chunk|wave_file_name|audio_file_timestamp|chunk_second|chunk_microsecond|chunk_time_in_audio|frame_duration|dB\r\n"
    )
    samprate, wavdata = read(fname)
    out = open(outfname,'w')
    out.write(hdr)
    avg = np.average(wavdata)
    wavdata0 = wavdata + 32768
    lg = np.average(np.log10(wavdata0))
    # print (avg, lg)
    sample0 = -((np.sin(2*np.pi*np.arange(22)/22))*32768*0.55).astype(int)
    sample1 = -((np.sin(2*np.pi*np.arange(44)/44))*32768*0.65).astype(int)
    sample0 = [-1198, -3460, -4845, -6920, -9776, -11690, -11709, -10206, -8229, -6118, -3396, -7, 3196, 5700, 7311, 8479, 10173, 11902, 12412, 11101, 7913, 3300]
    sample1 = [-1350,-4833,-6173,-6904,-8542,-10988,-12815,-13067,-12092,-10711,-9543,-8612,-7911,-7158,-6686,-6637,-6623,-6449,-5842,-4500,-2482,188,3066,5820,8262,10534,12634,14816,16749,17713,17650,16665,14935,13373,12077,11218,10396,9456,8056,6334,3863,259]
    # print (sample0)
    # print (sample1)
    p0 = 1
    i = 0
    # cd = np.zeros(len(wavdata)-20)
    # for i in range(len(cd)):
        # cd[i] = 1 if wavdata[i+1] < 0 and wavdata[i] >= 0 else 0
    # print (sample0)
    # chks = cd.where(1)
    d0 = 'x'
    z0 = 0
    while i < len(wavdata)-1:
        try:
            # sec = int("{0:0.6f}".format(round(float(tm),6)).split('.')[0])
            # micsec = "{0:0.6f}".format(round(float(tm),6)).split('.')[1].lstrip('0')
            #handle 000000 - there has to be a better way
            # micsec = int(float(micsec)) if len(micsec)>=1 else 0
            # cm = mtime + datetime.timedelta(seconds=sec,microseconds=micsec)
            # out.write("{}|{}|{}|{}|{}|{}|{}|{}\n".format(i, fname, mtime, sec, micsec, cm, frame_duration, 20*log10( sqrt(mean(chunk**2)))))
            dat = wavdata[i]
            if dat < 0 and p0 >=0:
                n = nextcheck(wavdata, i+10) - i
                e0 = np.std(sample0 - wavdata[i:i+len(sample0)])
                e1 = np.std(sample1 - wavdata[i:i+len(sample1)])
                if d0 != 'x':
                    if e0 < e1 and n > 13 and n < 30:
                        d = '0'
                    elif n > 30 and n < 48:
                        d = '1'
                    else:
                        d = 'x'
                else:
                    if e0 * 1.3 < e1 and n < 30:
                        d = '0'
                    elif e0 > e1 * 1.3 and n > 30:
                        d = '1'
                # elif n > 13 and n < 28:
                    # d = '0'
                # elif n > 30 and n < 48:
                    # d = '1'
                    else:
                        d = 'x'                
                p0 = dat
                i = i + (n - 3)
                if d != '0' or z0 < 30:
                    print ("%c" % d, end='')
                if d0 != 'x' and d == 'x' and n < 48:
                    str = " %d %d %d" % (e0, e1, n)
                    print(str)
                # else:
                    # str = "%c" % d
                    # print(str, end='')
                #print ("%c %d %d" % (d, e0, e1))
                #out.write("\n%c %d %d\n" % (d, e0, e1))
                d0 = d
                if d == '0':
                    z0 = z0 + 1
                else:
                    z0 = 0
            else:
                i = i + 1
            p0 = dat
            dati = dat + 32768
            datf = dati/65536
            out.write("%d," % (dat))
            tm += frame_duration
        except ValueError as ex:
            print("value error:" + str(ex))
            out.write(str(i) + str(ex))
        except e:
            print("ERROR" + str(e))
            out.write(str(i) + str(e))
    out.close()
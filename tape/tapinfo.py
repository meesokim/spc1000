import os, sys
import re 

count = 0
uncount = 0
_surrogates = re.compile(r"[\uDC80-\uDCFF]")
def parity_check(data):
    paribits = 0
    onebits = 0
    for b in data[:-2]:
        if b[-1] == '1':
            paribits += 1
        onebits += sum([int(c) for c in b[:8]])        
    return paribits, onebits

def dump(bin):
    global count, uncount
    TYPE = ['Basic(0)','Machine(1)']
    MATCHED = ['not matched','=']
    tag_pos = bin.find('1'*40+'0'*40+'11')
    complete = 0
    if tag_pos > -1:
        headers = bin[tag_pos+82:][:130*9]
        sdata = []
        for i in range(0, len(headers), 9):
            sdata.append(headers[i:i+9])
        p, o = parity_check(sdata)
        idata = []
        for s in sdata:
            idata.append((int(s[:-1],2)))
        import struct
        HEADER = struct.unpack('b17sHHHb', bytes(idata[:25]))
        checksum = idata[-2]*256+idata[-1]
        length = HEADER[2]
        zeropos = 0
        for c in HEADER[1]:
            if c == 0:
                break
            zeropos += 1
        title = ''.join([chr(c) for c in HEADER[1][:zeropos] if chr(c) >= ' '])
        complete += int(p==len(sdata[:-2])) + int(o==checksum)
        print('parity: ', p==len(sdata[:-2]), 'checksum:', o==checksum)
        print('Name:', title)
        print('Type:', TYPE[HEADER[0]==1])
        print('Loading Address:', hex(HEADER[3]))
        print('Checksum:', checksum, MATCHED[o==checksum], o)
        print('Length:', length)
        if HEADER[0] == 1:
            print('Jump Address: ', hex(HEADER[4]))
        print('Protect:', HEADER[5])
        print(len(idata))
        bodies = bin[tag_pos+82+130*9:]
        btag_pos = bodies.find('1'*20+'0'*20+'11')
        if btag_pos > -1:
            errors = 0
            bodies = bodies[btag_pos+42:][:(length+4)*9]
            bin = bin[tag_pos+82+130*9+btag_pos+42+len(bodies):]
            if len(bodies) < (HEADER[2]+2)*9:
                print('length error', len(bodies), (HEADER[2]+2)*9)
            bdata = []
            idata = []
            onebits = 0
            for i in range(0,(length+2)*9,9):
                b = bodies[i:i+9]
                bdata.append(b)
                idata.append(int(b[:-1],2))
                if len(b) != 9:
                    print('length', b)
                if b[-1] != '1' and errors < 10:
                    print('parity', b, int(i/9))
                    errors += 1
            p, o = parity_check(bdata)
            checksum = idata[-2]*256+idata[-1]
            complete += int(p==len(bdata[:-2])) + int(o==checksum)
            # print(len(bdata[:-2]), bdata[-2:], idata[-2:], bodies[-9*4:])
            print('parity:', p==len(bdata[:-2]), 'checksum:', o == checksum)
            print('Length:', length)
            print(len(idata))
        else:
            bin = []
    else:
        bin = []
    if complete > 3:
        print('complete:', f,  title, hex(HEADER[3]), length)
        count += 1
    else:
        uncount += 1
    return bin
        
            
if __name__=='__main__':
    for f in sys.argv[1:]:
        tapbin = open(f,'r').read()
        pcount = count
        while True:
            try:
                tapbin = dump(tapbin)
            except:
                import traceback
                # traceback.print_exc()
                break
            if len(tapbin) < 100:
                break
        if pcount < count:
            print('FILE:', f)
    print('Total:', count)
    print('Abnormal:', uncount)
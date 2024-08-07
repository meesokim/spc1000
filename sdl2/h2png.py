import os, sys, numpy as np
from PIL import Image

def h2png(file):
    a = open(file).read()
    b = [int(b[-2:], 16) for b in ''.join([c for c in a[a.index('='):] if c in ',0123456789ABCDEFx']).split(',')]
    c = Image.new('RGB',size=(28*16,16*16))
    x0 = 0
    y0 = 0
    for ix, d in enumerate(b):
        i = ix % 32
        if ix < 32 * 161:
            x0 = (int(ix / 32) * 16) % (16 * 20)
            y0 = int(ix / 32 / 20) * 16
        elif ix < 32 * 248:
            x0 = (int((ix - 32 * 160) / 32) * 16) % (16 * 22)
            y0 = 8 * 16 + int((ix - 32 * 160) / 32 / 22) * 16
        else:
            x0 = (int((ix - 32 * 248) / 32) * 16) % (16 * 28)
            y0 = 12 * 16 + int((ix - 32 * 248) / 32 / 28) * 16
        x = x0 + i % 16
        for j in range(0, 8):
            y = y0 + j + (0 if i < 16 else 8)
            c.putpixel((x, y),(255,255,255) if ((d >> j)&1) else (0,0,0))

    c.save(f'{file}.png')

def png2h(file):
    c = Image.open(file)
    if not '.h' in file:
        GAP = [-1,-1,0]
    else:
        GAP = [0,0,0]
    bitmap = np.array(c.getchannel(0))
    unique, counts = np.unique(bitmap, return_counts=True)
    white = unique[0 if counts[0] < counts[1] else 1]
    # white = unique[0] if black == unique[1] else unique[1]
    x0 = 0
    y0 = 0
    carray = []
    b = []
    for ix in range(0, 360 * 32):
        d = 0
        i = ix % 32
        if ix < 32 * 161:
            x0 = (int(ix / 32) * 16) % (16 * 20) + GAP[0] * 16
            y0 = int(ix / 32 / 20) * 16
        elif ix < 32 * 248:
            x0 = (int((ix - 32 * 160) / 32) * 16) % (16 * 22) + GAP[1] * 16
            y0 = 8 * 16 + int((ix - 32 * 160) / 32 / 22) * 16
        else:
            x0 = (int((ix - 32 * 248) / 32) * 16) % (16 * 28) + GAP[2] * 16
            y0 = 12 * 16 + int((ix - 32 * 248) / 32 / 28) * 16
        if not ix % 32:
            if len(b):
                carray.append(b)
            b = []
        x = x0 + i % 16
        if x >= 0:
            for j in range(0, 8):
                y = y0 + j + (0 if i < 16 else 8)
                if bitmap[y][x] == white:
                    d += 1 << j
        b.append(d)
    carray.append(b)
    print()
    print('const uint8_t K_font[360][32] = {       ')
    for ix, b in enumerate(carray):
        print()
        print(f'  {{{','.join([f'0x{d:02X}' for d in b[:16]])},')
        print(f'   {','.join([f'0x{d:02X}' for d in b[16:]])}}}',end='')
        if ix + 1 == len(carray):
            break
        print(',',end='')
    print('};\n')


if __name__=='__main__':
    if len(sys.argv) > 1:
        file = sys.argv[1]
        if os.path.exists(file):
            if '.png' == file[-4:]:
                png2h(file)
            else:
                h2png(file)
import re
from PIL import Image
import pytesseract
import pandas as pd

'''
Usage:
--psm <number>

eg. --psm 6

Page segmentation modes:
  0    Orientation and script detection (OSD) only.
  1    Automatic page segmentation with OSD.
  2    Automatic page segmentation, but no OSD, or OCR.
  3    Fully automatic page segmentation, but no OSD. (Default)
  4    Assume a single column of text of variable sizes.
  5    Assume a single uniform block of vertically aligned text.
  6    Assume a single uniform block of text.
  7    Treat the image as a single text line.
  8    Treat the image as a single word.
  9    Treat the image as a single word in a circle.
 10    Treat the image as a single character.
 11    Sparse text. Find as much text as possible in no particular order.
 12    Sparse text with OSD.
 13    Raw line. Treat the image as a single text line,
                        bypassing hacks that are Tesseract-specific.
'''

#f = input('Enter file name: ')
import sys

if len(sys.argv) > 1:
    imgfiles = sys.argv[1:]
else:
    imgfiles = [f'{i}.png' for i in range(1,16)]

configs = ['--oem 3 --psm 4 -c tessedit_char_blacklist="@?" -c tessedit_char_whitelist="0123456789ABCDEF: "'] # '--psm 6 tessedit_char_whitelist=0123456789ABCDEF:']
concat_items = []

# common_pattern = input('Enter common pattern: ')
# len_item = 11
import cv2, numpy as np
from PIL import Image, ImageDraw, ImageFont
font = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', 20)  
for config in configs:
    print(f'Config: {config}')
    for f in imgfiles:
        img  = cv2.imread(f)
        # img  = cv2.cvtColor(img_cv, cv2.COLOR_BGR2RGB)
        img = cv2.resize(img, None, fx=2, fy=2, interpolation=cv2.INTER_CUBIC)

        img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

        kernel = np.ones((1,1), np.uint8)
        img = cv2.dilate(img, kernel, iterations=1)
        img = cv2.erode(img, kernel, iterations=1)

        img = cv2.threshold(cv2.medianBlur(img, 3), 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)[1]
        
        oimg = Image.fromarray(img).convert("RGB")
        
        text = pytesseract.image_to_string(img,config=config)
        boxs = pytesseract.image_to_boxes(img,config=config)
        draw = ImageDraw.Draw(oimg)
        w, h = oimg.size
        for no, box in enumerate([box.split(' ') for box in boxs.split('\n')]):
            if len(box) == 6:
                coord = [int(b) for b in box[1:-1]]
                y0 = h - coord[1]
                y1 = h - coord[3]
                coord[1] = y1
                coord[3] = y0
                draw.rectangle(coord, outline=(255,0,0,255))
                # print(box[0], coord[2:])
                draw.text((coord[0],y0), box[0],(0,0,255),font=font)
        oimg.save(f'{f.split(".")[0]}.ocr.png')
        # print(type(boxs))
        # print(boxs)
        print(text)

# concat_df = pd.DataFrame({'Entry':list(set(concat_items))})
# print(concat_df)

# writer = pd.ExcelWriter('file.xlsx')
# concat_df.to_excel(writer)
# writer.close()

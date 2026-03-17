#!/usr/bin/env python3
import sys
import struct

def convert_bmp_to_c(file_path):
    with open(file_path, 'rb') as f:
        # Read BMP header + info header (14 + 40 = 54 bytes)
        header = f.read(54)
        if header[:2] != b'BM':
            print(f"Error: {file_path} is not a valid BMP file.")
            return

        # Info Header starts at offset 14
        # width: +4 (4 bytes), height: +8 (4 bytes), planes: +12 (2 bytes), bpp: +14 (2 bytes)
        width, height, planes, bpp = struct.unpack('<iiHH', header[18:30])
        if bpp != 8:
            print(f"Error: Only 8-bit BMP is supported in this script. Found {bpp}-bit. width={width}, height={height}")
            return

        # Read palette (usually 256 colors for 8-bit BMP)
        # Palette starts after info header (offset 54)
        palette = []
        for _ in range(256):
            b, g, r, _ = f.read(4)
            palette.append((r, g, b))

        # Seek to pixel data (from header offset)
        pixel_offset = struct.unpack('<I', header[10:14])[0]
        f.seek(pixel_offset)

        # BMP rows are padded to 4 bytes
        abs_height = abs(height)
        row_size = (width + 3) & ~3
        pixels = []
        for _ in range(abs_height):
            row = f.read(row_size)
            pixels.append(row[:width])

        # BMP pixels are stored bottom-to-top by default (positive height)
        if height > 0:
            pixels.reverse()

        arr = []
        for row in pixels:
            for color_idx in row:
                r, g, b = palette[color_idx]
                # RGB888 to RGB565 (Little Endian in output)
                val = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
                arr.append(val)

        c_file_path = f"{file_path}.c"
        with open(c_file_path, 'w') as f_out:
            var_name = file_path.replace('.', '_').replace('/', '_')
            f_out.write(f"unsigned char {var_name}[{len(arr)*2}] = {{\n")
            for i, val in enumerate(arr):
                if i % 8 == 0:
                    f_out.write("\t")
                # Store as little-endian bytes (typical for Circle)
                f_out.write(f"0x{val & 0xFF:02x}, 0x{(val >> 8) & 0xFF:02x}, ")
                if (i + 1) % 8 == 0:
                    f_out.write("\n")
            if len(arr) % 8 != 0:
                f_out.write("\n")
            f_out.write("};\n")
        print(f"Successfully converted {file_path} to {c_file_path}")

if __name__ == '__main__':
    if len(sys.argv) > 1:
        convert_bmp_to_c(sys.argv[1])
    else:
        print("Usage: ./bmp2c.py file.bmp")

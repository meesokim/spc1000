import os, sys

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
        HEADER[1] = title
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

def parity_check(data):
    paribits = 0
    onebits = 0
    for b in data[:-2]:
        if b[-1] == '1':
            paribits += 1
        onebits += sum([int(c) for c in b[:8]])        
    return paribits, onebits

TOKEN = [
	( "'"         , 0, 0x3A ),
	( "*"         , 0, 0xFD ),
	( "-"         , 0, 0xF9 ),
	( "/"         , 0, 0xFC ),
	( "?"         , 0, 0x8C ),
	( "^"         , 0, 0xFE ),
	( "\\"        , 0, 0xFB ),
	( "+"         , 0, 0xF8 ),
	( "<>"        , 0, 0xF0 ),
	( "<="        , 0, 0xF3 ),
	( "<"         , 0, 0xF7 ),
	( "=<"        , 0, 0xF1 ),
	( "=>"        , 0, 0xF2 ),
	( "="         , 0, 0xF5 ),
	( "><"        , 0, 0xEF ),
	( ">="        , 0, 0xF4 ),
	( ">"         , 0, 0xF6 ),
	( "ABS"       , 1, 0x82 ),
	( "AND"       , 0, 0xED ),
	( "ASC"       , 1, 0xA9 ),
	( "ATN"       , 1, 0x8B ),
	( "AUTO"      , 0, 0x88 ),
	( "BEEP"      , 0, 0xA4 ),
	( "CALL"      , 0, 0xB9 ),
	( "CDBL"      , 1, 0x92 ),
	( "CGINIT"    , 0, 0xC9 ),
	( "CHR$"      , 1, 0xA0 ),
	( "CINT"      , 1, 0x94 ),
	( "CIRCLE"    , 0, 0xCB ),
	( "CLEAR"     , 0, 0x97 ),
	( "CLOSE"     , 0, 0xCF ),
	( "CLS"       , 0, 0x96 ),
	( "COLOR"     , 0, 0xC6 ),
	( "CONT"      , 0, 0x95 ),
	( "COS"       , 1, 0x84 ),
	( "CSNG"      , 1, 0x93 ),
	( "CSRLIN"    , 1, 0xC1 ),
	( "DATA"      , 0, 0x8F ),
	( "DEFDBL"    , 0, 0xA7 ),
	( "DEFINT"    , 0, 0xA5 ),
	( "DEFSNG"    , 0, 0xA6 ),
	( "DEFSTR"    , 0, 0xA8 ),
	( "DELETE"    , 0, 0x89 ),
	( "DEF"       , 0, 0xA9 ),
	( "DIM"       , 0, 0x91 ),
	( "DUMP"      , 0, 0xAA ),
	( "EDIT"      , 0, 0x93 ),
	( "ELSE"      , 0, 0xB8 ),
	( "END"       , 0, 0xA2 ),
	( "ERL"       , 1, 0xC0 ),
	( "ERROR"     , 0, 0xB5 ),
	( "ERR"       , 1, 0xBF ),
	( "EXP"       , 1, 0x87 ),
	( "FAC"       , 1, 0x9C ),
	( "FIX"       , 1, 0x8E ),
	( "FN"        , 1, 0xBD ),
	( "FOR"       , 0, 0x8A ),
	( "FRAC"      , 1, 0x8D ),
	( "FRE"       , 1, 0xB7 ),
	( "GOSUB"     , 0, 0x82 ),
	( "GOTO"      , 0, 0x81 ),
	( "HCOPY"     , 0, 0xBD ),
	( "HEX$"      , 1, 0xA2 ),
	( "IF"        , 0, 0x8E ),
	( "INKEY$"    , 1, 0xB5 ),
	( "INPUT"     , 0, 0x8D ),
	( "INP"       , 1, 0x91 ),
	( "INSTR"     , 1, 0xB6 ),
	( "INT"       , 1, 0x81 ),
	( "KEY"       , 0, 0xBE ),
	( "LABEL"     , 0, 0xC2 ),
	( "LEFT$"     , 1, 0xB2 ),
	( "LEN"       , 1, 0xAA ),
	( "LET"       , 0, 0x99 ),
	( "LINE"      , 0, 0xC5 ),
	( "LIST#1"    , 0, 0x87 ),
	( "LIST"      , 0, 0x86 ),
	( "LOAD"      , 0, 0xAB ),
	( "LOCATE"    , 0, 0xBA ),
	( "LOG"       , 1, 0x86 ),
	( "LPOS"      , 1, 0x9B ),
	( "MEN$"      , 1, 0xB8 ),
	( "MERGE"     , 0, 0xAD ),
	( "MID$"      , 1, 0xB4 ),
	( "MOD"       , 0, 0xFA ),
	( "MON"       , 0, 0xBC ),
	( "NEW"       , 0, 0x9A ),
	( "NEXT"      , 0, 0x8B ),
	( "NOT"       , 0, 0xEE ),
	( "OCT$"      , 1, 0xA3 ),
	( "OFF"       , 0, 0x9C ),
	( "ON"        , 0, 0x98 ),
	( "OR"        , 0, 0xEC ),
	( "OUT"       , 0, 0xAF ),
	( "PAINT"     , 0, 0xCC ),
	( "PAI"       , 1, 0x8F ),
	( "PAR"       , 1, 0xB0 ),
	( "PATTERN"   , 0, 0xCA ),
	( "PEEK"      , 1, 0x8A ),
	( "PLAY"      , 0, 0xA3 ),
	( "POKE"      , 0, 0x9B ),
	( "POP"       , 0, 0xC0 ),
	( "POS"       , 1, 0x9A ),
	( "PRESET"    , 0, 0xC8 ),
	( "PRINT"     , 0, 0x8C ),
	( "PSET"      , 0, 0xC7 ),
	( "PUSH"      , 0, 0xBF ),
	( "RAD"       , 1, 0x90 ),
	( "READ"      , 0, 0x90 ),
	( "REM"       , 0, 0x92 ),
	( "RENUM"     , 0, 0xB7 ),
	( "REPEAT"    , 0, 0x9F ),
	( "RESTORE"   , 0, 0x85 ),
	( "RESUME"    , 0, 0xB6 ),
	( "RETURN"    , 0, 0x84 ),
	( "RIGHT$"    , 1, 0xB3 ),
	( "RND"       , 1, 0x89 ),
	( "ROPEN"     , 0, 0xCD ),
	( "RUN"       , 0, 0x83 ),
	( "SAVE"      , 0, 0xAC ),
	( "SCREEN"    , 0, 0xC4 ),
	( "SCRN$"     , 1, 0xB9 ),
	( "SEARCH"    , 0, 0xB0 ),
	( "SGN"       , 1, 0x8C ),
	( "SIN"       , 1, 0x83 ),
	( "SOUND"     , 0, 0xB1 ),
	( "SPACE$"    , 1, 0xA7 ),
	( "SPC"       , 0, 0xE8 ),
	( "SQR"       , 1, 0x88 ),
	( "STEP"      , 0, 0xE0 ),
	( "STOP"      , 0, 0x94 ),
	( "STR$"      , 1, 0xA1 ),
	( "STRING$"   , 1, 0xBB ),
	( "SUM"       , 1, 0x9D ),
	( "SWAP"      , 0, 0xB3 ),
	( "TAB"       , 0, 0xE7 ),
	( "TAN"       , 1, 0x85 ),
	( "THEN"      , 0, 0xE1 ),
	( "TO"        , 0, 0xDF ),
	( "TRACE"     , 0, 0xA1 ),
	( "UNTIL"     , 0, 0xA0 ),
	( "USING"     , 0, 0xE2 ),
	( "USR"       , 1, 0xBE ),
	( "VAL"       , 1, 0xAB ),
	( "VARPTR"    , 1, 0xBA ),
	( "WEND"      , 0, 0x9E ),
	( "WHILE"     , 0, 0x9D ),
	( "WOPEN"     , 0, 0xCE ),
	( "XOR"       , 0, 0xEB )]

STATEMENT = [
    'GOTO', 'GOSUB', 'RUN', 'RETURN', 'RESTORE', 'LIST', 'LIST#1', 'AUTO', 
    'DELETE', 'FOR', 'NEXT', 'PRINT', 'INPUT', 'IF', 'DATA', 'READ',
    'DIM', 'REM', 'EDIT', 'STOP', 'CONT', 'CLS', 'CLEAR', 'ON',
    'LET', 'NEW', 'POKE', 'OFF', 'WHILE', 'WEND', 'REPEAT', 'UNTIL',
    'TRACE', 'END', 'PLAY', 'BEEP', 'DEFINT', 'DEFSNG', 'DEFTBL', 'DEFSTR',
    'DEF', 'DUMP', 'LOAD', 'SAVE', 'MERGE', '', 'OUT', 'SEARCH',
    'SOUND', '', 'SWAP', '', 'ERROR', 'RESUME', 'RENUM', 'ELSE', 
    'CALL', 'LOCATE', '', 'MON', 'HCOPY', 'KEY', 'PUSH', 'POP',
    '', 'LABEL', '', 'SCREEN', 'LINE', 'COLOR', 'PSET', 'PRESET',
    'CGINIT', 'PATTERN', 'CIRCLE', 'PAINT', 'ROPEN', 'WOPEN', 'CLOSE', '',
    '', '', '', '', '', '', '', '',
    '', '', '', '', '', '', 'TO', 'STEP',
    'THEN', 'USING', '', '', '', '', 'TAB', 'SPC',
    '', '', 'XOR', 'OR', 'AND', 'NOT', '><', '<>',
    '=<', '=>', '<=', '>=', '=', '>', '<', '+',
    '-', 'MOD', '\\', '/', '/', '*', '^', '']

STATEMENT2 = [
    'INT', 'ABS', 'SIN', 'COS', 'TAN', 'LOG', 'EXP', 'SQR', 
    'RND', 'PEEK', 'ATN', 'SGN', 'FRAC', 'FIX', 'PAI', 'RAD',
    'INP', 'CDBL', 'CSNG', 'CINT', '', '', '', '',
    '', 'POS', 'LPOS', 'FAC', 'SUM', '', '', 'CHR$', 
    'STR$', 'HEX$', 'OCT$', '', '', '', 'SPACE$', '',
    'ASC', 'LEN', 'VAL', '', '', '', '', 'PAR',
    '', 'LEFT$', 'RIGHT$', 'MID$', 'INKEY$', 'INSTR', 'FRE', 'MEN$',
    'SCRN$', 'VARPTR', 'STRING$', '', 'FN', 'USR', 'ERR', 'ERL', 'CSRLIN'
]

def load_tape(f):
    bin = open(f, 'r').read()
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
        HEADER = list(struct.unpack('b17sHHHH', bytes(idata[:26])))
        checksum = idata[-2]*256+idata[-1]
        length = HEADER[2]
        zeropos = 0
        for c in HEADER[1]:
            if c == 0:
                break
            zeropos += 1
        title = ''.join([chr(c) for c in HEADER[1][:zeropos] if chr(c) >= ' '])
        complete += int(p==len(sdata[:-2])) + int(o==checksum)
        basic = HEADER[0]==1
        loading_address = HEADER[3]
        jump_address = HEADER[4]
        HEADER.append(bytes(idata[25:]))
        # print('parity: ', p==len(sdata[:-2]), 'checksum:', o==checksum)
        # print('Name:', title)
        # print('Type:', TYPE[HEADER[0]==1])
        # print('Loading Address:', hex(HEADER[3]))
        # print('Checksum:', checksum, MATCHED[o==checksum], o)
        # print('Length:', length)
        # if HEADER[0] == 1:
        #     print('Jump Address: ', hex(HEADER[4]))
        # print('Protect:', HEADER[5])
        # print(len(idata))
        bodies = bin[tag_pos+82+130*9:]
        btag_pos = bodies.find('1'*20+'0'*20+'11')
        if btag_pos > -1:
            errors = 0
            bodies = bodies[btag_pos+42:][:(length+4)*9]
            bin = bin[tag_pos+82+130*9+btag_pos+42+len(bodies):]
            bdata = []
            idata = []
            onebits = 0
            for i in range(0,(length+2)*9,9):
                b = bodies[i:i+9]
                bdata.append(b)
                idata.append(int(b[:-1],2))
                # if len(b) != 9:
                #     print('length', b)
                if b[-1] != '1':
                    # print('parity', b, int(i/9))
                    errors += 1
            p, o = parity_check(bdata)
            checksum = idata[-2]*256+idata[-1]
            complete += int(p==len(bdata[:-2])) + int(o==checksum)
            # print(len(bdata[:-2]), bdata[-2:], idata[-2:], bodies[-9*4:])
            # print('parity:', p==len(bdata[:-2]), 'checksum:', o == checksum)
            # print('Length:', length)
            # print(len(idata))
        else:
            bin = []
    else:
        bin = []
    # if complete > 3:
    #     print('complete:', f,  title, hex(HEADER[3]), length)
    #     count += 1
    # else:
    #     uncount += 1
    return HEADER, idata

def short(b):
    return b[0] + b[1] * 256

import numpy as np, struct
def bas2txt(body, start_addr=0x7c9d):
    pos = 0
    codes = []
    quote = False
    while pos < len(body):
        code = []
        next_line = short(body[pos:pos+2]) - start_addr
        line_number = short(body[pos+2:pos+4])
        pos += 4
        code.append(line_number)
        code.append(' ')
        token = 1
        quote = False
        while token and pos < len(body):
            token = body[pos]
            pos += 1
            if token > 0 and token < 0xb:
                num = token - 1
                # print(str(num))
                code.append(str(num))
            elif token == 0xb:
                line_number = short(body[pos:pos+2])
                code.append(str(line_number))
                pos += 2
            elif token == 0xc:
                line_address = short(body[pos:pos+2]) - start_addr
                # code.append(f'ADDR:{line_address}')
                pos += 2
            elif token == 0x10:
                code.append(oct(short(body[pos:pos+2])).replace('0o','&O'))
                pos += 2
            elif token == 0x11:
                code.append(hex(short(body[pos:pos+2])).replace('0x','&H').upper())
                pos += 2
            elif token == 0x12:
                code.append(str(struct.unpack('h', bytes(body[pos:pos+2]))[0]))
                pos += 2
            elif token == 0x14:
                code.append(str(np.array(body[pos:pos+4], dtype=float)))
                pos += 4
            elif token == 0x18:
                code.append(str(np.array(body[pos:pos+8], dtype=float64)))
                pos += 8
            elif (token >= 0x20 and token <= 0x7f):
                code.append(chr(token))
                if token == 0x27 or chr(token) == '"': 
                    quote = not quote
            elif quote:
                # code.append(chr(0x2700 + token))
                code.append(chr(token))
            elif token >= 0x81 and token < 0xff:
                code.append(STATEMENT[token-0x81])
            elif token == 0xff:
                token = body[pos]
                code.append(STATEMENT2[token-0x81])
                pos += 1
                # print(body[pos:pos+3])
        codes.append(''.join([str(a) if type(a) != str else a for a in code]))
    f = open('source.txt', 'wb')
    for code in codes:
        f.write(bytes([ord(c) for c in code]))
        f.write(b'\n')
    # print('\n'.join(codes).encode('utf-8'))
    # print('\n'.join(codes))
    
if __name__=='__main__':
    args = sys.argv[1:]
    for f in args:
        if not os.path.exists(f):
            continue
        header, body = load_tape(f)
        if header[0] == 2:
            bas2txt(body, header[3])
            break

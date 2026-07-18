#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

typedef unsigned char byte;
typedef unsigned short word;

#include "Z80.h"
#include "common.h"
#include "tape_loader.h"

// Define the global instances declared in common.h
SPCSystem spcsys;
SPCConfig spconf;
SPCSimul spcsim;

extern "C" char tap0[];
extern "C" unsigned char ROM[32768];

static TapeLoaderConfig tapeCfg;

static unsigned int batch_start_cycles = 0;
static int batch_start_icount = 0;
static unsigned int spc_cycles = 0;

static unsigned int GetCycles(void)
{
    return batch_start_cycles + (batch_start_icount - spcsys.Z80R.ICount);
}

static int tapeLen = 0;
static int tapePos = 0;
static int consecutiveZeros = 0;
static unsigned int casLastTime = 0;
static int casReadVal = 0;
static unsigned int casBitEndTime = 0;
static unsigned int casBitInvTime = 0;
static bool sync_found = false;
static int nextBlockDataPos = 0;
static int inject_bits_left = 0;
static const char *inject_stream = nullptr;

static int ReadTapeBit(void)
{
    if (tapeLen == 0)
    {
        int len = 0;
        while (tap0[len]) len++;
        tapeLen = len;
    }

    if (tapePos >= tapeLen)
    {
        return 0;
    }

    int c;
    int zero_skip = tapeCfg.zero_skip;
    if (zero_skip < 1) zero_skip = 1;

    if (consecutiveZeros > zero_skip)
    {
        while (tapePos < tapeLen && tap0[tapePos] == '0')
        {
            tapePos++;
        }
        if (tapePos < tapeLen)
        {
            c = (tap0[tapePos++] == '1' ? 1 : 0);
        }
        else
        {
            c = 0;
        }
        consecutiveZeros = 0;
    }
    else
    {
        c = (tap0[tapePos++] == '1' ? 1 : 0);
    }

    if (c == 0)
    {
        consecutiveZeros++;
    }
    else
    {
        consecutiveZeros = 0;
    }

    return c;
}

struct DecodedBlock {
    int sync_pos;
    std::vector<unsigned char> data;
};

static std::vector<DecodedBlock> decoded_blocks;
static int current_block_idx = -1;
static int current_byte_idx = -1;
static int current_bit_idx = -1;
static int virtual_cksm = 0;

static void DecodeTape() {
    int len = 0;
    while (tap0[len]) len++;
    tapeLen = len;

    decoded_blocks.clear();
    int scan_pos = 0;
    while (scan_pos + 8 < tapeLen) {
        if (tap0[scan_pos] == '1' && tap0[scan_pos+1] == '1' &&
            tap0[scan_pos+2] == '0' && tap0[scan_pos+3] == '0' &&
            tap0[scan_pos+4] == '0' && tap0[scan_pos+5] == '0' &&
            tap0[scan_pos+6] == '0' && tap0[scan_pos+7] == '0') 
        {
            int zero_count = 0;
            for (int i = 1; i <= 20; i++) {
                if (scan_pos - i >= 0 && tap0[scan_pos - i] == '0')
                    zero_count++;
            }
            if (zero_count >= 15 || scan_pos == 113) {
                DecodedBlock block;
                block.sync_pos = scan_pos;
                decoded_blocks.push_back(block);
                scan_pos += 8;
                continue;
            }
        }
        scan_pos++;
    }

    for (size_t i = 0; i < decoded_blocks.size(); i++) {
        int start_idx = decoded_blocks[i].sync_pos + 11;
        int end_idx = (i + 1 < decoded_blocks.size()) ? decoded_blocks[i+1].sync_pos : tapeLen;
        int bits_len = end_idx - start_idx;
        int num_bytes = bits_len / 9;
        
        for (int b = 0; b < num_bytes; b++) {
            int byte_idx = start_idx + b * 9;
            unsigned char val = 0;
            for (int bit = 0; bit < 8; bit++) {
                if (tap0[byte_idx + bit] == '1') {
                    val |= (1 << (7 - bit));
                }
            }
            decoded_blocks[i].data.push_back(val);
        }
    }

    if (decoded_blocks.size() > 0 && decoded_blocks[0].data.size() > 0) {
        if (decoded_blocks[0].data[0] == 0x61) { // 'a'
            decoded_blocks[0].data.insert(decoded_blocks[0].data.begin(), 0x00);
            if (decoded_blocks[0].data.size() > 130) {
                decoded_blocks[0].data.resize(130);
            }
        }
        printf("Decoded blocks[0].data: ");
        for (size_t k = 0; k < 20 && k < decoded_blocks[0].data.size(); k++) {
            printf("%02X ", decoded_blocks[0].data[k]);
        }
        printf("\n");
    }
}

static int CasRead(void)
{
    unsigned int cycles = GetCycles();
    unsigned short SP = spcsys.Z80R.SP.W;
    unsigned short ret_addr = spcsys.RAM[SP] | (spcsys.RAM[(SP + 1) & 0xFFFF] << 8);
    bool in_mkrd = (ret_addr >= 0x028B && ret_addr < 0x02E0);

    if (in_mkrd)
    {
        if (!sync_found)
        {
            // Scan for configured sync pattern starting from tapePos
            const char *pattern = tapeCfg.sync_pattern;
            int pat_len = strlen(pattern);
            int scan_pos = tapePos;
            while (scan_pos + pat_len <= tapeLen)
            {
                bool match = true;
                for (int k = 0; k < pat_len; k++)
                {
                    if (tap0[scan_pos + k] != pattern[k])
                    {
                        match = false;
                        break;
                    }
                }
                if (match)
                {
                    int zero_count = 0;
                    for (int i = 1; i <= 20; i++)
                    {
                        if (scan_pos - i >= 0 && tap0[scan_pos - i] == '0')
                            zero_count++;
                    }
                    if (zero_count >= tapeCfg.precursor_zeros)
                    {
                        nextBlockDataPos = scan_pos + tapeCfg.start_offset;
                        sync_found = true;
                        break;
                    }
                }
                scan_pos++;
            }
            if (!sync_found)
            {
                nextBlockDataPos = tapePos;
                sync_found = true;
            }
        }

        if (ret_addr == 0x02AA) // MKRD5
            return 1;
        else if (ret_addr == 0x02BA) // MKRD2
            return 0;
        else
            return 1;
    }
    else
    {
        static bool injected = false;
        static int inject_bits_left = 0;
        const char* inject_stream = "000000100"; // 0x02 + stop bit 0

        if (sync_found)
        {
            tapePos = nextBlockDataPos;
            sync_found = false;
        }

        if (inject_bits_left == 0)
        {
            for (int i = 0; i < tapeCfg.injection_count && i < MAX_INJECTIONS; i++)
            {
                if (tapePos == tapeCfg.injection_pos[i] && !tapeCfg.injection_done[i])
                {
                    inject_bits_left = 9;
                    inject_stream = tapeCfg.injection_bits[i];
                    tapeCfg.injection_done[i] = true;
                    break;
                }
            }
        }

        int ret = 0;
        if (inject_bits_left > 0 && inject_stream != nullptr)
        {
            ret = (inject_stream[9 - inject_bits_left] == '1' ? 1 : 0);
            inject_bits_left--;
        }
        else
        {
            if (tapePos < tapeLen)
            {
                ret = (tap0[tapePos] == '1' ? 1 : 0);
            }
            ReadTapeBit(); // Advance tapePos
        }
        return ret;
    }
}

extern "C" {


byte InZ80(word Port)
{
    if (Port >= 0x8000 && Port <= 0x8009)
    {
        return spcsys.keyMatrix[Port - 0x8000];
    }
    else if ((Port & 0xFFFE) == 0x4000)
    {
        if (Port & 0x01)
        {
            if (spcsys.psgRegNum == 14)
            {
                byte r = 0xff;
                if (spcsys.cas.button == 1 && spcsys.cas.motor)
                {
                    if (CasRead() == 1) r |= 0x80;
                    else r &= 0x7f;
                    r &= ~0x40; // Motor On (0)
                }
                else
                {
                    r |= 0x40;  // Motor Off (1)
                }
                return r;
            }
        }
        return 0x1f;
    }
    else if (Port == 0x4003)
    {
        return (tapeLen > 0 ? 1 : 0);
    }
    else if (Port == 0x4004)
    {
        byte retval = 0;
        for (int i = 0; i < 8; i++)
        {
            if (ReadTapeBit())
                retval |= (1 << (7 - i));
        }
        ReadTapeBit(); // Skip stop bit
        return retval;
    }
    return 0xff;
}

void OutZ80(word Port, byte Value)
{
    if (Port < 0x2000)
        spcsys.VRAM[Port] = Value;
    else if ((Port & 0xE000) == 0xA000)
        spcsys.IPLK = spcsys.IPLK ? 0 : 1;
    else if ((Port & 0xE000) == 0x2000)
        spcsys.GMODE = Value;
    else if ((Port & 0xE000) == 0x6000)
    {
        if (spcsys.cas.button != 0)
        {
            if (Value & 0x02)
            {
                if (spcsys.cas.pulse == 0) spcsys.cas.pulse = 1;
            }
            else
            {
                if (spcsys.cas.pulse)
                {
                    spcsys.cas.pulse = 0;
                    spcsys.cas.motor = !spcsys.cas.motor;
                    if (spcsys.cas.motor)
                    {
                        casLastTime = GetCycles();
                        tapePos = 0;
                        consecutiveZeros = 0;
                        casReadVal = 0;
                    }
                }
            }
        }
    }
    else if (Port == 0x4003)
    {
        if (Value == 0)
        {
            tapePos = 0;
            consecutiveZeros = 0;
            casLastTime = GetCycles();
            casReadVal = 0;
            spcsys.cas.button = 1;
            spcsys.cas.motor = 1;
        }
    }
    else if ((Port & 0xFFFE) == 0x4000)
    {
        if (!(Port & 0x01))
            spcsys.psgRegNum = Value & 0x1f;
    }
}

void PatchZ80(Z80 *R) {}
word LoopZ80(Z80 *R)
{
    static int count = 0;
    if (count++ < 10)
    {
        printf("LoopZ80 called: ICount=%d, PC=0x%04X\n", R->ICount, R->PC.W);
    }
    return INT_NONE;
}

} // extern "C"

int main()
{
    // Load ROM
    FILE *fp = fopen("spcall.rom", "rb");
    if (!fp)
    {
        printf("Error opening spcall.rom\n");
        return 1;
    }
    fread(spcsys.ROM, 1, 32768, fp);
    fclose(fp);
    printf("Loaded ROM successfully.\n");

    // Initialize SpcSystem state
    memcpy(spcsys.RAM, spcsys.ROM, 32768);
    spcsys.IPLK = 0;
    spcsys.cas.button = 1; // CAS_PLAY
    spcsys.cas.motor = 1; // Starts ON
    memset(spcsys.keyMatrix, 0xff, 10);
    DecodeTape();
    TapeLoaderConfig_InitDefaults(&tapeCfg);

    // Initialize Z80
    Z80 *R = &spcsys.Z80R;
    ResetZ80(R);

    // Load tape loader configuration defaults
    TapeLoaderConfig_InitDefaults(&tapeCfg);

    // Force tape entry point
    // We set SP to 0xF000
    // And push 0xFFFF as return address so it RETs to 0xFFFF when done
    R->SP.W = 0xF000;
    spcsys.RAM[R->SP.W] = 0xFF;
    spcsys.RAM[R->SP.W+1] = 0xFF;

    // Set HL, BC, DE for CLOAD inputs
    R->HL.W = 0x4000;
    R->BC.W = 0x4000;
    R->DE.W = 0x4000;
    
    // Jump directly to FLOAD (0x0110)
    R->PC.W = 0x0110;

    printf("Starting Z80 execution at PC=0x%04X, SP=0x%04X\n", R->PC.W, R->SP.W);

    struct TraceEntry {
        unsigned long long step;
        unsigned int pc;
        unsigned int sp;
        unsigned int af;
        unsigned int bc;
        unsigned int de;
        unsigned int hl;
        unsigned char op[3];
        unsigned int cycles;
    };
    const int TRACE_SIZE = 150;
    TraceEntry trace_buffer[TRACE_SIZE];
    int trace_head = 0;
    bool trace_wrapped = false;

    unsigned long long total_instructions = 0;
    unsigned int prev_pc = 0;
    int loop_detect_count = 0;

    while (R->PC.W != 0xFFFF)
    {
        int count = 10000; // Batch execution for speed
        R->ICount = count;
        batch_start_cycles = spc_cycles;
        batch_start_icount = R->ICount;

        unsigned int current_pc = R->PC.W;
        static byte last_val = 0;
        if (spcsys.RAM[0x1396] != last_val)
        {
            printf("RAM[0x1396] changed from 0x%02X to 0x%02X at PC=0x%04X\n", 
                   last_val, spcsys.RAM[0x1396], current_pc);
            last_val = spcsys.RAM[0x1396];
        }
        if (current_pc == 0x0172)
        {
            printf("LD (HL), A executed: HL=0x%04X, A=0x%02X\n", 
                   R->HL.W, R->AF.B.h);
        }
        ExecZ80(R);
        spc_cycles += (count - R->ICount);

        total_instructions += count;

        if (R->PC.W == prev_pc)
        {
            loop_detect_count++;
            if (loop_detect_count > 10000)
            {
                printf("Loop detected: Z80 is stuck at PC=0x%04X (cycles=%u, tapePos=%d)\n", R->PC.W, spc_cycles, tapePos);
                break;
            }
        }
        else
        {
            prev_pc = R->PC.W;
            loop_detect_count = 0;
        }

        // Safeguard to prevent infinite loops
        if (spc_cycles > 100000000) // 100 million cycles
        {
            printf("Safeguard timeout reached at cycles=%u. Z80 is at PC=0x%04X\n", spc_cycles, R->PC.W);
            break;
        }
    }

    // Dump circular buffer on failure
    if (R->PC.W != 0xFFFF)
    {
        printf("\n--- Trace leading up to failure ---\n");
        int start = trace_wrapped ? trace_head : 0;
        int count_to_print = trace_wrapped ? TRACE_SIZE : trace_head;
        for (int i = 0; i < count_to_print; i++)
        {
            int idx = (start + i) % TRACE_SIZE;
            printf("Step %3lld: PC=0x%04X SP=0x%04X AF=0x%04X BC=0x%04X DE=0x%04X HL=0x%04X Opcode: 0x%02X 0x%02X 0x%02X cycles=%u\n",
                   trace_buffer[idx].step,
                   trace_buffer[idx].pc,
                   trace_buffer[idx].sp,
                   trace_buffer[idx].af,
                   trace_buffer[idx].bc,
                   trace_buffer[idx].de,
                   trace_buffer[idx].hl,
                   trace_buffer[idx].op[0], trace_buffer[idx].op[1], trace_buffer[idx].op[2],
                   trace_buffer[idx].cycles);
        }
    }

    if (R->PC.W == 0xFFFF)
    {
        printf("Success! Z80 successfully loaded tape and returned to 0xFFFF at cycles=%u, tapePos=%d\n", spc_cycles, tapePos);
        printf("RAM dump at 0x1396 (FILMOD):\n");
        for (int i = 0; i < 64; i++) {
            printf("%02X ", spcsys.RAM[0x1396 + i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("RAM dump at 0x5196:\n");
        for (int i = 0; i < 64; i++) {
            printf("%02X ", spcsys.RAM[0x5196 + i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("Searching for sequence 0x58, 0xDC, 0x5C, 0x5B in RAM:\n");
        for (int i = 0; i < 65532; i++) {
            if (spcsys.RAM[i] == 0x58 && spcsys.RAM[i+1] == 0xDC && spcsys.RAM[i+2] == 0x5C && spcsys.RAM[i+3] == 0x5B) {
                printf("Sequence found at RAM[0x%04X]\n", i);
            }
        }
    }
    else
    {
        printf("Fail. Final state: PC=0x%04X, SP=0x%04X, cycles=%u, tapePos=%d\n", R->PC.W, R->SP.W, spc_cycles, tapePos);
    }

    return 0;
}

#include "cassette.h"
#include <fatfs/ff.h>
#ifdef HOST_COMPILE
#include <stdio.h>
#endif

extern "C" {
    #include <bzlib.h>
    #include <miniz_zip.h>
}

static void lower(char *out, const char *in) {
    while (*in) {
        char c = *in;
        if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
        *out++ = c;
        in++;
    }
    *out = '\0';
}

static bool is_supported_extension(const char *ext) {
    char ext_lower[16];
    lower(ext_lower, ext);
    return (strcmp(ext_lower, ".tap") == 0 ||
            strcmp(ext_lower, ".cas") == 0 ||
            strcmp(ext_lower, ".zip") == 0 ||
            strcmp(ext_lower, ".bz2") == 0);
}

static int compare_files(const ZFile &f1, const ZFile &f2) {
    const char *s1 = f1.c_str();
    const char *s2 = f2.c_str();
    while (*s1 && *s2) {
        char c1 = *s1;
        char c2 = *s2;
        if (c1 >= 'A' && c1 <= 'Z') c1 = c1 - 'A' + 'a';
        if (c2 >= 'A' && c2 <= 'Z') c2 = c2 - 'A' + 'a';
        if (c1 != c2) return (unsigned char)c1 - (unsigned char)c2;
        s1++;
        s2++;
    }
    char c1 = *s1;
    char c2 = *s2;
    if (c1 >= 'A' && c1 <= 'Z') c1 = c1 - 'A' + 'a';
    if (c2 >= 'A' && c2 <= 'Z') c2 = c2 - 'A' + 'a';
    return (unsigned char)c1 - (unsigned char)c2;
}

static void sort_files(ZFile *files, unsigned int size) {
    for (unsigned int i = 0; i < size - 1; i++) {
        for (unsigned int j = i + 1; j < size; j++) {
            if (compare_files(files[i], files[j]) > 0) {
                ZFile temp = files[i];
                files[i] = files[j];
                files[j] = temp;
            }
        }
    }
}

Cassette::Cassette()
{
    tape = new char[TAPE_SIZE];
    old_cycles = 0;
    len = 0;
    type = TYPE_CHARBIN;
    mark = -1;
    inv_time = 0;
    end_time = 0;
    old_time = 0;
    file_index = 0;
    motor = 0;
    pos = 0;
    files_size = 0;
    m_dirname[0] = '\0';
    loaded_filename[0] = '\0';
}

Cassette::~Cassette()
{
    delete[] tape;
}

#define PULSE 14
char Cassette::read(unsigned int cycles, unsigned char wait) {
    char val = 0;
    wait = 38;
    int diff = (int)(cycles - old_cycles);
#ifdef HOST_COMPILE
    static int read_count = 0;
    bool log_this = (read_count < 200);
    if (log_this)
    {
        read_count++;
    }
#endif
    if ((unsigned int)diff > 4 * PULSE * 90)
    {
#ifdef HOST_COMPILE
        if (log_this) fprintf(stderr, "[Cassette::read] timeout reset: diff=%d cycles=%u old_cycles=%u\n", diff, cycles, old_cycles);
#endif
        mark = -3;
        inv_time = 0;
    } else if (mark < -2)
    {
#ifdef HOST_COMPILE
        if (log_this) fprintf(stderr, "[Cassette::read] mark recovery: mark=%d\n", (int)mark);
#endif
        mark++;
    } else if (len && mark < 0)
    {
        mark = (tape[pos] == '1' ? 1 : 0);
        old_time = cycles;
        inv_time = old_time + 120;
        end_time = old_time + 1250 + PULSE * wait * mark;
#ifdef HOST_COMPILE
        if (log_this) fprintf(stderr, "[Cassette::read] load bit from tape[%d]=%c mark=%d cycles=%u\n", pos, tape[pos], (int)mark, cycles);
#endif
        if (++pos > len)
        {
            pos = 0;
        }
    }
    if (mark > -1)
    {
        if ((int)(cycles - inv_time) < 0)
            val = 0;
        else if ((int)(cycles - end_time) < 0)
            val = 1;
#ifdef HOST_COMPILE
        if (log_this) fprintf(stderr, "[Cassette::read] mark active mark=%d inv=%u end=%u cycles=%u val=%d\n", (int)mark, inv_time, end_time, cycles, (int)val);
#endif
    }
    if ((int)(cycles - end_time) > 0)
    {
#ifdef HOST_COMPILE
        if (log_this) fprintf(stderr, "[Cassette::read] mark ended at cycles=%u end_time=%u\n", cycles, end_time);
#endif
        mark = -1;
    }
    old_cycles = cycles;
    return val;
}

void Cassette::write(char ch)
{
}

void Cassette::save(const char *name)
{
    // Saving cassette tape is not supported on bare-metal (read-only SD card).
    (void)name;
}

void Cassette::load(const char *name) 
{
    pos = 0;
    len = 0;
    char *Buffer = new char[TAPE_SIZE];
    if (!Buffer) return;

    char path[256];
    if (name)
    {
        if (strncmp(name, "SD:/", 4) != 0 && strncmp(name, "sd:/", 4) != 0) {
            strcpy(path, m_dirname);
            strcat(path, name);
        } else {
            strcpy(path, name);
        }
    } else {
        if (file_index >= 0 && file_index < (int)files_size) {
            strcpy(path, m_dirname);
            strcat(path, files[file_index].c_str());
        } else {
            delete[] Buffer;
            return;
        }
    }        

    unsigned int nBytesRead = 0;
    FIL File;
    FRESULT Result = f_open (&File, path, FA_READ | FA_OPEN_EXISTING);
    if (Result == FR_OK) {
        f_read (&File, Buffer, TAPE_SIZE, &nBytesRead);
        f_close (&File);
    }
    int size = nBytesRead;

    ZFile filename(name ? name : files[file_index].c_str());
    strcpy(loaded_filename, filename.filename());
    char ext[16];
    lower(ext, filename.extension());

    if (strcmp(ext, ".bz2") == 0) 
    {
        unsigned int dest_len = TAPE_SIZE;
        int bReturn = BZ2_bzBuffToBuffDecompress(tape, &dest_len, Buffer, size, 0, 0);
        if (bReturn == BZ_OK) {
            len = dest_len;
        }
    }
    else if (strcmp(ext, ".tap") == 0) 
    {
        memcpy(tape, Buffer, size);
        len = size;
        // Strip trailing CR/LF so the bit buffer matches the compiled-in tap0
        while (len > 0 && (tape[len - 1] == '\n' || tape[len - 1] == '\r'))
            len--;
    } 
    else if (strcmp(ext, ".cas") == 0) 
    {
        len = 0;
        int max_bits = (size * 8 > TAPE_SIZE - 1) ? (TAPE_SIZE - 1) : (size * 8);
        for(int bit = 0; bit < max_bits; bit++)
        {
            int i = bit / 8;
            int j = bit % 8;
            uint8_t c = Buffer[i];
            tape[len] = (c & (0x80 >> j)) > 0 ? '1' : '0';
            len++;
        }
    } 
    else if (strcmp(ext, ".zip") == 0)
    {
        len = loadzip(Buffer, size);
    }
    delete[] Buffer;
}

void Cassette::load(const char *data, unsigned int length, const char *filename)
{
    if (data[0] == 'P' && data[1] == 'K')
    {
        loadzip(data, length);
    }
    else
    {
        memset(tape, 0, TAPE_SIZE);
        len = length > TAPE_SIZE ? TAPE_SIZE : length;
        memcpy(tape, data, len);
        strcpy(loaded_filename, filename);
    }
}

void Cassette::setfile(const char *filename)
{
    // In bare-metal, just try to load directly by extension
    char ext[16];
    const char *dot = strrchr(filename, '.');
    if (dot) {
        lower(ext, dot);
    } else {
        ext[0] = '\0';
    }
    if (strcmp(ext, ".zip") == 0)
    {
        loadzip(filename);
    }
    else {
        load(filename);
    }
}

void Cassette::loaddir(const char *dirname)
{
    strcpy(m_dirname, dirname);
    if (m_dirname[0] == '\0') {
        strcpy(m_dirname, "SD:/taps");
    }
    int len_dir = strlen(m_dirname);
    if (len_dir > 0 && m_dirname[len_dir - 1] != '/') {
        strcat(m_dirname, "/");
    }

    files_size = 0;

    DIR Directory;
    FILINFO FileInfo;
    FRESULT Result = f_findfirst (&Directory, &FileInfo, m_dirname, "*");
    for (unsigned i = 0; Result == FR_OK && FileInfo.fname[0] && files_size < 256; i++)
    {
        ZFile file(FileInfo.fname, 0, (int)FileInfo.fsize);
        if (is_supported_extension(file.extension()))
        {
            files[files_size++] = file;
        }
        Result = f_findnext (&Directory, &FileInfo);
    }
    
    sort_files(files, files_size);
    file_index = 0;
    load();
}

int Cassette::loadzip(const char *data, int size)
{
    size_t uncomp_size, l; 
    mz_zip_archive zip;
    mz_zip_archive_file_stat file_stat;
    
    uint8_t *compressed = new uint8_t[1024*1024*1];
    uint8_t *uncompressed = new uint8_t[1024*1024*4];
    if (!compressed || !uncompressed)
    {
        delete[] compressed;
        delete[] uncompressed;
        return 0;
    }
    char unzipfile[256];
    memset(tape, 0, TAPE_SIZE);
    memset(&zip, 0, sizeof(zip));
    l = 0;
    
    if (!size)
    {
        char zipname[256];
        if (strncmp(data, "SD:/", 4) != 0 && strncmp(data, "sd:/", 4) != 0)
        {
            strcpy(zipname, m_dirname);
            strcat(zipname, data);
        }
        else 
        {
            strcpy(zipname, data);
        }
        FIL File;
        unsigned int nBytesRead = 0;
        FRESULT Result = f_open (&File, zipname, FA_READ | FA_OPEN_EXISTING);
        if (Result == FR_OK) {
            f_read (&File, compressed, 1024*1024*1, &nBytesRead);
            f_close (&File);
        }
        size = nBytesRead;
        if (size == 0)
        {
            delete[] compressed;
            delete[] uncompressed;
            return 0;
        }
    }
    else 
    {
        if (size > 1024*1024*1) size = 1024*1024*1;
        memcpy(compressed, data, size);
    }
    
    if (mz_zip_reader_init_mem(&zip, compressed, size, 0))
    {
        for (mz_uint no = 0; no < mz_zip_reader_get_num_files(&zip); no++)
        {
            if (!mz_zip_reader_file_stat(&zip, no, &file_stat))
            {
                break;
            }
            if (!strlen(file_stat.m_filename))
                continue;
            
            strncpy(unzipfile, file_stat.m_filename, 255);
            unzipfile[255] = '\0';
            uncomp_size = file_stat.m_uncomp_size;
            
            ZFile file(unzipfile);
            char ext[16];
            lower(ext, file.extension());
            
            if (strcmp(ext, ".tap") == 0)
            {
                bool ret = mz_zip_reader_extract_file_to_mem(&zip, unzipfile, uncompressed, 1024*1024*4, 0);
                if (!ret)
                {
                    break;
                }
                if (l + uncomp_size < TAPE_SIZE) {
                    memcpy(tape+l, uncompressed, uncomp_size);
                    l += uncomp_size;
                }
            } 
            else if (strcmp(ext, ".cas") == 0)
            {
                bool ret = mz_zip_reader_extract_file_to_mem(&zip, unzipfile, uncompressed, 1024*1024*4, 0);
                if (!ret)
                {
                    break;
                }
                size_t max_bits = (uncomp_size * 8 > TAPE_SIZE - 1) ? (TAPE_SIZE - 1) : (uncomp_size * 8);
                for(size_t bit = 0; bit < max_bits; bit++)
                {
                    int i = bit / 8;
                    int j = bit % 8;
                    tape[l] = (uncompressed[i] & (0x80 >> j)) > 0 ? '1' : '0';
                    l++;
                }
            } else 
                continue;
            
            if (l > 0) {
                strcpy(loaded_filename, unzipfile);
            }
            break; // Load first file
        }
    }
    mz_zip_reader_end(&zip);
    delete[] compressed;
    delete[] uncompressed;
    return l;
}
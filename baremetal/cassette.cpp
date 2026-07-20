#include "cassette.h"
#include <fatfs/ff.h>

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
    int diff = cycles - old_cycles;
    if (diff > 4 * PULSE * 90)
    {
        mark = -3;
        inv_time = 0;
    } else if (mark < -2)
    {
        mark++;
    } else if (len && mark < 0)
    {
        mark = (tape[pos] == '1' ? 1 : 0);
        old_time = cycles;
        inv_time = cycles + 70;
        end_time = cycles + 160 + PULSE * wait * mark;
        if (++pos > len)
        {
            pos = 0;
        }
    }
    if (mark > -1)
    {
        if (cycles < inv_time)
            val = 0;
        else if (cycles < end_time)
            val = 1;
    }
    if (cycles > end_time)
        mark = -1;
    old_cycles = cycles;
    return val;
}

void Cassette::write(char ch)
{
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
        unsigned int dest_len = TAPE_SIZE - 1;
        int bReturn = BZ2_bzBuffToBuffDecompress(tape, &dest_len, Buffer, size, 0, 0);
        if (bReturn == BZ_OK) {
            len = dest_len;
        }
    }
    else if (strcmp(ext, ".tap") == 0) 
    {
        memcpy(tape, Buffer, size);
        len = size;
    } 
    else if (strcmp(ext, ".cas") == 0) 
    {
        len = 0;
        for(int i = 0; i < size; i++)
        {
            uint8_t c = Buffer[i];
            for(int j = 0; j < 8; j++)  
            {
                if (len < TAPE_SIZE - 1) {
                    tape[i*8+j] = (c&(0x80>>j))>0 ? '1' : '0';
                    len++;
                }
            }
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
        ZFile file(FileInfo.fname);
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
    }
    else 
    {
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
                for(unsigned int i = 0; i < uncomp_size; i++)
                {
                    for(int j = 0; j < 8; j++)  
                    {
                        if (l < TAPE_SIZE - 1) {
                            tape[l] = (uncompressed[i]&(0x80>>j))>0 ? '1' : '0';
                            l++;
                        }
                    }
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
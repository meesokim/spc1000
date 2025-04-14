#ifndef __circle__
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
#endif 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include "cassette.h"
#include <bzlib.h>
#include <miniz_zip.h>
inline string lower(string data) {
    string ret = data;
    for (int i = 0; i < data.length(); i++)
        ret[i] = tolower(data[i]);
    return ret;
}
bool in_array(const std::string &value, const std::vector<std::string> &array)
{
    return std::find(array.begin(), array.end(), value) != array.end();
}

#define PULSE 14
char Cassette::read(uint32_t cycles, uint8_t wait) {
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
        // mark = (type == TYPE_CHARBIN ? (tape[pos] == '1' ? 1 : 0) : (tape[pos>>3] & (1 << (pos % 8) ? 1 : 0)));
        mark = (tape[pos] == '1' ? 1:0);
        // printf("%d", mark);
        // if (pos < 100)
        //     printf("%d.%d ", mark, pos);
        old_time = cycles;
        inv_time = cycles + 70;
        end_time = cycles + 160 + PULSE * wait * mark;
        // if (pos < 100)
        //     printf("%d--[%d]%d/%d,%d\n", mark, pos, inv_time - cycles, end_time - cycles, wait);
            // printf("%d\n", mark);
        if (++pos > len)
        {
            pos = 0;
            printf("tape rewinded.\n");
        }
    }
    if (mark > -1)
    {
        if (cycles < inv_time)
            val = 0;
        else if (cycles < end_time)
            val = 1;
    }
    // if (pos < 100 && inv_time > 0)
    //     printf("%d(%d)%c\n", val, cycles - old_time, val != mark ? '*' : 0);
    if (cycles > end_time)
        mark = -1;
    old_cycles = cycles;
    return val;
}

void Cassette::write(char ch)
{
    // printf("%d", ch);
}

void Cassette::load(const char *name) 
{
    pos = 0;
    len = 0;
    string file;
    int size = 0;
    char Buffer[TAPE_SIZE];
    if (!name)
    {
#ifdef __EMSCRIPTEN__
        file = files[file_index].string();
#else
        file = files[file_index].filename();
#endif
    } else {
        file = name;
    }        
#ifdef __circle__
    FIL File;
    int nBytesRead;
    Result = f_open (&File, file, FA_WRITE | FA_CREATE_ALWAYS);
    if (Result != FR_OK) {
        f_read (&File, Buffer, sizeof Buffer, &nBytesRead);
    }
    size = nBytesRead;
#else        
    memset(tape, 0, sizeof tape);
    ifstream f(file);
    f.seekg(0, std::ios::end);
    size = f.tellg();
    if (size > TAPE_SIZE) size = TAPE_SIZE;
    f.seekg(0, std::ios::beg);
    f.read((char *)Buffer, size);
    f.close();
#endif
    ZFile filename(file);
    loaded_filename = filename;
    string ext = lower(filename.extension());
    // printf("ext: %s (%d)\n", ext.c_str(), size);
    if (!ext.compare(".bz2")) 
    {
        bz_stream bStream;
        bStream.next_in = Buffer;
        bStream.avail_in = size;
        bStream.next_out = tape;
        bStream.avail_out = len;
        BZ2_bzDecompressInit(&bStream, 0, 0);
        int bReturn = BZ2_bzDecompress(&bStream);
    }
    else if (!ext.compare(".tap")) 
    {
        memcpy(tape, Buffer, size);
        len = size;
    } 
    else if (!ext.compare(".cas")) 
    {
        len = 0;
        for(int i = 0; i < size; i++)
        {
            uint8_t c = Buffer[i];
            for(int j = 0; j < 8; j++)  
            {
                tape[i*8+j] = (c&(0x80>>j))>0 ? '1' : '0';
                len++;
                // printf("%c", tape[i*8+j]);
            }
        }
    } 
    else if (!ext.compare(".zip"))
    {
        // cout << filename << endl;
        len = loadzip(Buffer, size);
    }
    printf("%s (%d)\n", loaded_filename.c_str(), len);
}

void Cassette::load(const char *data, int length, const char *filename)
{
    if (data[0] == 'P' && data[1] == 'K')
    {
        loadzip(data, length);
    }
    else
    {
        memset(tape, 0, sizeof tape);
        len = length > sizeof tape ? sizeof tape : length;
        memcpy(tape, data, len);
        // printf("load:%s (%d)\n", tape, len);
        loaded_filename = filename;
    }
}

#include <sys/stat.h>
void Cassette::setfile(const char *filename)
{
    struct stat sb;
    stat(filename, &sb);
    string ext = lower(filename);
    if (S_ISDIR(sb.st_mode))
    {
        loaddir(filename);
        printf("loaddir\n");
    }
    else if (!ext.compare(".zip"))
    {
        loadzip(filename);
        printf("loadzip\n");
    }
    else {
        load(filename);
        printf("load\n");
    }
}
void Cassette::loaddir(const char *dirname)
{
    // int index = -1;
    // this->dirname = (char *)dirname;
    printf("loaddir:%s\n", dirname);
    int index = 0;
#ifdef __circle__
	DIR Directory;
	FILINFO FileInfo;
	FRESULT Result = f_findfirst (&Directory, &FileInfo, "SD:/", "*");
    for (unsigned i = 0; Result == FR_OK && FileInfo.fname[0]; i++)
    {
        ZFile file(FileInfo.fname);
        if (in_array(lower(file.extension()), exts))
        {
            files.push_back(file);
        }
        Result = f_findnext (&Directory, &FileInfo);
    }
#else
    for (const auto & entry : fs::directory_iterator(dirname))
    {
        // cout << entry.path().extension() << endl;
        ZFile file(entry.path(), index++);
        if (in_array(lower(file.extension()), exts))
        {
            // cout << entry.path() << endl;
#ifdef __EMSCRIPTEN__
            files.push_back(file.string());
#else
            files.push_back(file);
#endif
            printf("%d. %s\n", file.index, file.c_str());
        }
    }
#endif
    sort(files.begin(), files.end());
    file_index = 0;
    load();
}

int Cassette::loadzip(const char *data, int size)
{
    size_t uncomp_size, len; 
    mz_zip_archive zip;
    mz_zip_archive_file_stat file_stat;
    uint8_t compresssed[1024*1024*1];
    uint8_t uncompressed[1024*1024*4];
    char unzipfile[1024];
    memset(tape, 0, sizeof tape);
    memset(&zip, 0, sizeof(zip));
    len = 0;
    if (!size)
    {
        string zipname(data);
        ifstream file(zipname, std::ios_base::binary);
        file.seekg(0, std::ios::end);
        size = file.tellg();
        if (size > TAPE_SIZE) size = TAPE_SIZE;
        file.seekg(0, std::ios::beg);
        file.read((char *)compresssed, size);
        file.close();
        printf("loadzip: %s (%d)\n", zipname, size);
    }
    else 
    {
        // printf("memcpy: %d\n", size);
        memcpy(compresssed, data, size);
    }
    if (mz_zip_reader_init_mem(&zip, compresssed, size, 0))
    {
        // printf("mz_zip_reader_init_mem:\n");
        for (mz_uint no = 0;no < mz_zip_reader_get_num_files(&zip); no++)
        {
            if (!mz_zip_reader_file_stat(&zip, no, &file_stat))
            {
                mz_zip_reader_end(&zip);
                // return EXIT_FAILURE;
                break;
            }
            // cout << "unzipped:" << file_stat.m_filename << endl;
            // printf("files:%d.%s (%d)\n", file_stat.m_file_index, file_stat.m_filename, file_stat.m_comp_size);
            if (!strlen(file_stat.m_filename))
                continue;
            strcpy(unzipfile, file_stat.m_filename);
            uncomp_size = file_stat.m_uncomp_size;
            ZFile file(unzipfile);
            string ext = lower(file.extension()); 
            if (!ext.compare(".tap"))
            {
                bool ret = mz_zip_reader_extract_file_to_mem(&zip, unzipfile, uncompressed, sizeof uncompressed, 0);
                // printf("%x,%s extracted\n", p, unzipfile);
                if (!ret)
                {
                    printf("fatal error\n");
                    exit(0);
                }
                memcpy(tape+len, uncompressed, uncomp_size);
                len += uncomp_size;
            } 
            else if (!ext.compare(".cas"))
            {
                bool ret = mz_zip_reader_extract_file_to_mem(&zip, unzipfile, uncompressed, sizeof uncompressed, 0);
                // printf("%x,%s extracted\n", p, unzipfile);
                if (!ret)
                {
                    printf("fatal error\n");
                    exit(0);
                }
                for(int i = 0; i < uncomp_size; i++)
                {
                    for(int j = 0; j < 8; j++)  
                    {
                        tape[len+i*8+j] = (uncompressed[i]&(0x80>>j))>0 ? '1' : '0';
                    }
                }
                uncomp_size = uncomp_size * 8; 
                len += uncomp_size;
            } else 
                continue;
            printf("unzip:%d. %s(%d)\n", no+1, unzipfile, (int) uncomp_size);
            if (!no) {
                loaded_filename = unzipfile;
            }
        }
    }
    mz_zip_reader_end(&zip);
    return len;
}
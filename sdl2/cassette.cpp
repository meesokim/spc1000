typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

#include "cassette.h"
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

// #define PULSE ((1349-200)*0.5)
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
    string filename;
    pos = 0;
    len = 0;
#ifndef __EMSCRIPTEN__
    if (archive)
    {
        zip_file* file = zip_fopen_index(archive, files[file_index].index, 0); 
        if (file) { 
            name = files[file_index].filename().c_str();
            len = zip_fread(file, tape, sizeof(tape));
            zip_fclose(file); 
        }        
    }
#endif
    if (!name)
    {
#ifdef __EMSCRIPTEN__
        filename = files[file_index].string();
#else
        filename = files[file_index].filename();
#endif
    } else {
        filename = name;
    }        
    // cout << filename << endl;
    name = filename.c_str();
    if ( strcmp(name+strlen(name)-4, ".bz2") == 0 ) 
    {
        FILE *f = fopen(name, "rb");
        BZFILE *bzf;
        int bzError;
        bzf = BZ2_bzReadOpen(&bzError, f, 0, 0, NULL, 0);
        if (bzError != BZ_OK) {
            fprintf(stderr, "E: BZ2_bzReadOpen:  %d\n", bzError);
            return;
        }
        // printf("bzip\n");
        len = BZ2_bzRead(&bzError, bzf, tape, sizeof tape);
        fclose(f);
    }
    else if (strcmp(name+strlen(name)-4, ".tap") == 0 ) 
    {
        // printf("tap:%s\n", name);
        // FILE *f = fopen(name, "r");
        memset(tape, 0, sizeof tape);
        ifstream file(filename);
        file.seekg(0, std::ios::end);
        len = file.tellg();
        if (len > TAPE_SIZE) len = TAPE_SIZE;
        file.seekg(0, std::ios::beg);
        file.read(tape, len);
        file.close();
    } 
    else if (strcmp(name+strlen(name)-4, ".cas") == 0 ) 
    {
        memset(tape, 0, sizeof tape);
        ifstream file(filename, std::ios_base::binary);
        file.seekg(0, std::ios::end);
        len = file.tellg() * 8;
        if (len > TAPE_SIZE) len = TAPE_SIZE;
        file.seekg(0, std::ios::beg);
        printf("cas:%s(%d)\n", name, len);
        for(int i = 0; i < len>>3; i++)
        {
            uint8_t c = file.get();
            for(int j = 0; j < 8; j++)  
            {
                tape[i*8+j] = (c&(0x80>>j))>0 ? '1' : '0';
                // printf("%c", tape[i*8+j]);
            }
        }
        file.close();
        // printf("%s", tape);
    }
    printf("%s (%d)\n", name, len);
}

void Cassette::load(const char *data, int length)
{
    memset(tape, 0, sizeof tape);
    len = length > sizeof tape ? sizeof tape : length;
    memcpy(tape, data, length);
    // printf("drop file from web (%d)\n", len);
}

#include <sys/stat.h>
void Cassette::setfile(const char *filename)
{
    struct stat sb;
    stat(filename, &sb);
    if (S_ISDIR(sb.st_mode))
    {
        loaddir(filename);
        printf("loaddir\n");
    }
    else if (!strcmp(filename+strlen(filename)-4, ".zip"))
    {
        loadzip(filename);
        printf("loadzip\n");
    }
    else {
        load(filename);
        printf("load\n");
    }
}
#include <algorithm>

void Cassette::loaddir(const char *dirname)
{
    int index = -1;
    this->dirname = (char *)dirname;
    printf("loaddir:%s\n", dirname);
    for (const auto & entry : fs::directory_iterator(dirname))
    {
        // cout << entry.path().extension() << endl;
        if (!entry.path().extension().compare(".tap") || !entry.path().extension().compare(".cas") || !entry.path().extension().compare(".bz2"))
        {
            // cout << entry.path() << endl;
#ifdef __EMSCRIPTEN__
            files.push_back(entry.path());
#else
            files.push_back(ZFile(entry.path(), index));
#endif
            // printf("%s,%d\n", entry.path().string().c_str(), index );
        }
    }
    sort(files.begin(), files.end());
    file_index = 0;
    load();
}

void Cassette::loadzip(const char *zipname)
{
#ifndef __EMSCRIPTEN__
    archive = zip_open(zipname, 0, NULL); 
    if (!archive) { 
        std::cerr << "Failed to open the zip file." << std::endl; 
        return; 
    }
    int numFiles = zip_get_num_files(archive);
    for (int i = 0; i < numFiles; ++i) 
    {
        struct zip_stat fileInfo; 
        zip_stat_init(&fileInfo);
        if (zip_stat_index(archive, i, 0, &fileInfo) == 0) 
        {
            // Step 4: Extract and print file contents 
            string filename(fileInfo.name);
            if (filename.find(".tap") == filename.length()-4)
            {
                files.push_back(ZFile(filename,i));
                std::cout << fileInfo.name << "," << i << std::endl; 
            }
        }
    }
    sort(files.begin(), files.end());
    load();
#endif
}
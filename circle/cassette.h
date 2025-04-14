#ifndef CASSETTE_H_
#define CASSETTE_H_

#ifndef __circle__
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <algorithm>
#include <filesystem>

#include <string>
using namespace std;
// namespace fs = filesystem;
enum casmode {CASSETTE_STOP, CASSETTE_PLAY, CASSETTE_REC};
enum castype {TYPE_CHARBIN, TYPE_BINARY};

class ZFile {
public:
    string fname;
    int index;
    ZFile(string f, int i=0) { fname = f; index = i;};
    bool operator<(const ZFile& other) {
        return fname < other.fname;
    }
    string operator=(const char *name) {
        return name;
    }
    operator string() {
        return fname;
    }
    string filename() { return fname; }
    string extension() { return fname.substr(fname.find_last_of(".") + 1); }
    const char *c_str() { return fname.c_str(); }
};

#define TAPE_SIZE (1024 * 1024 * 6)
class Cassette {
    unsigned int old_cycles;
    char tape[TAPE_SIZE];
    int len = 0;
    char type = TYPE_CHARBIN;
    char mark = -1;
    unsigned int inv_time, end_time, old_time;
#ifdef __EMSCRIPTEN__    
    vector<filesystem::path> files;
#else
    vector<ZFile> files;
#endif
    unsigned int file_index = 0;
    char *dirname;
    string loaded_filename;
    vector<string> exts {".tap",".cas",".zip",".bz2"};
public:
    char motor;
    int pos = 0;
    Cassette() {}
    void initTick(unsigned int tick) { old_cycles = tick; }
    void load(const char *name = NULL);
    void load(const char *data, int length, const char *filename);
    void save(const char *name);
    char read(unsigned int, unsigned char);
    char read1() { return 0;}
    void write(char);
    void next() { if (++file_index >= files.size()) file_index = 0; load(); }
    void get_title(char *buf) { strcpy(buf, loaded_filename.c_str()); };
    void prev() { if (--file_index < 0) file_index = files.size() - 1; load();}
    void settape(unsigned int i) 
    {
        if ( i >= files.size() )
            file_index = files.size() - 1; 
        else 
            file_index = i; 
        load(); 
    };
    void setfile(const char *);
    void loaddir(const char *);
    int loadzip(const char *, int len = 0);
};

#endif
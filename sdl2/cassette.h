#ifndef CASSETTE_H_
#define CASSETTE_H_

#include <string.h>
#include <iostream>
#include <filesystem>
#include <vector>
#ifndef __EMSCRIPTEN__
#include <zip.h> 
#endif
#include <bzlib.h>
using namespace std;
namespace fs = std::filesystem;

enum casmode {CASSETTE_STOP, CASSETTE_PLAY, CASSETTE_REC};
enum castype {TYPE_CHARBIN, TYPE_BINARY};

class ZFile {
public:
    string fname;
    int index;
    ZFile(string f, int i) { fname = f; index = i;};
    bool operator<(const ZFile& other) {
        return fname < other.fname;
    }
    string filename() { return fname; }
};

#define TAPE_SIZE (1024 * 1024 * 6)
class Cassette {
    uint32_t old_cycles;
    char tape[TAPE_SIZE];
    int len = 0;
    char type = TYPE_CHARBIN;
    char mark = -1;
    uint32_t inv_time, end_time, old_time;
#ifdef __EMSCRIPTEN__    
    vector<filesystem::path> files;
#else
    zip* archive;
    vector<ZFile> files;
#endif
    int file_index = 0;
    char *dirname;
public:
    char motor;
    int pos = 0;
    Cassette() {}
    void initTick(uint32_t tick) { old_cycles = tick; }
    void load(const char *name = NULL);
    void save(const char *name);
    char read(uint32_t, uint8_t);
    char read1() { return 0;}
    void write(char);
    void next() { if (++file_index >= files.size()) file_index = 0; load(); }
    void get_title(char *buf) { strcpy(buf, files[file_index].filename().c_str()); };
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
    void loadzip(const char *);
};

#endif
#ifndef CASSETTE_H_
#define CASSETTE_H_

#ifndef __circle__
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
#endif

#include <string.h>
#include <stdlib.h>

enum castype {TYPE_CHARBIN, TYPE_BINARY};

class ZFile {
public:
    char fname[128];
    int index;
    int size;
    ZFile() { fname[0] = '\0'; index = 0; size = 0; }
    ZFile(const char *f, int i=0, int s=0) {
        strncpy(fname, f, 127);
        fname[127] = '\0';
        index = i;
        size = s;
    }
    const char *c_str() const { return fname; }
    const char *filename() const {
        const char *base = fname;
        const char *p = fname;
        while (*p) {
            if (*p == '/' || *p == '\\') base = p + 1;
            p++;
        }
        return base;
    }
    const char *extension() const {
        const char *dot = strrchr(fname, '.');
        return dot ? dot : "";
    }
};

#define TAPE_SIZE (512 * 1024) // 512KB

class Cassette {
    unsigned int old_cycles;
    char *tape;
    int len;
    char type;
    char mark;
    unsigned int inv_time, end_time, old_time;
    
    ZFile files[256];
    unsigned int files_size;
    int file_index;
    
    char m_dirname[256];
    char loaded_filename[256];
    char m_zip_name[256];
    char m_zip_files[32][128];
    int  m_zip_file_starts[32];
    int  m_zip_file_sizes[32];
    int  m_zip_file_count;
    const char *exts[4];

public:
    char motor;
    int pos;
    int get_len() const { return len; }
    char *get_tape() { return tape; }
    Cassette();
    ~Cassette();
    void initTick(unsigned int tick) { old_cycles = tick; }
    void load(const char *name = nullptr);
    void load(const char *data, unsigned int length, const char *filename);
    void save(const char *name);
    char read(unsigned int, unsigned char);
    char read_bit() {
        if (pos >= len) return 1;
        return (tape[pos++] == '1') ? 1 : 0;
    }
    void write(char);
    void next() {
        if (files_size == 0) return;
        file_index++;
        if (file_index >= (int)files_size) file_index = 0;
        load();
    }
    void get_title(char *buf) { strcpy(buf, loaded_filename); }
    bool is_zip() const { return m_zip_name[0] != '\0'; }
    void get_zip_name(char *buf) const { strcpy(buf, m_zip_name); }
    int get_zip_file_count() const { return m_zip_file_count; }
    int get_zip_file_index() const {
        for (int i = m_zip_file_count - 1; i >= 0; i--)
            if (pos >= m_zip_file_starts[i]) return i + 1;
        return 1;
    }
    const char *get_zip_file_name() const {
        int idx = get_zip_file_index() - 1;
        if (idx >= 0 && idx < m_zip_file_count) return m_zip_files[idx];
        return loaded_filename;
    }
    int get_zip_file_size() const {
        int idx = get_zip_file_index() - 1;
        if (idx >= 0 && idx < m_zip_file_count) return m_zip_file_sizes[idx];
        return len;
    }
    int get_count() const { return files_size; }
    int get_index() const { return file_index + 1; } // 1-based
    int get_size() const { return files[file_index].size; }
    void prev() {
        if (files_size == 0) return;
        file_index--;
        if (file_index < 0) file_index = files_size - 1;
        load();
    }
    void settape(unsigned int i) {
        if (files_size == 0) return;
        if (i >= files_size) file_index = files_size - 1;
        else file_index = i;
        load();
    }
    void setfile(const char *filename);
    void loaddir(const char *dirname);
    int loadzip(const char *data, int size = 0);
};

#endif
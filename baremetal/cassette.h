#ifndef CASSETTE_H_
#define CASSETTE_H_

#ifndef __circle__
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

#define TAPE_SIZE (1536 * 1024) // 1.5MB

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
    void ensure_loaded() const;
    int get_len() { ensure_loaded(); return len; }
    char *get_tape() { ensure_loaded(); return tape; }
    Cassette();
    ~Cassette();
    void initTick(unsigned int tick) { old_cycles = tick; }
    void load(const char *name = nullptr);
    void load(const char *data, unsigned int length, const char *filename);
    void save(const char *name);
    char read(unsigned int, unsigned char);
    char read_bit() {
        ensure_loaded();
        if (pos >= len) return 1;
        if (is_zip() && m_zip_file_count > 0) {
            for (int i = 0; i < m_zip_file_count; i++) {
                if (pos >= m_zip_file_starts[i]) {
                    snprintf(loaded_filename, sizeof(loaded_filename), "%s", m_zip_files[i]);
                }
            }
        }
        return (tape[pos++] == '1') ? 1 : 0;
    }
    void write(char);
    void get_title(char *buf) { strcpy(buf, loaded_filename); }
    bool is_zip() const { return m_zip_name[0] != '\0'; }
    void get_zip_name(char *buf) const { strcpy(buf, m_zip_name); }
    int get_zip_file_count() {
        ensure_loaded();
        return m_zip_file_count;
    }
    int get_zip_file_index() const {
        ensure_loaded();
        if (m_zip_file_count <= 0) return 1;
        for (int i = m_zip_file_count - 1; i >= 0; i--)
            if (pos >= m_zip_file_starts[i]) return i + 1;
        return 1;
    }
    const char *get_zip_file_name() {
        ensure_loaded();
        int idx = get_zip_file_index() - 1;
        if (idx >= 0 && idx < m_zip_file_count && m_zip_files[idx][0] != '\0')
            return m_zip_files[idx];
        if (loaded_filename[0] != '\0')
            return loaded_filename;
        return m_zip_name;
    }
    int get_zip_file_size() const {
        ensure_loaded();
        int idx = get_zip_file_index() - 1;
        if (idx >= 0 && idx < m_zip_file_count) return m_zip_file_sizes[idx];
        return len;
    }
    int get_count() const { 
        if (files_size == 0 && m_zip_file_count > 0) return m_zip_file_count;
        return files_size; 
    }
    int get_index() const { 
        return file_index + 1; 
    }
    int get_size() const { 
        if (m_zip_name[0] != '\0' && m_zip_file_count > 0) {
            int idx = get_zip_file_index() - 1;
            if (idx >= 0 && idx < m_zip_file_count) return m_zip_file_sizes[idx];
        }
        return files[file_index].size; 
    }
    void next() {
        if (files_size == 0) return;
        file_index++;
        if (file_index >= (int)files_size) file_index = 0;
        load();
    }
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
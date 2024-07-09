#ifndef CASSETTE_H_
#define CASSETTE_H_

enum casmode {CASSETTE_STOP, CASSETTE_PLAY, CASSETTE_REC};
enum castype {TYPE_CHARBIN, TYPE_BINARY};

#define TAPE_SIZE (1024 * 1024 * 4)
class Cassette {
    uint32_t prev;
    char *tape;
    int pos = 0;
    int len = TAPE_SIZE;
    char type = TYPE_CHARBIN;
public:
    char motor;
    Cassette() {
        tape = new char[TAPE_SIZE];
    }
    void initTick(uint32_t tick) { prev = tick; }
    void load(const char *name);
    void save(const char *name);
    char read(uint32_t);
    char read1() { return 0;}
    void write(char);
};

#endif
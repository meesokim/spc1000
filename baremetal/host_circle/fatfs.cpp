#include <fatfs/ff.h>
#include <string>
#include <cstring>

static std::string MapPath(const char *path) {
    std::string s = path;
    if (s.rfind("SD:/", 0) == 0) {
        s = "./sdcard/" + s.substr(4);
    }
    return s;
}

FRESULT f_open(FIL *fp, const char *path, byte mode) {
    std::string mapped = MapPath(path);
    const char *fmode = "rb";
    if (mode & FA_WRITE) {
        if (mode & FA_OPEN_ALWAYS) {
            FILE *test = fopen(mapped.c_str(), "rb");
            if (test) {
                fclose(test);
                fmode = "r+b";
            } else {
                fmode = "wb";
            }
        } else {
            fmode = "wb";
        }
    }
    fp->fh = fopen(mapped.c_str(), fmode);
    return (fp->fh != nullptr) ? FR_OK : 1;
}

FRESULT f_close(FIL *fp) {
    if (fp->fh) {
        fclose(fp->fh);
        fp->fh = nullptr;
    }
    return FR_OK;
}

FRESULT f_read(FIL *fp, void *buff, unsigned btr, unsigned *br) {
    if (!fp->fh) return 1;
    size_t bytes = fread(buff, 1, btr, fp->fh);
    if (br) *br = bytes;
    return FR_OK;
}

FRESULT f_write(FIL *fp, const void *buff, unsigned btw, unsigned *bw) {
    if (!fp->fh) return 1;
    size_t bytes = fwrite(buff, 1, btw, fp->fh);
    if (bw) *bw = bytes;
    return FR_OK;
}

FRESULT f_lseek(FIL *fp, unsigned long ofs) {
    if (!fp->fh) return 1;
    fseek(fp->fh, ofs, SEEK_SET);
    return FR_OK;
}

unsigned long f_size(FIL *fp) {
    if (!fp->fh) return 0;
    long curr = ftell(fp->fh);
    fseek(fp->fh, 0, SEEK_END);
    long size = ftell(fp->fh);
    fseek(fp->fh, curr, SEEK_SET);
    return size;
}

FRESULT f_mount(FATFS *fs, const char *path, byte opt) {
    return FR_OK;
}

FRESULT f_findfirst(DIR *dp, FILINFO *fno, const char *path, const char *pattern) {
    if (!dp || !fno) return 1;
    std::string mapped = MapPath(path);
    dp->dp = opendir(mapped.c_str());
    if (!dp->dp) return 1;
    return f_findnext(dp, fno);
}

FRESULT f_findnext(DIR *dp, FILINFO *fno) {
    if (!dp || !dp->dp || !fno) return 1;
    struct dirent *de = readdir(dp->dp);
    if (!de) {
        closedir(dp->dp);
        dp->dp = nullptr;
        fno->fname[0] = '\0';
        return 1;
    }
    strncpy(fno->fname, de->d_name, sizeof(fno->fname) - 1);
    fno->fname[sizeof(fno->fname) - 1] = '\0';
    return FR_OK;
}

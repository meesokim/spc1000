FILE *ob_fopen(const char *filename, const char *mode);
int ob_fclose(FILE *stream);
size_t ob_fread(void *ptr, size_t size, size_t count, FILE *stream);
size_t ob_fwrite(const void *ptr, size_t size, size_t count, FILE *stream);
int ob_fflush(FILE *stream);
int ob_feof(FILE *stream);
int ob_fseek(FILE *stream, long offset, int whence);
long ob_ftell(FILE *stream);
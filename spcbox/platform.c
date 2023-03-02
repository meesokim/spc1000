/*
 * Copyright (c) 2014 Marco Maccaferri and Others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// #include "platform.h"
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "fatfs/ff.h"

FILE *ob_fopen(const char *filename, const char *mode)
{
  FRESULT res;
  BYTE flags = 0;
  FIL *fil;
  int i;

  fil = malloc(sizeof(FIL));
  if (!fil)
    return NULL;

  for (i=0; mode[i] != 0; i++) {
    switch (mode[i]) {
      case 'w':
        flags |= FA_WRITE | FA_CREATE_ALWAYS;
        break;
      case 'r':
        flags |= FA_READ;
        break;
      case '+':
        flags |= FA_READ | FA_WRITE;
        break;
    }
  }

  res = f_open(fil, filename, flags);
  if (res != FR_OK) {
    free(fil);
    return NULL;
  }

  return (FILE *) fil;
}

int ob_fclose(FILE *stream)
{
  FRESULT res;
  FIL *fil = (FIL *) stream;
  res = f_close(fil);
  if (res != FR_OK)
    return -1;

  free(fil);
  return 0;
}
size_t ob_fread(void *ptr, size_t size, size_t count, FILE *stream)
{
  FRESULT res;
  FIL *fil = (FIL *) stream;
  UINT bread;
  res = f_read(fil, ptr, size * count, &bread);
  if (res != FR_OK)
    return 0;

  return bread;
}
size_t ob_fwrite(const void *ptr, size_t size, size_t count, FILE *stream)
{
  FRESULT res;
  FIL *fil = (FIL *) stream;
  UINT bwrite;
  res = f_write(fil, ptr, size * count, &bwrite);
  if (res != FR_OK)
    return 0;

  return bwrite;
}
int ob_fflush(FILE *stream)
{
  FRESULT res;
  FIL *fil;
  if (!stream)
    return 0;

  fil = (FIL *) stream;
  res = f_sync(fil);
  if (res != FR_OK)
    return -1;

  return 0;
}
int ob_feof(FILE *stream)
{
  FIL *fil = (FIL *) stream;
  return f_eof(fil);
}
int ob_fseek(FILE *stream, long offset, int whence)
{
  FRESULT res;
  FIL *fil = (FIL *) stream;
  long o;
  switch (whence) {
    case SEEK_SET:
      o = offset;
      break;
    case SEEK_CUR:
      o = offset + f_tell(fil);
      break;
    case SEEK_END:
      o = f_size(fil) + offset;
      if (o < 0)
        o = 0;
      break;
    default:
      return -1;
  }
  res = f_lseek(fil, o);
  if (res != FR_OK)
    return -1;

  return 0;
}

long ob_ftell(FILE *stream)
{
  FIL *fil = (FIL *) stream;
  return f_tell(fil);
}

int ob_stat(const char *restrict pathname,
                struct stat *restrict statbuf) {
  FILINFO fno;
  FRESULT fr = f_stat(pathname, &fno);
  // statbuf->st_size = fno.fsize;
  // statbuf->st_time = fno.ftime;
  return fr;
};

void ob_rewind(FILE *stream) {
  f_rewind(stream);
}

FILE *freopen(const char *filename, const char *mode, FILE *stream)
{
  return 0;
}

int remove(const char *c) {
  return 0;
}

int close(int filedescriptor) {
  return 0;
}

int printf(const char *fmt, ...)
{
  return 0;
}

void *
memchr (void const *s, int c_in, size_t n)
{
  /* On 32-bit hardware, choosing longword to be a 32-bit unsigned
     long instead of a 64-bit uintmax_t tends to give better
     performance.  On 64-bit hardware, unsigned long is generally 64
     bits already.  Change this typedef to experiment with
     performance.  */
  typedef unsigned long int longword;

  const unsigned char *char_ptr;
  const longword *longword_ptr;
  longword repeated_one;
  longword repeated_c;
  unsigned char c;

  c = (unsigned char) c_in;

  /* Handle the first few bytes by reading one byte at a time.
     Do this until CHAR_PTR is aligned on a longword boundary.  */
  for (char_ptr = (const unsigned char *) s;
       n > 0 && (size_t) char_ptr % sizeof (longword) != 0;
       --n, ++char_ptr)
    if (*char_ptr == c)
      return (void *) char_ptr;

  longword_ptr = (const longword *) char_ptr;

  /* All these elucidatory comments refer to 4-byte longwords,
     but the theory applies equally well to any size longwords.  */

  /* Compute auxiliary longword values:
     repeated_one is a value which has a 1 in every byte.
     repeated_c has c in every byte.  */
  repeated_one = 0x01010101;
  repeated_c = c | (c << 8);
  repeated_c |= repeated_c << 16;
  if (0xffffffffU < (longword) -1)
    {
      repeated_one |= repeated_one << 31 << 1;
      repeated_c |= repeated_c << 31 << 1;
      if (8 < sizeof (longword))
	{
	  size_t i;

	  for (i = 64; i < sizeof (longword) * 8; i *= 2)
	    {
	      repeated_one |= repeated_one << i;
	      repeated_c |= repeated_c << i;
	    }
	}
    }

  /* Instead of the traditional loop which tests each byte, we will test a
     longword at a time.  The tricky part is testing if *any of the four*
     bytes in the longword in question are equal to c.  We first use an xor
     with repeated_c.  This reduces the task to testing whether *any of the
     four* bytes in longword1 is zero.
     We compute tmp =
       ((longword1 - repeated_one) & ~longword1) & (repeated_one << 7).
     That is, we perform the following operations:
       1. Subtract repeated_one.
       2. & ~longword1.
       3. & a mask consisting of 0x80 in every byte.
     Consider what happens in each byte:
       - If a byte of longword1 is zero, step 1 and 2 transform it into 0xff,
	 and step 3 transforms it into 0x80.  A carry can also be propagated
	 to more significant bytes.
       - If a byte of longword1 is nonzero, let its lowest 1 bit be at
	 position k (0 <= k <= 7); so the lowest k bits are 0.  After step 1,
	 the byte ends in a single bit of value 0 and k bits of value 1.
	 After step 2, the result is just k bits of value 1: 2^k - 1.  After
	 step 3, the result is 0.  And no carry is produced.
     So, if longword1 has only non-zero bytes, tmp is zero.
     Whereas if longword1 has a zero byte, call j the position of the least
     significant zero byte.  Then the result has a zero at positions 0, ...,
     j-1 and a 0x80 at position j.  We cannot predict the result at the more
     significant bytes (positions j+1..3), but it does not matter since we
     already have a non-zero bit at position 8*j+7.
     So, the test whether any byte in longword1 is zero is equivalent to
     testing whether tmp is nonzero.  */

  while (n >= sizeof (longword))
    {
      longword longword1 = *longword_ptr ^ repeated_c;

      if ((((longword1 - repeated_one) & ~longword1)
	   & (repeated_one << 7)) != 0)
	break;
      longword_ptr++;
      n -= sizeof (longword);
    }

  char_ptr = (const unsigned char *) longword_ptr;

  /* At this point, we know that either n < sizeof (longword), or one of the
     sizeof (longword) bytes starting at char_ptr is == c.  On little-endian
     machines, we could determine the first such byte without any further
     memory accesses, just by looking at the tmp result from the last loop
     iteration.  But this does not work on big-endian machines.  Choose code
     that works in both cases.  */

  for (; n > 0; --n, ++char_ptr)
    {
      if (*char_ptr == c)
	return (void *) char_ptr;
    }

  return NULL;
}
 
//[출처] FatFs로 표준 입출력 함수 사용하기(fopen() 등)|작성자 바람

void abort( void) {

}

void __sync_synchronize() {
    
}

struct tm *localtime (const time_t *__timer) 
{
   return 0;
}

time_t mktime (struct tm *__tp) {
  return 0;
}

time_t time (time_t *__timer) {
  return 0;
}

#include <utime.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

int
__utimes (const char *file, const struct timeval tvp[2])
{
  struct utimbuf buf, *times;
  if (tvp)
    {
      times = &buf;
      buf.actime = tvp[0].tv_sec + tvp[0].tv_usec / 1000000;
      buf.modtime = tvp[1].tv_sec + tvp[1].tv_usec / 1000000;
    }
  else
    times = NULL;
  return utime (file, times);
}

int
utime (const char *file, const struct utimbuf *times)
{
  struct timeval timevals[2];
  struct timeval *tvp;
  if (times != NULL)
    {
      timevals[0].tv_sec = (time_t) times->actime;
      timevals[0].tv_usec = 0L;
      timevals[1].tv_sec = (time_t) times->modtime;
      timevals[1].tv_usec = 0L;
      tvp = timevals;
    }
  else
    tvp = NULL;
  return __utimes (file, tvp);
}
weak_alias (__utimes, utimes);

extern void *__dso_handle;
extern void *_fini;
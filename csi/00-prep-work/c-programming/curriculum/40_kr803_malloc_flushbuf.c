/* K&R 8-3: Design and write _flushbuf, _fflush, and _fclose. */
#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

constexpr int MY_EOF = -1;
constexpr size_t MY_BUFSIZE = 16;
constexpr size_t MY_OPEN_MAX = 8;

enum my_flags {
  MY_READ = 1,
  MY_WRITE = 2,
  MY_UNBUF = 4,
  MY_EOF_FLAG = 8,
  MY_ERR = 16
};

typedef struct my_iobuf {
  int cnt;    /* characters left in buffer */
  char *ptr;  /* next character position */
  char *base; /* location of buffer */
  int flag;   /* mode of file access */
  int fd;     /* file descriptor */
} MyFILE;

MyFILE my_iob[MY_OPEN_MAX] = {
    {0, nullptr, nullptr, MY_READ, 0},
    {0, nullptr, nullptr, MY_WRITE, 1},
    {0, nullptr, nullptr, MY_WRITE | MY_UNBUF, 2},
};

/*
 * _flushbuf: allocates buffer on first write if base == nullptr, flushes
 * buffered bytes (ptr - base) to fp->fd, and if c != MY_EOF stores c as the
 * first byte of the refreshed buffer. Returns (unsigned char)c (or 0 on
 * MY_EOF flush), or MY_EOF on error.
 */
int _flushbuf(int c, MyFILE *fp) {
  (void)c;
  (void)fp;
  /* TODO: implement */
  return MY_EOF;
}

/*
 * _fflush: flushes output buffer for fp (or all open write streams if
 * fp == nullptr). Returns 0 on success, MY_EOF on error.
 */
int _fflush(MyFILE *fp) {
  (void)fp;
  /* TODO: implement */
  return MY_EOF;
}

/*
 * _fclose: flushes fp, frees any allocated buffer, closes fp->fd, and resets
 * fp fields (flag = 0, fd = -1, base = ptr = nullptr, cnt = 0).
 * Returns 0 on success, MY_EOF on error.
 */
int _fclose(MyFILE *fp) {
  (void)fp;
  /* TODO: implement */
  return MY_EOF;
}

int main(void) {
  int pipefd[2];
  assert(pipe(pipefd) == 0);

  MyFILE *fp = &my_iob[3];
  *fp = (MyFILE){0, nullptr, nullptr, MY_WRITE, pipefd[1]};

  /* First _flushbuf allocates buffer and stores 'A' */
  assert(_flushbuf('A', fp) == 'A');
  assert(fp->base != nullptr);
  assert(fp->ptr == fp->base + 1);
  assert(fp->cnt == (int)(MY_BUFSIZE - 1));

  /* Write 'B' into buffer and flush */
  *fp->ptr++ = 'B';
  fp->cnt--;
  assert(_fflush(fp) == 0);

  /* Closing flushes and closes write end of pipe */
  assert(_fclose(fp) == 0);
  assert(fp->fd == -1);
  assert(fp->flag == 0);

  char read_buf[16] = {0};
  ssize_t n = read(pipefd[0], read_buf, sizeof(read_buf) - 1);
  close(pipefd[0]);
  assert(n == 2);
  assert(strcmp(read_buf, "AB") == 0);

  printf("[PASS] Exercise 38: _flushbuf, _fflush, _fclose verified.\n");
  return EXIT_SUCCESS;
}

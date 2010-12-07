#define _XOPEN_SOURCE 600
#define _FILE_OFFSET_BITS 64
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static int do_fadvise_dontneed(const char *fname)
{
  int fd = open(fname, O_RDONLY);
  if (fd < 0) {
    fputs("(open failed) ", stderr);
    perror(fname);
    return 1;
  }
  int r = fdatasync(fd);
  if (r != 0) {
    fputs("(fdatasync failed) ", stderr);
    perror(fname);
  } else {
    r = posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
    if (r != 0) {
      fputs("(posix_fadvise failed) ", stderr);
      perror(fname);
    }
  }
  close(fd);
  return (r != 0);
}

int main(int argc, char **argv)
{
  int i = 0;
  int r = 0;
  for (i = 1; i < argc; ++i) {
    r |= do_fadvise_dontneed(argv[i]);
  }
  return r;
}

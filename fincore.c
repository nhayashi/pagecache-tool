#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>

int main(int argc, char *argv[]) {
    int fd;
    off_t offset = 0, off_limit;
    struct stat sb;
    void *pa = (char *)0;
    unsigned char *vec = (unsigned char *)0;
    size_t page_size = sysconf(_SC_PAGE_SIZE);
    register size_t page_index;
    int sum = 0;

#ifdef __x86_64
    printf("pagesize: %ld\n", page_size);
#else
    printf("pagesize: %d\n", page_size);
#endif

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (fstat(fd, &sb) != 0) {
        perror("fstat");
        close(fd);
        exit(EXIT_FAILURE);
    }

#ifdef __x86_64
    printf("filesize: %ld\n", sb.st_size);
    off_limit = sb.st_size;
    printf("offset limit: %ld\n", off_limit);
#else
    printf("filesize: %lld\n", sb.st_size);
    off_limit = ((1024 * 1024 * 1024) / page_size) * page_size;
    printf("offset limit: %lld\n", off_limit);
#endif

    while (sb.st_size > offset) {
#ifdef __x86_64
        printf("offset: %lu\n", offset);
#else
        printf("offset: %llu\n", offset);
#endif
        pa = mmap((void *)0, off_limit, PROT_NONE, MAP_SHARED, fd, offset);
        if (pa == MAP_FAILED) {
            perror("mmap");
            close(fd);
            exit(EXIT_FAILURE);
        }

        printf("page num: %llu\n",
                (unsigned long long)(off_limit + page_size - 1) / page_size);

        vec = calloc(1, (off_limit + page_size - 1) / page_size);
        if (vec == (void *)0) {
            perror("calloc");
            close(fd);
            exit(EXIT_FAILURE);
        }

        if (mincore(pa, off_limit, vec) != 0) {
            fprintf(stderr, "mincore(%p, %llu, %p): %s\n",
                    pa, (unsigned long long)off_limit, vec, strerror(errno));
            free(vec);
            close(fd);
            exit(EXIT_FAILURE);
        }

        for (page_index = 0; page_index <= off_limit / page_size; page_index++) {
            if (vec[page_index] & 1) {
                //printf("%lu\n", (unsigned long)page_index);
                sum++;
            }
        }

        free(vec);
        vec = (unsigned char *)0;

        munmap(pa, off_limit);

        offset += off_limit;
    }
    close(fd);

    printf("in memory pages: %d\n", sum);
    printf("in memory size: %llu\n", (unsigned long long)sum * page_size);

    exit(EXIT_SUCCESS);
}




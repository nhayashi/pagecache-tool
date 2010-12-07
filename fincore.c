#include <errno.h> /* errno */
#include <fcntl.h> /* fcntl, open */
#include <stdio.h> /* perror, fprintf, stderr, printf */
#include <stdlib.h> /* exit, calloc, free */
#include <string.h> /* strerror */
#include <sys/stat.h> /* stat, fstat */
#include <sys/types.h> /* size_t */
#include <unistd.h> /* sysconf, close */
#include <sys/mman.h> /* mmap */

int main(int argc, char *argv[]) {
    int fd;
    struct stat sb;
    void *pa = (char *)0;
    unsigned char *vec = (unsigned char *)0;
    size_t pageSize = sysconf(_SC_PAGE_SIZE);
    register size_t pageIndex;
    int sum = 0;

    printf("pagesize: %ld\n", pageSize);

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

    printf("filesize: %ld\n", sb.st_size);

    pa = mmap((void *)0, sb.st_size, PROT_NONE, MAP_SHARED, fd, 0);
    if (pa == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("page num: %ld\n", (sb.st_size + pageSize - 1) / pageSize);

    vec = calloc(1, (sb.st_size + pageSize - 1) / pageSize);
    if (vec == (void *)0) {
        perror("calloc");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (mincore(pa, sb.st_size, vec) != 0) {
        fprintf(stderr, "mincore(%p, %lu, %p): %s\n",
                pa, (unsigned long)sb.st_size, vec, strerror(errno));
        free(vec);
        close(fd);
        exit(EXIT_FAILURE);
    }

    for (pageIndex = 0; pageIndex <= sb.st_size / pageSize; pageIndex++) {
        if (vec[pageIndex] & 1) {
            //printf("%lu\n", (unsigned long)pageIndex);
            sum++;
        }
    }

    free(vec);
    vec = (unsigned char *)0;

    munmap(pa, sb.st_size);
    close(fd);

    printf("in memory pages: %d\n", sum);

    exit(EXIT_SUCCESS);
}




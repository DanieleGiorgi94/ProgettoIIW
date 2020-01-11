#include "posix_library.h"

unsigned long write_block(int fd, void *buf, unsigned long size)
{
    ssize_t v = 0;
    unsigned long r = 0;
    while (size > r) {
        while((v = write(fd, buf, size - r)) == -1) {
            if (errno != EINTR)
                print_error("write()", &v);
        }
        r += v;
        buf += v;
    }
    return r;
}
unsigned long read_block(int fd, void *buf, unsigned long size)
{
    ssize_t v = 0;
    unsigned long r = 0;
    while (size > r) {
        while((v = read(fd, buf, size - r)) == -1) {
            if (errno != EINTR)
                print_error("read()", &v);
        }
        if (v == 0)
            return r;
        r += v;
        buf += v;
    }
    return r;
}
int open_file(char *pathname, int flags)
{
    int fd;
    while ((fd = open(pathname, flags, 0666)) == -1) {
        if (errno != EINTR)
            print_error("open()", &fd);
    }
    return fd;
}
void close_file(int fd)
{
    int r;
    while ((r = close(fd)) == -1) {
        if (errno != EINTR) {
            int res = close(fd);
            print_error("close()", &res);
        }
    }
}
int move_offset(int fd, char c, off_t offset)
{
    int r;
    if (c == END) {
        r = lseek(fd, 0, SEEK_END);
        print_error("lseek()", &r);
    }
    if (c == CUR) {
        r = lseek(fd, offset, SEEK_CUR);
        print_error("lseek()", &r);
    }
    if (c == SET) {
        r = lseek(fd, offset, SEEK_SET);
        print_error("lseek()", &r);
    }

    return r;
}

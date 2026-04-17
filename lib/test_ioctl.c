#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <lfa.h>

int main(int argc, char **argv) {
    const char *dev = "/dev/lfa0";
    int fd;
    int32_t a = 2;
    int32_t b = 10;
    int32_t read_a = 0;
    int32_t read_b = 0;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [device]\n", argv[0]);
        return 2;
    }
    if (argc == 2) {
        dev = argv[1];
    }

    fd = open(dev, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "open(%s) failed: %s\n", dev, strerror(errno));
        return 1;
    }

    if (ioctl(fd, LFA_WRITE_A, &a) < 0) {
        fprintf(stderr, "ioctl LFA_WRITE_A failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    if (ioctl(fd, LFA_WRITE_B, &b) < 0) {
        fprintf(stderr, "ioctl LFA_WRITE_B failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    if (ioctl(fd, LFA_READ_A, &read_a) < 0) {
        fprintf(stderr, "ioctl LFA_READ_A failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    if (ioctl(fd, LFA_READ_B, &read_b) < 0) {
        fprintf(stderr, "ioctl LFA_READ_B failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    close(fd);

    printf("A: wrote=%d read=%d\n", a, read_a);
    printf("B: wrote=%d read=%d\n", b, read_b);

    if (read_a != a || read_b != b) {
        fprintf(stderr, "ioctl value mismatch\n");
        return 1;
    }

    printf("ioctl test passed\n");
    return 0;
}

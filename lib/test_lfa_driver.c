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
    int fd = -1;
    int rc = 1;

    int32_t a = 2;
    int32_t b = 10;
    int32_t read_a = 0;
    int32_t read_b = 0;

    const int32_t input[4] = {1, 2, 3, 4};
    int32_t output[4] = {0, 0, 0, 0};
    const int32_t expected[4] = {12, 14, 16, 18};

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

    /* Setup; Write LFA parameters */
    if (ioctl(fd, LFA_WRITE_A, &a) < 0) {
        fprintf(stderr, "ioctl LFA_WRITE_A failed: %s\n", strerror(errno));
        goto out;
    }
    if (ioctl(fd, LFA_WRITE_B, &b) < 0) {
        fprintf(stderr, "ioctl LFA_WRITE_B failed: %s\n", strerror(errno));
        goto out;
    }

    /* Write input values to the LFA device */
    if (write(fd, input, sizeof(input)) != (ssize_t)sizeof(input)) {
        fprintf(stderr, "write(%s) failed: %s\n", dev, strerror(errno));
        goto out;
    }

    printf("Waiting 1 second before reading output vector\n");
    sleep(1);

    /* Read output values from the LFA device */
    if (read(fd, output, sizeof(output)) != (ssize_t)sizeof(output)) {
        fprintf(stderr, "read(%s) failed: %s\n", dev, strerror(errno));
        goto out;
    }

    printf("lane0: 0x%08x\n", (uint32_t)output[0]);
    printf("lane1: 0x%08x\n", (uint32_t)output[1]);
    printf("lane2: 0x%08x\n", (uint32_t)output[2]);
    printf("lane3: 0x%08x\n", (uint32_t)output[3]);

    if (output[0] != expected[0] || output[1] != expected[1] ||
        output[2] != expected[2] || output[3] != expected[3]) {
        fprintf(stderr, "expected: 0x%08x 0x%08x 0x%08x 0x%08x\n",
                (uint32_t)expected[0], (uint32_t)expected[1],
                (uint32_t)expected[2], (uint32_t)expected[3]);
        fprintf(stderr, "got:      0x%08x 0x%08x 0x%08x 0x%08x\n",
                (uint32_t)output[0], (uint32_t)output[1], (uint32_t)output[2],
                (uint32_t)output[3]);
        goto out;
    }

    printf("lfa driver test passed\n");
    rc = 0;

out:
    if (fd >= 0) {
        close(fd);
    }
    return rc;
}

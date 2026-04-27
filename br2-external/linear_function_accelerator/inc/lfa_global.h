#ifndef LFA_GLOBAL_H
#define LFA_GLOBAL_H

#include <linux/atomic.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/kdev_t.h>

enum lfa_state {
    LFA_STATE_IDLE = 0,
    LFA_STATE_BUSY = 1,
    LFA_STATE_COMPLETE = 2
};

struct lfa_device_data {
    struct cdev lfa_cdev;
    dev_t dev_num;
    int minor;
    int irq;
    atomic_t open_count;
    void *input_buffer;
    void *output_buffer;
    void __iomem *mmio_base;
    enum lfa_state state;
};

#endif // LFA_GLOBAL_H

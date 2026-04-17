#include "lfa_ccallbacks.h"
#include "lfa_accelerator.h"
#include "lfa_global.h"

#include <linux/atomic.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>

int lfa_open(struct inode *inode, struct file *file) {
    struct lfa_device_data *data =
        container_of(inode->i_cdev, struct lfa_device_data, lfa_cdev);

    /* Allow only one open file at a time */
    if (atomic_inc_return(&data->open_count) > 1) {
        atomic_dec(&data->open_count);
        return -EBUSY;
    }

    /* Allocate input and output buffers */
    data->input_buffer = kmalloc(INPUT_BUFFER_SIZE, GFP_KERNEL);
    if (!data->input_buffer) {
        atomic_dec(&data->open_count);
        return -ENOMEM;
    }
    data->output_buffer = kmalloc(OUTPUT_BUFFER_SIZE, GFP_KERNEL);
    if (!data->output_buffer) {
        kfree(data->input_buffer);
        atomic_dec(&data->open_count);
        return -ENOMEM;
    }

    /* Store data ptr in file private data for read/write function */
    file->private_data = data;

    return 0;
}

int lfa_release(struct inode *inode, struct file *file) {
    struct lfa_device_data *data =
        container_of(inode->i_cdev, struct lfa_device_data, lfa_cdev);

    /* Free input and output buffers */
    kfree(data->input_buffer);
    kfree(data->output_buffer);

    atomic_dec(&data->open_count);
    return 0;
}

ssize_t lfa_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    /* Check if the read size is as expected */
    if (len != OUTPUT_BUFFER_SIZE) {
        pr_warn("lfa ignore input: unexpected read size: %zu (expected %zu)\n",
                len, OUTPUT_BUFFER_SIZE);
        return -EINVAL;
    }

    /* Copy data from device to output buffer */
    struct lfa_device_data *data = filp->private_data;
    memcpy_fromio(data->output_buffer,
                  (u8 __iomem *)data->mmio_base + OUTPUT_REG_OFFSET,
                  OUTPUT_BUFFER_SIZE);

    /* Copy data from kernel space to user space */
    if (copy_to_user(buf, data->output_buffer, len)) {
        return -EFAULT;
    }

    return len;
}

ssize_t lfa_write(struct file *filp, const char __user *buf, size_t len,
                  loff_t *off) {
    /* Check if the write size is as expected */
    if (len != INPUT_BUFFER_SIZE) {
        pr_warn("lfa ignore input: unexpected write size: %zu (expected %zu)\n",
                len, INPUT_BUFFER_SIZE);
        return -EINVAL;
    }

    /* Copy data from user space to kernel space */
    struct lfa_device_data *data = filp->private_data;
    if (copy_from_user(data->input_buffer, buf, len)) {
        return -EFAULT;
    }

    /* Copy data into device */
    memcpy_toio((u8 __iomem *)data->mmio_base + INPUT_REG_OFFSET,
                data->input_buffer, INPUT_BUFFER_SIZE);

    return len;
}

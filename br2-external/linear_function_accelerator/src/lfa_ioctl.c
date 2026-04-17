#include "lfa_ioctl.h"
#include "lfa_accelerator.h"
#include "lfa.h"
#include "lfa_global.h"

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/uaccess.h>

long lfa_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    pr_info("IOCTL command received: %u\n", cmd);

    struct lfa_device_data *data = file->private_data;
    s32 value = 0;

    if (!data || !data->mmio_base) {
        return -ENODEV;
    }

    switch (cmd) {
    case LFA_WRITE_A:
        if (copy_from_user(&value, (int32_t*)arg, sizeof(value))) {
            pr_err("Data Write : Err!\n");
            return -EFAULT;
        }
        iowrite32(value, (u8 __iomem *)data->mmio_base + A_OFFSET);
        pr_info("Value = %d\n", value);
        break;

    case LFA_READ_A:
        value = ioread32((u8 __iomem *)data->mmio_base + A_OFFSET);
        if (copy_to_user((int32_t*)arg, &value, sizeof(value))) {
            pr_err("Data Read : Err!\n");
            return -EFAULT;
        }
        break;

    case LFA_WRITE_B:
        if (copy_from_user(&value, (int32_t*)arg, sizeof(value))) {
            pr_err("Data Write : Err!\n");
            return -EFAULT;
        }
        iowrite32(value, (u8 __iomem *)data->mmio_base + B_OFFSET);
        pr_info("Value = %d\n", value);
        break;

    case LFA_READ_B:
        value = ioread32((u8 __iomem *)data->mmio_base + B_OFFSET);
        if (copy_to_user((int32_t*)arg, &value, sizeof(value))) {
            pr_err("Data Read : Err!\n");
            return -EFAULT;
        }
        break;

    default:
        pr_info("Invalid IOCTL command - Ignore command\n");
    }
    return 0;
}

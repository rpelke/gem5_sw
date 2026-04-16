#include "lfa_device.h"

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/idr.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>

static dev_t lfa_base_dev_num;
static struct class *dev_class = NULL;
static bool chrdev_region_allocated = false;

/* ID allocator for minor numbers */
static DEFINE_IDA(lfa_minors);

/* Maximum number of minor numbers */
#define LFA_MAX_MINORS 256

/* Mutex for protecting device registration */
static DEFINE_MUTEX(lfa_device_lock);

static const char *driver_name = "lfa";
static const char *class_name = "lfa";
static const char *device_name = "lfa";

static int lfa_open(struct inode *inode, struct file *file) {
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

static int lfa_release(struct inode *inode, struct file *file) {
    struct lfa_device_data *data =
        container_of(inode->i_cdev, struct lfa_device_data, lfa_cdev);

    /* Free input and output buffers */
    kfree(data->input_buffer);
    kfree(data->output_buffer);

    atomic_dec(&data->open_count);
    return 0;
}

static ssize_t lfa_read(struct file *filp, char __user *buf, size_t len,
                        loff_t *off) {
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

static ssize_t lfa_write(struct file *filp, const char __user *buf, size_t len,
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

static const struct file_operations lfa_fops = {
    .owner = THIS_MODULE,
    .read = lfa_read,
    .write = lfa_write,
    .open = lfa_open,
    .release = lfa_release,
};

int lfa_register_device(struct platform_device *pdev) {
    int ret;
    int minor;
    struct device *dev_ret;
    struct lfa_device_data *device_data;
    struct resource *res;

    /* Allocate and initialize device data struct */
    device_data =
        devm_kzalloc(&pdev->dev, sizeof(struct lfa_device_data), GFP_KERNEL);
    if (!device_data) {
        return -ENOMEM;
    }
    atomic_set(&device_data->open_count, 0);
    device_data->input_buffer = NULL;
    device_data->output_buffer = NULL;
    device_data->mmio_base = NULL;
    device_data->dev_num = 0;
    device_data->minor = -1;
    device_data->state = LFA_STATE_IDLE;

    /* Get MMIO resource and map it:
     * This maps the physical address range of the device into the kernel's
     * virtual address space, allowing the driver to access the device's
     * registers. The resource is defined in the device tree
     */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
        return -ENODEV;
    }
    device_data->mmio_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(device_data->mmio_base)) {
        return PTR_ERR(device_data->mmio_base);
    }

    mutex_lock(&lfa_device_lock);

    /* Allocate device numbers from (MAJOR, 0) to (MAJOR, LFA_MAX_MINORS-1):
     * This only has to be done once for the entire driver, since we can use the
     * minor number to differentiate between devices.
     */
    if (!chrdev_region_allocated) {
        ret = alloc_chrdev_region(&lfa_base_dev_num, 0, LFA_MAX_MINORS,
                                  driver_name);
        if (ret < 0) {
            goto err_unlock;
        }
        chrdev_region_allocated = true;
    }

    /* Create device class:
     * This creates an entry in /sys/class/<class_name> and allows udev to
     * create device nodes in /dev/ when we create devices below. We only need
     * one class for all devices.
     */
    if (!dev_class) {
        dev_class = class_create(class_name);
        if (IS_ERR(dev_class)) {
            ret = PTR_ERR(dev_class);
            dev_class = NULL;
            goto err_unregister_region;
        }
    }

    /* Allocate a minor number for this device:
     * This is done for each device instance and is used to create a unique
     * device number for each device.
     */
    minor = ida_alloc(&lfa_minors, GFP_KERNEL);
    if (minor < 0) {
        ret = minor;
        goto err_destroy_class;
    }
    device_data->minor = minor;
    device_data->dev_num = MKDEV(MAJOR(lfa_base_dev_num), minor);

    /* Initialize and register character device */
    cdev_init(&device_data->lfa_cdev, &lfa_fops);
    device_data->lfa_cdev.owner = THIS_MODULE;
    ret = cdev_add(&device_data->lfa_cdev, device_data->dev_num, 1);
    if (ret < 0) {
        goto err_free_minor;
    }

    /* Create a new device file:
     * Create entry in /sys/class/<class_name>/<device_name>,
     * triggers creation of /dev/<device_name> via udev
     */
    dev_ret = device_create(dev_class, &pdev->dev, device_data->dev_num, NULL,
                            "%s%d", device_name, device_data->minor);
    if (IS_ERR(dev_ret)) {
        ret = PTR_ERR(dev_ret);
        goto err_device_create;
    }

    /* Save device data for later use:
     * It can be retrieved using `platform_get_drvdata`.
     */
    platform_set_drvdata(pdev, device_data);

    mutex_unlock(&lfa_device_lock);

    pr_info("%s: initialized successfully (major=%d, minor=%d)\n", driver_name,
            MAJOR(device_data->dev_num), MINOR(device_data->dev_num));
    return 0;

err_device_create:
    cdev_del(&device_data->lfa_cdev);
err_free_minor:
    ida_free(&lfa_minors, device_data->minor);
err_destroy_class:
    if (ida_is_empty(&lfa_minors) && dev_class) {
        class_destroy(dev_class);
        dev_class = NULL;
    }
err_unregister_region:
    if (ida_is_empty(&lfa_minors) && chrdev_region_allocated) {
        unregister_chrdev_region(lfa_base_dev_num, LFA_MAX_MINORS);
        chrdev_region_allocated = false;
    }
err_unlock:
    mutex_unlock(&lfa_device_lock);
    return ret;
}

void lfa_unregister_device(struct platform_device *pdev) {
    struct lfa_device_data *device_data = platform_get_drvdata(pdev);

    if (!device_data) {
        return;
    }

    mutex_lock(&lfa_device_lock);

    device_destroy(dev_class, device_data->dev_num);
    cdev_del(&device_data->lfa_cdev);
    ida_free(&lfa_minors, device_data->minor);

    if (ida_is_empty(&lfa_minors) && dev_class) {
        class_destroy(dev_class);
        dev_class = NULL;
    }
    if (ida_is_empty(&lfa_minors) && chrdev_region_allocated) {
        unregister_chrdev_region(lfa_base_dev_num, LFA_MAX_MINORS);
        chrdev_region_allocated = false;
    }

    platform_set_drvdata(pdev, NULL);
    mutex_unlock(&lfa_device_lock);
}

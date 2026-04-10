#include "lfa_device.h"

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>

static dev_t dev_num;
static struct class *dev_class;

static const char *driver_name = "lfa";
static const char *class_name = "lfa";
static const char *device_name = "lfa";

static int lfa_open(struct inode *inode, struct file *file) {
    struct lfa_device_data *data =
        container_of(inode->i_cdev, struct lfa_device_data, lfa_cdev);

    if (atomic_inc_return(&data->open_count) > 1) {
        atomic_dec(&data->open_count);
        return -EBUSY;
    }
    return 0;
}

static int lfa_release(struct inode *inode, struct file *file) {
    struct lfa_device_data *data =
        container_of(inode->i_cdev, struct lfa_device_data, lfa_cdev);

    atomic_dec(&data->open_count);
    return 0;
}

static ssize_t lfa_read(struct file *filp, char __user *buf, size_t len,
                        loff_t *off) {
    return 0;
}

static ssize_t lfa_write(struct file *filp, const char __user *buf, size_t len,
                         loff_t *off) {
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
    struct device *dev_ret;
    struct lfa_device_data *device_data;

    device_data =
        devm_kzalloc(&pdev->dev, sizeof(struct lfa_device_data), GFP_KERNEL);
    if (!device_data) {
        return -ENOMEM;
    }
    atomic_set(&device_data->open_count, 0);
    device_data->mmio_base = NULL;

    pr_info("INIT lfa driver\n");

    /* Allocate device numbers (adds entry to /proc/devices) */
    ret = alloc_chrdev_region(&dev_num, 0, 1, driver_name);
    if (ret < 0) {
        goto err_alloc_chrdev;
    }
    pr_info("Major = %d Minor = %d\n", MAJOR(dev_num), MINOR(dev_num));

    /* Initialize and register character device */
    cdev_init(&device_data->lfa_cdev, &lfa_fops);
    device_data->lfa_cdev.owner = THIS_MODULE;
    ret = cdev_add(&device_data->lfa_cdev, dev_num, 1);
    if (ret < 0) {
        goto err_cdev_add;
    }

    /* Create device class (creates /sys/class/<class_name>/) */
    dev_class = class_create(class_name);
    if (IS_ERR(dev_class)) {
        ret = PTR_ERR(dev_class);
        goto err_class_create;
    }

    /* Create device (creates /sys/class/<class_name>/<device_name>/
     * and triggers creation of /dev/<device_name> via udev) */
    dev_ret = device_create(dev_class, NULL, dev_num, NULL, device_name);
    if (IS_ERR(dev_ret)) {
        ret = PTR_ERR(dev_ret);
        goto err_device_create;
    }

    struct resource *res;
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
        ret = -ENODEV;
        goto err_device_create;
    }
    device_data->mmio_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(device_data->mmio_base)) {
        ret = PTR_ERR(device_data->mmio_base);
        goto err_device_create;
    }
    platform_set_drvdata(pdev, device_data);

    pr_info("Driver initialized successfully\n");
    return 0;

err_device_create:
    class_destroy(dev_class);
err_class_create:
    cdev_del(&device_data->lfa_cdev);
err_cdev_add:
    unregister_chrdev_region(dev_num, 1);
err_alloc_chrdev:
    return ret;
}

void lfa_unregister_device(struct platform_device *pdev) {
    struct lfa_device_data *device_data = platform_get_drvdata(pdev);

    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&device_data->lfa_cdev);

    unregister_chrdev_region(dev_num, 1);
}

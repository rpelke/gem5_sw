#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rebecca Pelke");
MODULE_DESCRIPTION("Driver for the linear function accelerator");
MODULE_VERSION("1.0");

/* ##### BIND DEVICE TO DRIVER #####
 * Called when a matching device is found (lfa_probe) or
 * removed (lfa_remove) and bound to this driver
 */
static int lfa_probe(struct platform_device *pdev) {
    dev_info(&pdev->dev, "device initialized\n");
    return 0;
}

static void lfa_remove(struct platform_device *pdev) {
    dev_info(&pdev->dev, "device removed\n");
}

static const struct of_device_id lfa_of_match[] = {
    {.compatible = "gem5,linear_function_accelerator"}, {}};

MODULE_DEVICE_TABLE(of, lfa_of_match);

static struct platform_driver lfa_driver = {
    .probe = lfa_probe,
    .remove = lfa_remove,
    .driver =
        {
            .name = "lfa",
            .of_match_table = lfa_of_match,
        },
};

/* ##### FILE OPERATIONS #####
 * Callbacks for file operation on the device file
 */

/* This function will be called when we open the Device file */
static int lfa_open(struct inode *inode, struct file *file) { return 0; }

/* This function will be called when we close the Device file */
static int lfa_release(struct inode *inode, struct file *file) { return 0; }

/* This function will be called when we read the Device file */
static ssize_t lfa_read(struct file *filp, char __user *buf, size_t len,
                        loff_t *off) {
    return 0;
}
/* This function will be called when we write the Device file */
static ssize_t lfa_write(struct file *filp, const char __user *buf, size_t len,
                         loff_t *off) {
    return len;
}

/* ##### INIT/EXIT MODULE #####
 * Called when the module is loaded (registers the driver) or unloaded
 * (unregisters the driver)
 */
static dev_t dev_num;
static struct class *dev_class;
static struct cdev lfa_cdev;

static const struct file_operations lfa_fops = {
    .owner = THIS_MODULE,
    .read = lfa_read,
    .write = lfa_write,
    .open = lfa_open,
    .release = lfa_release,
};

const char *driver_name = "lfa";
const char *class_name = "lfa";
const char *device_name = "lfa";

static int __init lfa_init(void) {
    int ret;
    struct device *dev_ret;

    pr_info("INIT lfa driver\n");

    /* Allocate device numbers (adds entry to /proc/devices) */
    ret = alloc_chrdev_region(&dev_num, 0, 1, driver_name);
    if (ret < 0) {
        pr_err("Cannot allocate device numbers\n");
        goto err_alloc_chrdev;
    }
    pr_info("Major = %d Minor = %d\n", MAJOR(dev_num), MINOR(dev_num));

    /* Initialize and register character device */
    cdev_init(&lfa_cdev, &lfa_fops);
    lfa_cdev.owner = THIS_MODULE;
    ret = cdev_add(&lfa_cdev, dev_num, 1);
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

    /* Register platform driver */
    ret = platform_driver_register(&lfa_driver);
    if (ret) {
        goto err_platform_register;
    }

    pr_info("Driver initialized successfully\n");
    return 0;

err_platform_register:
    device_destroy(dev_class, dev_num);
err_device_create:
    class_destroy(dev_class);
err_class_create:
    cdev_del(&lfa_cdev);
err_cdev_add:
    unregister_chrdev_region(dev_num, 1);
err_alloc_chrdev:
    return ret;
}

static void __exit lfa_exit(void) {
    pr_info("EXIT lfa driver\n");

    platform_driver_unregister(&lfa_driver);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&lfa_cdev);
    unregister_chrdev_region(dev_num, 1);

    pr_info("Driver removed successfully\n");
}

module_init(lfa_init);
module_exit(lfa_exit);

#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rebecca Pelke");
MODULE_DESCRIPTION("Driver for my custom peripheral");
MODULE_VERSION("1.0");

/* ##### BIND DEVICE TO DRIVER #####
 * Called when a matching device is found (my_custom_peripheral_probe) or
 * removed (my_custom_peripheral_remove) and bound to this driver
 */
static int my_custom_peripheral_probe(struct platform_device *pdev) {
    dev_info(&pdev->dev, "device initialized\n");
    return 0;
}

static void my_custom_peripheral_remove(struct platform_device *pdev) {
    dev_info(&pdev->dev, "device removed\n");
}

static const struct of_device_id my_custom_peripheral_of_match[] = {
    {.compatible = "gem5,my-custom-peripheral"}, {}};

MODULE_DEVICE_TABLE(of, my_custom_peripheral_of_match);

static struct platform_driver my_custom_peripheral_driver = {
    .probe = my_custom_peripheral_probe,
    .remove = my_custom_peripheral_remove,
    .driver =
        {
            .name = "my-custom-peripheral",
            .of_match_table = my_custom_peripheral_of_match,
        },
};

/* ##### INIT/EXIT MODULE #####
 * Called when the module is loaded (registers the driver) or unloaded
 * (unregisters the driver)
 */
static dev_t dev_num;
static struct class *dev_class;

const char *driver_name = "my_custom_peripheral";
const char *class_name = "my_custom_peripheral";
const char *device_name = "my_custom_peripheral";

static int __init my_driver_init(void) {
    int ret;
    struct device *dev_ret;

    pr_info("INIT my_custom_peripheral driver\n");

    /* Allocate device numbers (adds entry to /proc/devices) */
    ret = alloc_chrdev_region(&dev_num, 0, 1, driver_name);
    if (ret < 0) {
        pr_err("Cannot allocate device numbers\n");
        goto err_alloc_chrdev;
    }
    pr_info("Major = %d Minor = %d\n", MAJOR(dev_num), MINOR(dev_num));

    /* Create device class (creates /sys/class/<class_name>/)*/
    dev_class = class_create(class_name);
    if (IS_ERR(dev_class)) {
        ret = PTR_ERR(dev_class);
        goto err_class_create;
    }

    /* Create device (creates /sys/class/<class_name>/<device_name>/
     * and triggers creation of /dev/<device_name> via udev)*/
    dev_ret = device_create(dev_class, NULL, dev_num, NULL, device_name);
    if (IS_ERR(dev_ret)) {
        ret = PTR_ERR(dev_ret);
        goto err_device_create;
    }

    /* Register platform driver */
    ret = platform_driver_register(&my_custom_peripheral_driver);
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
    unregister_chrdev_region(dev_num, 1);
err_alloc_chrdev:
    return ret;
}

static void __exit my_driver_exit(void) {
    pr_info("EXIT my_custom_peripheral driver\n");

    platform_driver_unregister(&my_custom_peripheral_driver);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    unregister_chrdev_region(dev_num, 1);

    pr_info("Driver removed successfully\n");
}

module_init(my_driver_init);
module_exit(my_driver_exit);

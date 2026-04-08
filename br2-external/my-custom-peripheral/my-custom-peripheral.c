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
dev_t dev = 0;

static int __init my_driver_init(void) {
  pr_info("INIT my_custom_peripheral driver\n");

  /*Allocating Major number*/
  if ((alloc_chrdev_region(&dev, 0, 1, "my_custom_peripheral")) < 0) {
    printk(KERN_INFO "Cannot allocate major number for device 1\n");
    return -1;
  }
  printk(KERN_INFO "Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));
  printk(KERN_INFO "Kernel Module Inserted Successfully...\n");

  return platform_driver_register(&my_custom_peripheral_driver);
}

static void __exit my_driver_exit(void) {
  pr_info("EXIT my_custom_peripheral driver\n");

  unregister_chrdev_region(dev, 1);
  printk(KERN_INFO "Kernel Module Removed Successfully...\n");

  platform_driver_unregister(&my_custom_peripheral_driver);
}

module_init(my_driver_init);
module_exit(my_driver_exit);

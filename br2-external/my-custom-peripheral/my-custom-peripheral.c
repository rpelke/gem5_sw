#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rebecca Pelke");
MODULE_DESCRIPTION("Driver for my custom peripheral");
MODULE_VERSION("1.0");

// Called by the Linux kernel when a matching device is found
// (i.e., when a device tree node with compatible = "gem5,my-custom-peripheral"
// is matched to this driver).
static int my_custom_peripheral_probe(struct platform_device *pdev) {
  // Print a message to the kernel log (dmesg)
  dev_info(&pdev->dev, "driver loaded\n");
  return 0;
}

// Called when the driver is removed or the device is detached
// This is where you would normally clean up resources allocated in probe()
static int my_custom_peripheral_remove(struct platform_device *pdev) {
  dev_info(&pdev->dev, "driver unloaded\n");
  return 0;
}

// Device Tree match table
// This tells the kernel which "compatible" strings this driver supports
static const struct of_device_id my_custom_peripheral_of_match[] = {
    // If a DT node has this compatible string, this driver can handle it
    {.compatible = "gem5,my-custom-peripheral"},
    {}};

// Expose the match table to tools like udev/modprobe
MODULE_DEVICE_TABLE(of, my_custom_peripheral_of_match);

// Define the platform driver structure: This structure connects the callbacks
// (probe/remove) with the kernel driver model
static struct platform_driver my_custom_peripheral_driver = {
    // Function called when a matching device is found
    .probe = my_custom_peripheral_probe,

    // Function called when the device/driver is removed
    .remove_new = my_custom_peripheral_remove,

    .driver =
        {
            // Name of the driver (appears in sysfs and logs)
            .name = "my-custom-peripheral",

            // Link to the Device Tree match table
            // This enables automatic matching via "compatible"
            .of_match_table = my_custom_peripheral_of_match,
        },
};

module_platform_driver(my_custom_peripheral_driver);

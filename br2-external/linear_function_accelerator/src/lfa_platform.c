#include "lfa_device.h"

#include <linux/of.h>
#include <linux/platform_device.h>

static int lfa_probe(struct platform_device *pdev) {
    dev_info(&pdev->dev, "device initialized\n");

    int ret = lfa_register_device(pdev);
    if (ret) {
        dev_err(&pdev->dev, "failed to register device\n");
    }

    return ret;
}

static void lfa_remove(struct platform_device *pdev) {
    dev_info(&pdev->dev, "device removed\n");
    lfa_unregister_device(pdev);
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

module_platform_driver(lfa_driver);

#include "lfa_accelerator.h"
#include "lfa_device.h"
#include "lfa_global.h"

#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/platform_device.h>

/* IRQ handler */
static irqreturn_t lfa_irq_handler(int irq, void *dev_id) {
    pr_info("Interrupt(IRQ Handler)\n");
    return IRQ_WAKE_THREAD;
}

/* Threaded IRQ handler to perform the actual processing */
static irqreturn_t lfa_irq_thread_fn(int irq, void *dev_id) {
    pr_info("Interrupt(Threaded Handler)\n");

    struct platform_device *pdev = dev_id;
    struct lfa_device_data *data = platform_get_drvdata(pdev);

    /* Copy data from device to output buffer */
    memcpy_fromio(data->output_buffer,
                  (u8 __iomem *)data->mmio_base + OUTPUT_REG_OFFSET,
                  OUTPUT_BUFFER_SIZE);

    /* 64-bit write: 1 -> irqStatusReg @ offset 0x30 */
    writeq(1ULL, data->mmio_base + 0x30);

    return IRQ_HANDLED;
}

static int lfa_probe(struct platform_device *pdev) {
    struct lfa_device_data *device_data;
    int ret;
    int irq;
    dev_info(&pdev->dev, "device initialized\n");

    ret = lfa_register_device(pdev);
    if (ret) {
        dev_err(&pdev->dev, "failed to register device\n");
        return ret;
    }

    irq = platform_get_irq(pdev, 0);
    if (irq < 0) {
        dev_err(&pdev->dev, "failed to get irq from device tree: %d\n", irq);
        lfa_unregister_device(pdev);
        return irq;
    }
    dev_info(&pdev->dev, "got irq %d from device tree\n", irq);

    ret = request_threaded_irq(irq, (void *)lfa_irq_handler, lfa_irq_thread_fn,
                               IRQF_TRIGGER_RISING, dev_name(&pdev->dev), pdev);
    if (ret) {
        dev_err(&pdev->dev, "failed to request irq %d: %d\n", irq, ret);
        lfa_unregister_device(pdev);
        return ret;
    }
    dev_info(&pdev->dev, "requested irq %d successfully\n", irq);

    device_data = platform_get_drvdata(pdev);
    device_data->irq = irq;

    return ret;
}

static void lfa_remove(struct platform_device *pdev) {
    struct lfa_device_data *device_data = platform_get_drvdata(pdev);

    if (device_data && device_data->irq >= 0) {
        free_irq(device_data->irq, pdev);
    }

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

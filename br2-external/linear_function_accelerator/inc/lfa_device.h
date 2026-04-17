#ifndef LFA_DEVICE_H
#define LFA_DEVICE_H

#include <linux/platform_device.h>

#include "lfa_global.h"


int lfa_register_device(struct platform_device *pdev);
void lfa_unregister_device(struct platform_device *pdev);

#endif // LFA_DEVICE_H

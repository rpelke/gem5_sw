#ifndef LFA_IOCTL_H
#define LFA_IOCTL_H

#include <linux/fs.h>

long lfa_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#endif // LFA_IOCTL_H

#ifndef LFA_C_CALLBACKS_H
#define LFA_C_CALLBACKS_H

#include <linux/fs.h>
#include <linux/types.h>

#define INPUT_REG_OFFSET 0x10
#define OUTPUT_REG_OFFSET 0x20
#define INPUT_BUFFER_SIZE (4 * sizeof(u32))
#define OUTPUT_BUFFER_SIZE (4 * sizeof(u32))

int lfa_open(struct inode *inode, struct file *file);
int lfa_release(struct inode *inode, struct file *file);
ssize_t lfa_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
ssize_t lfa_write(struct file *filp, const char __user *buf, size_t len,
                  loff_t *off);

#endif // LFA_C_CALLBACKS_H

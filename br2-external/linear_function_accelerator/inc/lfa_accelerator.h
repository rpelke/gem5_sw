#ifndef LFA_ACCELERATOR_H
#define LFA_ACCELERATOR_H

#include <linux/types.h>

// Accelerator register interface
#define A_OFFSET 0x00
#define B_OFFSET 0x04
#define INPUT_REG_OFFSET 0x10
#define OUTPUT_REG_OFFSET 0x20
#define INPUT_BUFFER_SIZE (4 * sizeof(u32))
#define OUTPUT_BUFFER_SIZE (4 * sizeof(u32))

#endif // LFA_ACCELERATOR_H

#ifndef LFA_H
#define LFA_H

// IOCTL command definitions
#define LFA_WRITE_A _IOW('a','0',int32_t*)
#define LFA_WRITE_B _IOW('a','1',int32_t*)
#define LFA_READ_A _IOR('a','2',int32_t*)
#define LFA_READ_B _IOR('a','3',int32_t*)

#endif // LFA_H

#ifndef MYPIPI_H__
#define MYPIPI_H__

#define PIPISIZE 1024
#define MYPIPE_READ   0x000000001UL // unsigned long型位图,模拟UNIX/LINUX系统的位图
#define MYPIPE_WRITE  0x000000002UL //


typedef void mypipe_t 

mypipe_t *mypipe_init(void);

int mypipe_register(mypipe_t *, int opmap);

int mypipe_unregister(mypipe_t *, int opmap);

int mypipe_read(mypipe_t *, void *buf, size_t count);  //仿照系统io

int mypipe_write(mypipe_t *, const void *buf, size_t count);

int mypipe_destroy(mypipe_t *);


#endif

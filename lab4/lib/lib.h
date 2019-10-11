#ifndef __lib_h__
#define __lib_h__

void printf(const char *format,...);
//void myprintf();
int fork();
int sleep();
int exit();
int createsem();
int locksem();
int unlocksem();
int destroysem();
#endif

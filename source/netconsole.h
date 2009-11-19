#ifndef _NCON_H_
#define _NCON_H_

#include <network.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdarg.h>

#define dbgprintf NCsend

void NCconnect(u32 ipaddress);
void NCsend(const char * msg, ...);

#endif

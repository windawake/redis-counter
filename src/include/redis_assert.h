#ifndef __REDIS_ASSERT_H__
#define __REDIS_ASSERT_H__
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h> /* for _exit() */


#define serverAssertWithInfo(_c,_o,_e) ((_e)?(void)0 : (_serverAssertWithInfo(_c,_o,#_e,__FILE__,__LINE__),_exit(1)))

#define serverAssert(_e) ((_e)?(void)0 : (_serverAssert(#_e,__FILE__,__LINE__),_exit(1)))
#define serverPanic(...) _serverPanic(__FILE__,__LINE__,__VA_ARGS__),_exit(1)

#define assert(_e) ((_e)?(void)0 : (_serverAssert(#_e,__FILE__,__LINE__),_exit(1)))
#define panic(...) _serverPanic(__FILE__,__LINE__,__VA_ARGS__),_exit(1)


void _serverAssert(const char *estr,const char *file, int line);

void _serverPanic(const char *file, int line, const char *msg, ...);

#endif

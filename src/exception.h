#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <setjmp.h>

jmp_buf jumper[256];
extern int excpos;
int exclast;
const char* excmsg;

#ifdef EXCOFF
#define THROW(exc) {}
#define THROWS(exc, str) {}
#define THROWDOWN
#define TRHOWIF(exp, exc, str) {}
#else
#ifdef EXCWARN
#define THROW(exc)                                                       \
{                                                                        \
	exclast = exc;                                                   \
	printf("WARNING: Exception ##exc## thrown\n");                   \
	longjmp(jumper[excpos--], exc);                                  \
}                          
#else
#define THROW(exc)                                                       \
{                                                                        \
	exclast = exc;                                                   \
	longjmp(jumper[excpos--], exc);                                  \
}    
#endif                      
#define THROWDOWN THROW(exclast)
#define THROWS(exc, str)                                                 \
{                                                                        \
	excmsg = str;                                                    \
        THROW(exc);                                                      \
}      
#define THROWIF(expression, exc, str)                                    \
	if (expression)							 \
		THROWS(exc, str)
#endif


#ifdef EXCOFF
#define TRY
#define CATCHIF(exc) if (0)
#define CATCH if (0)
#define TRYEND 
#else
#define TRY                                                              \
switch (setjmp(jumper[++excpos])) {                                      \
case 0:

#define CATCHIF(exc)                                                     \
excpos--;								 \
break;                                                                   \
case exc:

#define CATCH                                                            \
excpos--;                                                                \
break;                                                                   \
default:

#define TRYEND excpos--; break;} 
#endif

#endif

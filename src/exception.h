#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <setjmp.h>

jmp_buf jumper[256];
extern int excpos;
int exclast;
const char* excmsg;

#define THROW(exc) { exclast = exc; longjmp(jumper[excpos--], exc); }
#define THROWDOWN THROW(exclast)
#define THROWS(exc, str)                                                 \
{                                                                        \
	excmsg = str;                                                    \
        THROW(exc);                                                      \
}      
#define THROWIF(expression, exc, str)                                    \
	if (expression)							 \
		THROWS(exc, str)

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

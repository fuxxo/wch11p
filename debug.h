#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <linux/printk.h>

#define MIN(x,y) (((x)<(y))?(x):(y))

char *shrink(char * str);
int dump_buffer( const char * info);

#ifdef DEBUG
#define d_print(...) printk(__VA_ARGS__)
#define d_dump(var) dump_buffer( var ## _string)
#define d_create(var) char * var ## _string = NULL
#define d_stringify(type, var) var ## _string = display_ ## type ( var )
#define d_destroy(var) do {if (var ## _string != NULL) kfree(var ## _string);} while (0)
#else
#define d_print(...)
#define d_dump(var)
#define d_create(var)
#define d_stringify(type, var)
#define d_destroy(var)
#endif

#endif

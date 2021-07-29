#include "debug.h"
#include <linux/slab.h>

char *shrink(char * str) {

  char *temp = NULL;
  
  if (str == NULL) return NULL;
  temp = kmalloc( strlen(str), GFP_KERNEL );
  if (temp != NULL) {
    strcpy( temp, str);
    kfree(str);
  } else {
    temp = str;
  }
  
  return temp;
  
};

int dump_buffer( const char * info) {

  int i = 0;
  for(i=0;i<strlen(info);i++) printk("%c", info[i]);
  return 0;
}

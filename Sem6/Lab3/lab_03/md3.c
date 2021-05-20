#include <linux/init.h> 
#include <linux/module.h>
#include "md.h"

MODULE_LICENSE("GPL");

// insmod: ERROR: could not insert module md3.ko: Operation not permitted
static int __init md_init(void) 
{ 
    printk("Module md3 is loading\n");
    printk("md1_data from md1: %s\n", md1_data); 
    printk("md1_proc() from md1: %s\n", md1_proc()); 
    return -1;
} 

static void __exit md_exit(void) 
{ 
   printk("Module md3 unloaded\n"); 
} 

module_init(md_init); 
module_exit(md_exit);
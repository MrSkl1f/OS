#include <linux/init.h> 
#include <linux/module.h>
#include "md.h"

MODULE_LICENSE("GPL");

static int __init md_init(void) 
{ 
    printk("Module md2 loaded\n");
    printk("md1_data from md1: %s\n", md1_data); 
    printk("md1_proc() from md1: %s\n", md1_proc());
    // printk("md1_local() from md1: %s\n", md1_local()); // implicit declaration of function ‘md1_local’
    // printk("md1_noexport() from md1: %s\n", md1_noexport()); // ERROR: "md1_noexport" [md2.ko] undefined!
    return 0; 
} 

static void __exit md_exit(void) 
{ 
   printk("Module md2 unloaded\n"); 
} 

module_init(md_init); 
module_exit(md_exit);
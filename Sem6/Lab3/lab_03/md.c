#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init_task.h>

MODULE_LICENSE("GPL");

static int __init md_init(void)
{
    printk("Module md is loading\n");

    struct task_struct* task = &init_task;
    do
    {
        printk(KERN_INFO "%s %d | %s %d\n", task->comm, task->pid, task->parent->comm, task->parent->pid);
    } while ((task = next_task(task)) != &init_task);
    

    printk(KERN_INFO "%s %d | %s %d\n", current->comm, current->pid, current->parent->comm, current->parent->pid);
    return 0;
}

static void __exit md_exit(void)
{
    printk("Module md unloaded\n");
}

module_init(md_init);
module_exit(md_exit);


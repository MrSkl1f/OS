#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");

#define KEYBOARDIRQ 1
#define PROC_FILE_NAME "tasklet"

struct tasklet_struct my_tasklet;
static long work_state = 0;
static struct proc_dir_entry *ent;

// Bottom Half Function
void my_tasklet_function(unsigned long data)
{
    printk(KERN_INFO "+ Tasklet: state - %d, count - %d, data - %ld\n",
        my_tasklet.state, my_tasklet.count, my_tasklet.data);
    work_state = my_tasklet.state;
}

//seq file
static int my_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "+ Tasklet old state: %ld, state: %ld, count: %d, data: %s\n", 
        work_state, my_tasklet.state, my_tasklet.count, my_tasklet.data);

    return 0;
}

static int my_proc_open(struct inode *inode, struct file *file)
{
	  return single_open(file, my_proc_show, NULL);
}

static struct proc_ops tasklet_proc_ops = 
{
    .proc_open = my_proc_open,
    .proc_release = single_release,

    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
};


// Обработчик прерывания
irqreturn_t irq_handler(int irq, void *dev, struct pt_regs *regs)
{
    // Проверка, что произошло 1-е прерывание
    if(irq == KEYBOARDIRQ)
    {
        // Постановка тасклета в очередь на выполнение
        tasklet_schedule(&my_tasklet);
        printk(KERN_INFO "+ Interrupt by KEYBOARD!\n");
        return IRQ_HANDLED; // прерывание обработано
    }
    else
    {
        return IRQ_NONE; 
    }
}

// Инициализация модуля
static int __init my_module_init(void)
{
    printk(KERN_INFO "+ MODULE loaded!\n");
    
    ent = proc_create(PROC_FILE_NAME, 0666, NULL, &tasklet_proc_ops);
    if (!ent) 
    {
        return -ENOMEM;	
    }
    printk(KERN_INFO "+ Seq file created!\n");  
    
    // irq – номер прерывания, *handler –указатель на обработчик прерывания, 
    // irqflags – флаги, devname – ASCII текст, представляющий устройство, связанное с прерыванием, 
    // dev_id – используется прежде всего для разделения (shared) линии прерывания
    int ret = request_irq(KEYBOARDIRQ, (irq_handler_t)irq_handler, IRQF_SHARED,
          "my_irq_handler_tasklet", (void *)(irq_handler));

    if (ret != 0)
    {
        printk(KERN_ERR "+ KEYBOARD IRQ handler wasn't registered");
        return ret;
    }
    tasklet_init(&my_tasklet, my_tasklet_function, (void *)(irq_handler));
    printk(KERN_INFO "+ KEYBOARD IRQ handler was registered successfully");
    return ret;
}

// Выход загружаемого модуля
static void __exit my_module_exit(void)
{
    // Освобождение линии прерывания
    free_irq(KEYBOARDIRQ, (void *)(irq_handler));

    // Удаление тасклета
    tasklet_disable(&my_tasklet);
    tasklet_kill(&my_tasklet);

    if (ent) {
        remove_proc_entry(PROC_FILE_NAME, NULL);
    }

    printk(KERN_DEBUG "+ MODULE unloaded!\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
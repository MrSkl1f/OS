#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

static struct proc_dir_entry *proc_entry;
static char* str;
int ind_write, ind_read; 
#define COOKIE_POT_SIZE PAGE_SIZE

ssize_t fortune_write(struct file *file, const char __user *buffer, size_t count, loff_t *offp)
{
    printk(KERN_INFO "+ fortbuf: write\n");
    if (copy_from_user(str + ind_write, buffer, count))
        return -EFAULT;

    ind_write += count;
    if (ind_write >= COOKIE_POT_SIZE) {
        ind_write = 0;
        str[ind_write] = '\0';
    }
    return count;
}

// если пользователь читает из файла
// read вызывается много раз, чтобы понимали когда заканчивали, то считаем, что когда прочиталось 1 раз => остальные ничего
ssize_t fortune_read(struct file *file, char __user *buffer, size_t count, loff_t *offp)
{
    printk(KERN_INFO "+ fortbuf : read\n");

    if (*offp > 0)
        return 0;
    
    // записывает в строку значение
    // копирует данные из ядра в пр-во пользователя => в строке лежит прочитанное значение
    copy_to_user(buffer, str + ind_read, count);
    *offp += count;

    ind_read = ind_write;

    return count;
}

static int fortune_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "+ fortbuf: open\n");
	return 0;
}

static int fortune_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "+ fortbuf: release\n");
	return 0;
}

static struct proc_ops fort_proc_ops={
	.proc_open = fortune_open,
	.proc_release = fortune_release,

	.proc_read = fortune_read,
	.proc_write = fortune_write
};

int fortune_module_init(void)
{
    str = (char *)vmalloc(COOKIE_POT_SIZE);
    if (!str)
    {
        printk(KERN_INFO "ERROR: No memory for create str\n");
        return -ENOMEM;
    }
    memset(str, 0, COOKIE_POT_SIZE);

    // создаешь файл 0666 - доступен для всех, NULL - в директории корня, на нем определены функции из fort_proc_ops
    proc_entry = proc_create("fortbuf", 0666, NULL, &fort_proc_ops);
    if (proc_entry == NULL)
    {
        vfree(str);
        printk(KERN_INFO "ERROR: Couldn't create proc entry\n");
        // чтобы модуль не загрузился дальше
        return -ENOMEM;
    }

    // директория в корне
    struct proc_dir_entry *dir = proc_mkdir("fortbuf_dir", NULL);
    // символическая ссылка на директорию. NULL - корень
    struct proc_dir_entry *symlink = proc_symlink("fortbuf_symlink", NULL, "/proc/fortbuf_dir");
    if ((dir == NULL) || (symlink == NULL))
    {
        vfree(str);
        printk(KERN_INFO "ERROR: Couldn't create proc dir, symlink\n");
        return -ENOMEM;
    }

    ind_write = 0;
    ind_read = 0;

    printk(KERN_INFO "+ fortbuf: Loaded\n");
    return 0;
}

void fortune_module_exit(void)
{
    // удаляешь все, что создавал
    remove_proc_entry("fortbuf", NULL);
    remove_proc_entry("fortbuf_symlink", NULL);
    remove_proc_entry("fortbuf_dir", NULL);
    
    if (str)
        vfree(str);

    printk(KERN_INFO "+ fortbuf: Unloaded\n");
}

module_init(fortune_module_init);
module_exit(fortune_module_exit);
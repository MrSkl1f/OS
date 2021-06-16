#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/rtmutex.h>
#include <linux/slab.h>
#define MY_IRQ_NUM 1

MODULE_LICENSE("GPL");
// очередь работ
static struct workqueue_struct *wq;
typedef struct
{
	struct work_struct work;
	int x;
} my_work_t;

static my_work_t *work1;
static my_work_t *work2;
static int counter = 0;

struct rt_mutex mutex;
struct lock_class_key key;


void my_workqueue_function(struct work_struct *work)
{
  	rt_mutex_lock(&mutex);
  	printk(KERN_INFO "++ Mutex locked!\n");

    my_work_t *my_work = (my_work_t *) work; 

	// проверка приостановлен ли элемент work
    printk(KERN_INFO "++ Work %d: %d\n", my_work->x, work_pending(&(my_work->work)));
    if (my_work->x == 1) 
    {
        printk(KERN_INFO "++ Work 2: %d\n", work_pending(&(work2->work)));
    }
    else 
    {
        printk(KERN_INFO "++ Work 1: %d\n", work_pending(&(work1->work)));
    }

    printk(KERN_INFO "++ Workqueue counter: %d\n", ++counter);

  	int sum = 0, i = 0;
  	while (i < 77777) 
    {
    	sum += i;
    	++i;
  	}

  	printk(KERN_INFO "++ Sum: %d\n", sum);
	printk(KERN_INFO "++ Mutex unocked!\n");
    
	rt_mutex_unlock(&mutex);
}


static irqreturn_t my_irq_handler(int irq_num, void *dev)
{
  	if (irq_num == MY_IRQ_NUM) 
    {
		if (work1 && work2) 
        {
			// Добавление в очередь работ
			queue_work(wq, (struct work_struct *)work2);
	  		queue_work(wq, (struct work_struct *)work1);
		}
		return IRQ_HANDLED;
  	}
	return IRQ_NONE;
}

static int __init my_workq_init(void)
{
	// irq – номер прерывания, *handler –указатель на обработчик прерывания, 
    // irqflags – флаги, devname – ASCII текст, представляющий устройство, связанное с прерыванием, 
    // dev_id – используется прежде всего для разделения (shared) линии прерывания
  	if (request_irq(MY_IRQ_NUM, my_irq_handler, IRQF_SHARED, "my_irq_handler_workqueue", my_irq_handler)) {
		return -1;
  	}
	// Создается очередь работ 
  	wq = create_workqueue("my_workqueue");
  	if (!wq) {
		free_irq(MY_IRQ_NUM, my_irq_handler);
		return -ENOMEM;
	}
	// Инициализация работ
	work1 = (my_work_t *) kmalloc(sizeof(my_work_t), GFP_KERNEL);
	if (work1) {
		INIT_WORK((struct work_struct *)work1, my_workqueue_function);
		work1->x = 1;
	}

	work2 = (my_work_t *) kmalloc(sizeof(my_work_t), GFP_KERNEL);
	if (work2) {
		INIT_WORK((struct work_struct *)work2, my_workqueue_function);
		work2->x = 2;
	}
	// Инициализация мьютекса
	__rt_mutex_init(&mutex, NULL, &key);

	printk(KERN_INFO "++ Module workq loaded!\n");
	printk(KERN_INFO "++ Irq handler registered!\n");

  	return 0;
}


static void __exit my_workq_exit(void)
{
	// Принудительное завершение всех работ
  	flush_workqueue(wq);

  	destroy_workqueue(wq);
	// Освобождение линии irq от указанного обработчика. 
  	free_irq(MY_IRQ_NUM, my_irq_handler);
  	
  	rt_mutex_destroy(&mutex);

  	printk(KERN_DEBUG "++ Module workq unloaded!\n");
}

module_init(my_workq_init);
module_exit(my_workq_exit);
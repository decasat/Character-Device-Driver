/*********************************************************************\
*	Filename: lab1_chr.c 
* 
*	Description: character drvier demo 1
*	programmer: Sateesh Kumar G
*	date begun: 04 DEC 2017
\*********************************************************************/

#include <linux/module.h>	// for init_module()
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/slab.h>

char modname[] = "lab1_chr";


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <asm/types.h>

#define MAJORNO 300
#define MINORNO 0
#define CHAR_DEV_NAME	"skdevice"

dev_t mydev;

struct cdev *sk_cdev;
int count = 1;

/*int (*open) (struct inode *, struct file *); */
static int sk_open(struct inode *inode, struct file *filp)
{
	pr_info("open called\n");
	return 0;
}

/* int (*release) (struct inode *, struct file *); */
static int sk_close(struct inode *inode, struct file *filp)
{
	pr_info("Close called \n");
	return 0;
}

/* ssize_t (*read) (struct file *, char __user *, size_t, loff_t *); */
static ssize_t sk_read(struct file *filp, char __user * buf,
		       size_t sz, loff_t * ppos)
{
	pr_info("Read called\n");
	return 0;
}

/* ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *); */
static ssize_t sk_write(struct file *filp, const char *buf, size_t sz,
			loff_t * ppos)
{
	pr_info("Write called\n");
	return 0;
}


static struct file_operations sk_fops = {
	.owner = THIS_MODULE,
	.open = sk_open,
	.release = sk_close,
	.read = sk_read,
	.write = sk_write
};

static int __init char_sk_init(void)
{
	int ret;
	pr_info("Installing \'%s\' module\n", modname);
	mydev = MKDEV(MAJORNO, MINORNO);
	register_chrdev_region(mydev, count, CHAR_DEV_NAME);
	sk_cdev = cdev_alloc();
	cdev_init(sk_cdev, &sk_fops);

	ret = cdev_add(sk_cdev, mydev, count);
	if (ret < 0) {
		printk("Error registring device driver\n");
		return ret;
	}
	printk(KERN_INFO "Device Registered : %s \n", CHAR_DEV_NAME);

	return 0;
}

static void __exit char_sk_exit(void)
{
	cdev_del(sk_cdev);
	unregister_chrdev_region(mydev, count);
	pr_info("Removing \'%s\' module\n", modname);
}

module_init(char_sk_init);
module_exit(char_sk_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SATEESH KG ");
MODULE_DESCRIPTION("Details: character driver demo");

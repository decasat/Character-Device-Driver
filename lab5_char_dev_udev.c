/************************************************************************\
 * Description: Dynamic way of creating device file 	
 * Author : Sateesh KG
 * File name : lab5_chr_dev_udev.c					*
 * compilation: use our qc script					*
 * 		./qc lab5_chr_dev_udev					*
 *     		insmod lab5_chr_dev_udev.ko				*
 *     									*
 * Invoking driver with our test_application.c				*
 * 		./test-appl
 ************************************************************************/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <asm/current.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/atomic.h>

#define CHAR_DEV_NAME "skdevice"
#define MAX_LENGTH 2000
#define SUCCESS 0

static char *kbuf;
struct cdev *sk_cdev;
dev_t mydev;
static int  count=1;
static atomic_t device_available =ATOMIC_INIT(1);
static struct class *sk_class;

/*   int (*open) (struct inode *, struct file *);  */

static int sk_open(struct inode *inode, struct file  *file)
{
	if(! atomic_dec_and_test(&device_available) )
	{
		atomic_inc(&device_available);
		return -EBUSY; /* already open */
	}

	printk (KERN_INFO "ref=%d\n", module_refcount(THIS_MODULE));
	return SUCCESS;
}

/*
 * int (*release) (struct inode *, struct file *);
 */

static int sk_release(struct inode *inode, struct file *file)
{
	atomic_inc(&device_available);
	return SUCCESS;
}

/*
 *ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
 */
static ssize_t sk_read(struct file *file, char *ubuf, size_t usize, loff_t *ppos)
{
	int maxbytes; /* number of bytes from ppos to MAX_LENGTH */
	int bytes_to_do; /* number of bytes to read */
	int nbytes; /* number of bytes actually read */


	maxbytes = MAX_LENGTH - *ppos;
	
	if( maxbytes > usize ) bytes_to_do = usize;
	else bytes_to_do = maxbytes;
	
	if( bytes_to_do == 0 ) {
		printk("Reached end of device\n");
		return -ENOSPC; /* Causes read() to return EOF */
	}
	
	nbytes = bytes_to_do - 
		 copy_to_user( ubuf, /* to */
			       kbuf + *ppos, /* from */
			       bytes_to_do ); /* how many bytes */

	*ppos += nbytes;
	return nbytes;	
}
/*
 *ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
 */
static ssize_t sk_write(struct file *file, const char *ubuf, size_t usize, loff_t *ppos)
{
	int nbytes; /* Number of bytes written */
	int bytes_to_do; /* Number of bytes to write */
	int maxbytes; /* Maximum number of bytes that can be written */

	printk("\n%s %ld\n", ubuf, usize);
	//pr_info("writecall : %d\n " , *ppos);

	maxbytes = MAX_LENGTH - *ppos;

	if( maxbytes > usize ) bytes_to_do = usize;
	else bytes_to_do = maxbytes;

	if( bytes_to_do == 0 ) {
		printk("Reached end of device\n");
		return -ENOSPC; /* Returns EOF at write() */
	}

	nbytes = bytes_to_do -
	         copy_from_user( kbuf + *ppos, /* to */
				 ubuf, /* from */
				 bytes_to_do ); /* how many bytes */
	*ppos += nbytes;
	return nbytes;
}


static loff_t sk_lseek (struct file *file, loff_t offset, int whence)
{
    loff_t tp;
    switch (whence) {

    case 0:                 /* SEEK_SET */
            tp = offset;
            break;
    case 1:                 /* SEEK_CUR */
            tp = file->f_pos + offset;
            break;
    case 2:                 /* SEEK_END */
           tp = MAX_LENGTH + offset;
           break;
    default:
          return -EINVAL;
    }
   
    tp = tp < MAX_LENGTH ? tp : MAX_LENGTH;
    tp = tp >= 0 ? tp : 0;
    file->f_pos = tp;
    return tp;
}


static struct file_operations sk_fops = {
	.owner = THIS_MODULE,
	.read = sk_read,
	.write = sk_write,
	.open = sk_open,
	.release = sk_release,
	.llseek = sk_lseek
};

static __init int sk_init(void)
{
	int ret;
	/*int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,
	 *                         const char *name)
	 * */

	if (alloc_chrdev_region (&mydev, 0, count, CHAR_DEV_NAME) < 0) {
            printk (KERN_ERR "failed to reserve major/minor range\n");
            return -1;
	 }

        if (!(sk_cdev = cdev_alloc ())) {
            printk (KERN_ERR "cdev_alloc() failed\n");
            unregister_chrdev_region (mydev, count);
            return -1;
 	}
	cdev_init(sk_cdev,&sk_fops);

	ret=cdev_add(sk_cdev,mydev,count);
	if( ret < 0 ) {
		printk(KERN_INFO "Error registering device driver\n");
	        cdev_del (sk_cdev);
                unregister_chrdev_region (mydev, count); 	
		return -1;
	}
	
	/*  #define class_create(owner, name) */
	sk_class = class_create (THIS_MODULE, "MYVIRTUAL");

	/* 
	 *struct device *device_create(struct class *class, struct device *parent,
	 *                              dev_t devt, void *drvdata, const char *fmt, ...)
	 */
        device_create (sk_class, NULL, mydev, NULL, "%s", "skdevice");

	printk(KERN_INFO"\nDevice Registered: %s\n",CHAR_DEV_NAME);
	printk (KERN_INFO "Major number = %d, Minor number = %d\n", MAJOR(mydev),MINOR(mydev));

	kbuf =(char *)kzalloc(MAX_LENGTH,GFP_KERNEL);
	return 0;
}

static __exit void  sk_exit(void)
{
	 device_destroy (sk_class, mydev);
         class_destroy (sk_class);
	 cdev_del(sk_cdev);
	 unregister_chrdev_region(mydev,1);
	 kfree(kbuf);
	 printk(KERN_INFO "\n Driver unregistered \n");
}
module_init(sk_init);
module_exit(sk_exit);

MODULE_AUTHOR("SATEESHKG");
MODULE_LICENSE("GPL");

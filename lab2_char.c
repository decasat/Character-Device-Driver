/*********************************************************************\
 *	Filename: lab1_chr.c 
 * 
 *	Description: character drvier demo 2
 *	programmer: Sateesh Kumar G <contact@sateeshkg.com>
 *	date begun: 04 DEC 2017
\*********************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/errno.h>

#define CHAR_DEV_NAME	"skdevice"
#define MAX_LENGTH	4000	
#define SUCCESS		0

static char kbuf[MAX_LENGTH]="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static int inuse=0;
static int major;
static int balance;

/*  int (*open) (struct inode *, struct file *); */
static int sk_open(struct inode *inode, struct file *filp)
{
	if(inuse)
	{
		printk(KERN_INFO "Device busy %s\n",CHAR_DEV_NAME);
		return -EBUSY;
	}
	inuse=1;
	printk(KERN_INFO "Open invoked\n");
	balance = strlen(kbuf);
	printk("in OPEN balance = %d\n",balance);


	return SUCCESS;
}

/*  int (*release) (struct inode *, struct file *); */
static int sk_close(struct inode *inode, struct file *filp)
{
	inuse = 0;
	return SUCCESS;
}
/*   ssize_t (*read) (struct file *, char __user *, size_t, loff_t *); */
static ssize_t sk_read(struct file *filp, char __user *buf, 
			size_t sz, loff_t *fpos)
{
	int nbytes,tmp;
	printk("in READ balance = %d\n",balance);
	if(balance == 0)
		return 0;
	if( balance <= sz )
	{
		nbytes = copy_to_user(buf, kbuf + *fpos, balance);
		printk("nbytes = %d\n", nbytes);
		tmp=balance;
		balance = 0;
		*fpos =0 ;
		printk(" < balance = %d\n",balance);
		return tmp;
	} else {
	nbytes=copy_to_user(buf,kbuf +*fpos,sz);
	balance -= sz;
	*fpos += sz;
	printk(">balance = %d\n",balance);
	return sz;
	}

}

/*  ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *); */
static ssize_t sk_write(struct file *filp, const char *buf,
			size_t sz, loff_t *fpos)
{
	int nbytes=0;
	printk(KERN_INFO "\nRecevice data from app %s, nbytes=%d\n", buf, (int)sz);

//	memset(kbuf, '\0', MAX_LENGTH);
	if( sz < MAX_LENGTH)
	{
		memset(kbuf, '\0', MAX_LENGTH);
		nbytes = copy_from_user(kbuf+(*fpos), buf, (size_t) sz);
		printk("write: nbytes = %d\n", nbytes);
		balance = sz;
		return sz;
	}
	else
	{
		printk(KERN_INFO "Too large write buf, overflow\n");
		return -1;
	}

}

		
static struct file_operations sk_fops = {
	.owner =	THIS_MODULE,
	.open	=	sk_open,
	.release=	sk_close,
	.read	=	sk_read,
	.write	=	sk_write
};

static int __init char_sk_init(void)
{
	/* static inline int register_chrdev(unsigned int major, const char *name,
	 *                                  const struct file_operations *fops) */
	major = register_chrdev(0,"skdevice", &sk_fops);
	if(major < 0)
	{
		printk(KERN_ALERT "Device registration failed\n");
		return -ENOMEM;
	}
	printk(KERN_INFO "Major no: %d DevName: %s\n",major,CHAR_DEV_NAME);

	return SUCCESS;
}

static void __exit char_sk_exit(void)
{
	unregister_chrdev(major,"skdevice");
	printk(KERN_INFO "char_dev unloaded\n");
}

module_init(char_sk_init);
module_exit(char_sk_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SATEESH KG <contact@sateeshkg.com>");
MODULE_DESCRIPTION("Details: character driver demo 2");


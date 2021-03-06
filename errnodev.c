#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define MODULE_NAME "errnodev"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("flarn2006");
MODULE_DESCRIPTION("Devices like /dev/full but for every error.");
MODULE_VERSION("0.1");

static int majorNumber;

static int dev_open(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static int dev_release(struct inode *, struct file *);

static struct file_operations fops = {
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
};

static int __init errnodev_init(void)
{
	printk(KERN_INFO MODULE_NAME ": initializing\n");

	majorNumber = register_chrdev(0, MODULE_NAME, &fops);
	if (majorNumber < 0) {
		printk(KERN_ALERT MODULE_NAME ": failed to register a major number\n");
		return majorNumber;
	}
	printk(KERN_INFO MODULE_NAME ": registered major number %d\n", majorNumber);
	printk(KERN_INFO MODULE_NAME ": please note: devices must be created manually using mknod.\n");

	return 0;
}

static void __exit errnodev_exit(void)
{
	printk(KERN_INFO MODULE_NAME ": exiting\n");
	unregister_chrdev(majorNumber, MODULE_NAME);
}

static int dev_open(struct inode *inodep, struct file *filep)
{
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	return -(signed)MINOR(filep->f_inode->i_rdev);
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	return -(signed)MINOR(filep->f_inode->i_rdev);
}

static int dev_release(struct inode *inodep, struct file *filep) {
	return 0;
}

module_init(errnodev_init);
module_exit(errnodev_exit);

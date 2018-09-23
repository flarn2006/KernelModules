#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("flarn2006");
MODULE_DESCRIPTION("Character device that outputs block size used for reading");
MODULE_VERSION("0.1");

static int majorNumber;
static struct class *devClass = NULL;
static struct device *dev = NULL;

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

static int __init blocksize_init(void)
{
	printk(KERN_INFO "blocksize: initializing\n");

	majorNumber = register_chrdev(0, "blocksize", &fops);
	if (majorNumber < 0) {
		printk(KERN_ALERT "blocksize: failed to register major number\n");
		return majorNumber;
	}
	printk(KERN_INFO "blocksize: registered major number %d\n", majorNumber);

	devClass = class_create(THIS_MODULE, "blocksize");
	if (IS_ERR(devClass)) {
		unregister_chrdev(majorNumber, "blocksize");
		printk(KERN_ALERT "blocksize: failed to register device class\n");
		return PTR_ERR(devClass);
	}

	dev = device_create(devClass, NULL, MKDEV(majorNumber, 0), NULL, "blocksize");
	if (IS_ERR(dev)) {
		class_destroy(devClass);
		unregister_chrdev(majorNumber, "blocksize");
		printk(KERN_ALERT "blocksize: failed to create device\n");
		return PTR_ERR(dev);
	}

	return 0;
}

static void __exit blocksize_exit(void)
{
	printk(KERN_INFO "blocksize: exiting\n");
	device_destroy(devClass, MKDEV(majorNumber, 0));
	class_unregister(devClass);
	class_destroy(devClass);
	unregister_chrdev(majorNumber, "blocksize");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
	return 0;
}

/* Approximate log10(2^n), where n is the number of bits in size_t */
/* Add room for 3 more bytes, for LF + null terminator + to simulate rounding up */
#define OUTPUT_BUFSIZE (8 * sizeof(size_t) * 31 / 100 + 3)

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	char output[OUTPUT_BUFSIZE];
	size_t output_size = min(len, scnprintf(output, OUTPUT_BUFSIZE, "%zu\n", len));

	if (copy_to_user(buffer, output, output_size) > 0)
		return -EFAULT;
	else
		return output_size;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	return -EINVAL;
}

static int dev_release(struct inode *inodep, struct file *filep) {
	return 0;
}

module_init(blocksize_init);
module_exit(blocksize_exit);

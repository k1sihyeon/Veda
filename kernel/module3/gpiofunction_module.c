#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SiHyeonKim");
MODULE_DESCRIPTION("Rasberry Pi GPIO LED Device Module");

#define GPIO_MAJOR      200
#define GPIO_MINOR      0
#define GPIO_DEVICE     "gpioled"
#define GPIO_LED        589             // gpio18
#define GPIO_BASE       (0x1f00000000)
#define GPIO_SIZE       (64 * 1024 * 1024)

static int gpio_open(struct inode *, struct file *);
static ssize_t gpio_read(struct file *, char *, size_t, loff_t *);
static ssize_t gpio_write(struct file *, const char *, size_t, loff_t *);
static int gpio_close(struct inode *, struct file *);

static struct file_operations gpio_fops = {
	.owner = THIS_MODULE,
	.read  = gpio_read,
	.write = gpio_write,
	.open  = gpio_open,
	.release = gpio_close,
};

static char msg[BLOCK_SIZE] = {0};
struct cdev gpio_cdev;

int init_module(void) {
    dev_t devno;
    unsigned int count;
    int err;

    printk(KERN_INFO "Hello module!\n");

    try_module_get(THIS_MODULE);

    devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
    register_chrdev_region(devno, 1, GPIO_DEVICE);

    cdev_init(&gpio_cdev, &gpio_fops);

    gpio_cdev.owner = THIS_MODULE;
    count = 1;
    err = cdev_add(&gpio_cdev, devno, count);
    if (err < 0) {
        printk("Error : Device Add\n");
        return -1;
    }

    printk("'mknod /dev/%s c %d 0'\n", GPIO_DEVICE, GPIO_MAJOR);
    printk("'chmod 666 /dev/%s'\n", GPIO_DEVICE);

    gpio_request(GPIO_LED, "LED");
    gpio_direction_output(GPIO_LED, 0);

    return 0;
}

void cleanup_module(void) {
    dev_t devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
    unregister_chrdev_region(devno, 1);

    cdev_del(&gpio_cdev);

    gpio_free(GPIO_LED);
    gpio_direction_output(GPIO_LED, 0);

    module_put(THIS_MODULE);

    printk(KERN_INFO "Good-bye module!\n");
}

static int gpio_open(struct inode *inod, struct file *fil) {
    printk("GPIO Device opened(%d/%d)\n", imajor(inod), iminor(inod));

    return 0;
}

static int gpio_close(struct inode *inod, struct file *fil) {
    printk("GPIO Device closed(%d)\n", MAJOR(fil->f_path.dentry->d_inode->i_rdev));

    return 0;
}

static ssize_t gpio_read(struct file *inode, char *buff, size_t len, loff_t *off) {
    int count;

    strcat(msg, " from kernel");
    count = copy_to_user(buff, msg, strlen(msg) + 1);

    printk("GPIO Device(%d) read : %s(%d)\n", MAJOR(inode->f_path.dentry->d_inode->i_rdev), msg, count);

    return count;
}

static ssize_t gpio_write(struct file *inode, const char *buff, size_t len, loff_t *off) {
    short count;
    memset(msg, 0, BLOCK_SIZE);
    count = copy_from_user(msg, buff, len);

    gpio_set_value(GPIO_LED, (!strcmp(msg, "0")) ? 0 : 1);

    printk("GPIO Device(%d) write : %s(%ld)\n", MAJOR(inode->f_path.dentry->d_inode->i_rdev), msg, len);

    return count;
}

// 모듈은 메인 x
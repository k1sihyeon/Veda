#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/uaccess.h>

#define GPIO_MAJOR      200
#define GPIO_MINOR      0
#define GPIO_DEVICE     "gpioled"
#define GPIO_LED        18

MODULE_LICENSE("GPL");
MODULE_AUTHER("SiHyeon Kim")
MODULE_DESCRIPTION("Rasberry Pi GPIO LED Device Module")

typedef struct {
    uint32_t status;
    uint32_t ctrl;
} GPIOregs;

#define GPIO ((GPIOregs *)GPIOBase)

typedef struct
{
    uint32_t Out;
    uint32_t OE;
    uint32_t In;
    uint32_t InSync;
} rioregs;

#define rio ((rioregs *)RIOBase)
#define rioXOR ((rioregs *)(RIOBase + 0x1000 / 4))
#define rioSET ((rioregs *)(RIOBase + 0x2000 / 4))
#define rioCLR ((rioregs *)(RIOBase + 0x3000 / 4))

static char msg[BUFSIZ] = {0};
struct cdev gpio_cdev;

static int gpio_open(struct inode*, struct file *);
static ssize_t gpio_read(struct file*, char*, size_t, loff_t*);
static ssize_t gpio_write(struct file *, const char *, size_t, loff_t *);
static int gpio_close(struct inode*, struct file*);

static struct file_operation gpio_fops = {
    .owner = THIS_MODULE,
    .read = gpio_read,
    .write = gpio_write,
    .open = gpio_open,
    .release = gpio_close,
};

int init_module(void) {

    dev_t devno;
    unsigned int count;
    static void* map;
    int err;

    printk(KERN_INFO "Hello module!\n");

    devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
    register_chrdev_region(devno, 1, GPIO_DEVICE);

    cdev_init(&gpio_cdev, &gpio_fops);

    dpio_cdev.owner = THIS_MODULE;
    count = 1;
    err = cdev_add(&gpio_cdev, devno, count);
    if (err < 0) {
        printk("Error : Device Add\n");
        return -1;
    }

    printk("'mknod /dev/%s c %d 0'\n", GPIO_DEVICE, GPIO_MAJOR);
    printk("'chmod 666 /dev/%s'\n", GPIO_DEVICE);

    map = ioremap(GPIO_BASE, GPIO_SIZE);
    if(!map) {
        printk("Error : mapping GPIO memory\n");
        iounmap(map);
        return -EBUSY;
    }

    gpio = (volatile unsigned int *)map;

    
    
}

void cleanup_module(void) {

}

// 모듈은 메인 x
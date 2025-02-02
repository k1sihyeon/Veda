#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SiHyeonKim");
MODULE_DESCRIPTION("Rasberry Pi GPIO LED Device Module");

#define GPIO_MAJOR      200
#define GPIO_MINOR      0
#define GPIO_DEVICE     "gpioled"
#define GPIO_LED        18
#define GPIO_BASE       (0x1f00000000)
#define GPIO_SIZE       (64 * 1024 * 1024)

typedef struct {
    uint32_t status;
    uint32_t ctrl;
} GPIOregs;

typedef struct {
    uint32_t Out;
    uint32_t OE;
    uint32_t In;
    uint32_t InSync;
} rioregs;

#define GPIO ((GPIOregs *)GPIOBase)
#define rio ((rioregs *)RIOBase)
#define rioXOR ((rioregs *)(RIOBase + 0x1000 / 4))
#define rioSET ((rioregs *)(RIOBase + 0x2000 / 4))
#define rioCLR ((rioregs *)(RIOBase + 0x3000 / 4))

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

static volatile uint32_t *gpio;     
static volatile uint32_t *PERIBase; // PERIBase는 매핑된 메모리의 시작 주소
static volatile uint32_t *GPIOBase; // GPIO 및 RIO 베이스 주소 설정
static volatile uint32_t *RIOBase;
static uint32_t pin = GPIO_LED;     // GPIO 핀 18 사용

static char msg[BLOCK_SIZE] = {0};
struct cdev gpio_cdev;

int init_module(void) {
    dev_t devno;
    unsigned int count;
    static void* map;
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

    map = ioremap(GPIO_BASE, GPIO_SIZE);
    if (!map) {
        printk("Error : mapping GPIO memory\n");
        iounmap(map);
        return -EBUSY;
    }

    gpio = (volatile uint32_t *)map;

    PERIBase = gpio;
    GPIOBase = PERIBase + 0xD0000 / 4;
    RIOBase = PERIBase + 0xe0000 / 4;
    volatile uint32_t *PADBase = PERIBase + 0xf0000 / 4;
    volatile uint32_t *pad = PADBase + 1;

    pad[pin] = 0x10;
    rioSET->OE = 0x01 << pin;   // Output Enable?
    rioSET->Out = 0x01 << pin;

    return 0;
}

void cleanup_module(void) {
    dev_t devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
    unregister_chrdev_region(devno, 1);

    cdev_del(&gpio_cdev);

    if (gpio)
        iounmap(gpio);

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

    uint32_t fn = 5;        // 기능 번호
    GPIO[pin].ctrl = fn;    // 핀을 해당 기능으로 설정

    (!strcmp(msg, "0")) ? (rioSET->Out = 0x01 << pin) : (rioCLR->Out = 0x01 << pin);

    printk("GPIO Device(%d) write : %s(%ld)\n", MAJOR(inode->f_path.dentry->d_inode->i_rdev), msg, len);

    return count;
}

// 모듈은 메인 x
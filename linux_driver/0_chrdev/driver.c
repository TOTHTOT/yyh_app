#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "my_device"
#define CLASS_NAME "my_class"

struct my_device_data {
    dev_t dev_num;              // 存储设备号
    struct cdev cdev;           // 字符设备结构
    struct class *class;        // 设备类
};

static struct my_device_data my_device;

static int my_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "Device opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "Device released\n");
    return 0;
}

static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    char *message = "Hello from the device!\n";
    int len = strlen(message);

    if (copy_to_user(buf, message, len))
        return -EFAULT;

    *f_pos += len;
    return len;
}

static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    char message[256];

    if (copy_from_user(message, buf, count))
        return -EFAULT;

    printk(KERN_INFO "Received message from user: %s\n", message);
    *f_pos += count;
    return count;
}

static struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
};

static int __init my_init(void)
{
    // 分配设备号
    if (alloc_chrdev_region(&my_device.dev_num, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ERR "Failed to allocate device number\n");
        return -1;
    }

    // 初始化字符设备
    cdev_init(&my_device.cdev, &my_fops);
    my_device.cdev.owner = THIS_MODULE;

    // 添加字符设备到系统
    if (cdev_add(&my_device.cdev, my_device.dev_num, 1) < 0) {
        printk(KERN_ERR "Failed to add character device\n");
        unregister_chrdev_region(my_device.dev_num, 1);
        return -1;
    }

    // 创建设备类
    my_device.class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(my_device.class)) {
        printk(KERN_ERR "Failed to create device class\n");
        cdev_del(&my_device.cdev);
        unregister_chrdev_region(my_device.dev_num, 1);
        return -1;
    }

    // 创建设备节点
    if (device_create(my_device.class, NULL, my_device.dev_num, NULL, DEVICE_NAME) == NULL) {
        printk(KERN_ERR "Failed to create device node\n");
        class_destroy(my_device.class);
        cdev_del(&my_device.cdev);
        unregister_chrdev_region(my_device.dev_num, 1);
        return -1;
    }

    printk(KERN_INFO "Device driver initialized\n");
    return 0;
}

static void __exit my_exit(void)
{
    // 销毁设备节点
    device_destroy(my_device.class, my_device.dev_num);
    // 销毁设备类
    class_destroy(my_device.class);
    // 从系统中移除字符设备
    cdev_del(&my_device.cdev);
    // 释放设备号
    unregister_chrdev_region(my_device.dev_num, 1);

    printk(KERN_INFO "Device driver unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Sample Linux character device driver");

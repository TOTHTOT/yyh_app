/*
 * @Description: 
 * @Author: TOTHTOT
 * @Date: 2023-06-04 17:15:15
 * @LastEditTime: 2023-06-04 17:15:23
 * @LastEditors: TOTHTOT
 * @FilePath: /yyh_app/linux_driver/5_keyirq/irq_key.c
 */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "key_device"
#define CLASS_NAME "key_class"

struct key_device 
{
    dev_t dev_num;              // 存储设备号
    struct cdev cdev;           // 字符设备结构
    struct class *class;        // 设备类
};

static struct key_device g_key_device_st;


static int key_gpio_init(void)
{

}


static int key_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "Device opened\n");
    return 0;
}

static int key_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "Device released\n");
    return 0;
}

static ssize_t key_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    char *message = "Hello from the device!\n";
    int len = strlen(message);

    if (copy_to_user(buf, message, len))
        return -EFAULT;

    *f_pos += len;
    return len;
}

static ssize_t key_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    char message[256];

    if (copy_from_user(message, buf, count))
        return -EFAULT;

    printk(KERN_INFO "Received message from user: %s\n", message);
    *f_pos += count;
    return count;
}

static struct file_operations key_fops = {
    .owner = THIS_MODULE,
    .open = key_open,
    .release = key_release,
    .read = key_read,
    .write = key_write,
};

static int __init key_init(void)
{
    // 分配设备号
    if (alloc_chrdev_region(&g_key_device_st.dev_num, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ERR "Failed to allocate device number\n");
        return -1;
    }

    // 初始化字符设备
    cdev_init(&g_key_device_st.cdev, &key_fops);
    g_key_device_st.cdev.owner = THIS_MODULE;

    // 添加字符设备到系统
    if (cdev_add(&g_key_device_st.cdev, g_key_device_st.dev_num, 1) < 0) {
        printk(KERN_ERR "Failed to add character device\n");
        unregister_chrdev_region(g_key_device_st.dev_num, 1);
        return -1;
    }

    // 创建设备类
    g_key_device_st.class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(g_key_device_st.class)) {
        printk(KERN_ERR "Failed to create device class\n");
        cdev_del(&g_key_device_st.cdev);
        unregister_chrdev_region(g_key_device_st.dev_num, 1);
        return -1;
    }

    // 创建设备节点
    if (device_create(g_key_device_st.class, NULL, g_key_device_st.dev_num, NULL, DEVICE_NAME) == NULL) {
        printk(KERN_ERR "Failed to create device node\n");
        class_destroy(g_key_device_st.class);
        cdev_del(&g_key_device_st.cdev);
        unregister_chrdev_region(g_key_device_st.dev_num, 1);
        return -1;
    }

    printk(KERN_INFO "Device driver initialized\n");
    return 0;
}

static void __exit key_exit(void)
{
    // 销毁设备节点
    device_destroy(g_key_device_st.class, g_key_device_st.dev_num);
    // 销毁设备类
    class_destroy(g_key_device_st.class);
    // 从系统中移除字符设备
    cdev_del(&g_key_device_st.cdev);
    // 释放设备号
    unregister_chrdev_region(g_key_device_st.dev_num, 1);

    printk(KERN_INFO "Device driver unloaded\n");
}

module_init(key_init);
module_exit(key_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Sample Linux character device driver");

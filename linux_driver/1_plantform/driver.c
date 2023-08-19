/*
 * @Description: platform 总线框架
 * @Author: TOTHTOT
 * @Date: 2023-02-23 17:14:12
 * @LastEditTime: 2023-05-27 11:50:57
 * @LastEditors: TOTHTOT
 * @FilePath: \yyh_app\linux_driver\1_plantform\driver.c
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/platform_device.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define xxxDEV_CNT 1       /* 设备号长度 	*/
#define xxxDEV_NAME "name" /* 设备名字 	*/

/* xxxdev设备结构体 */
struct xxxdev_dev
{
    struct cdev cdev;         /* cdev		*/
    /* 设备其他内容 */
    dev_t devid;              /* 设备号	*/
    struct class *class;      /* 类 		*/
    struct device *device;    /* 设备		*/
    struct device_node *node; /* xxx设备节点 */
};

struct xxxdev_dev xxxdev; /* xxx设备 */

/**
 * @name: 
 * @msg: 打开设备
 * @param {inode} *inode 传递给驱动的inode
 * @param {file} *filp 设备文件，file结构体有个叫做private_data的成员变量,
 *         一般在open的时候将private_data指向设备结构体。
 * @return {*} 0 成功;其他 失败
 * @author: TOTHTOT
 * @date: 2023年5月27日11:46:14
 */
static int xxx_open(struct inode *inode, struct file *filp)
{
    /* 函数具体内容 */
    return 0;
}

/*
 * @description		: 向设备写数据
 * @param - filp 	: 设备文件，表示打开的文件描述符
 * @param - buf 	: 要写给设备写入的数据
 * @param - cnt 	: 要写入的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t xxx_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    /* 函数具体内容 */
    
    return 0;
}

/**
 * @name: xxx_read
 * @msg: 从设备度数据
 * @param {file} *filp 设备文件，表示打开的文件描述符
 * @param {char __user} *buf 从设备读取的数据
 * @param {size_t} cnt 读取的数据长度
 * @param {loff_t} *offt 相对于文件首地址的偏移
 * @return {*} 写入的字节数，如果为负值，表示写入失败
 * @author: TOTHTOT
 * @date: 2023年5月27日11:45:29
 */
static ssize_t xxx_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    /* 函数具体内容 */
    return 0;
}

/**
 * @name: xxx_release
 * @msg: 关闭/释放设备
 * @param {inode} *inode 
 * @param {file} *filp 要关闭的设备文件(文件描述符)
 * @return {*}  0 成功;其他 失败
 * @author: TOTHTOT
 * @date: 2023年5月27日11:48:05
 */
static int xxx_release(struct inode *inode, struct file *filp)
{
	/* 函数具体内容 */
	return 0;
}

/* 设备操作函数 */
static struct file_operations xxx_fops = {
    .owner = THIS_MODULE,
    .open = xxx_open,
    .write = xxx_write,
    .read = xxx_read,
    .release = xxx_release,
};

/**
 * @name: xxx_probe
 * @msg: platform的probe函数在设备与驱动匹配成功后会执行
 * @param {platform_device} *pdev
 * @return {*}
 * @author: TOTHTOT
 * @date: 
 */
static int xxx_probe(struct platform_device *pdev)
{
    int ret;

    printk("xxx driver and device was matched!\r\n");

    /* 1、设置设备号 */
    ret = alloc_chrdev_region(&xxxdev.devid, 0, xxxDEV_CNT, xxxDEV_NAME);
    if (ret < 0)
    {
        pr_err("%s Couldn't alloc_chrdev_region, ret=%d\r\n", xxxDEV_NAME, ret);
        goto free_gpio;
    }

    /* 2、初始化cdev  */
    xxxdev.cdev.owner = THIS_MODULE;
    cdev_init(&xxxdev.cdev, &xxx_fops);

    /* 3、添加一个cdev */
    ret = cdev_add(&xxxdev.cdev, xxxdev.devid, xxxDEV_CNT);
    if (ret < 0)
        goto del_unregister;

    /* 4、创建类      */
    xxxdev.class = class_create(THIS_MODULE, xxxDEV_NAME);
    if (IS_ERR(xxxdev.class))
    {
        goto del_cdev;
    }

    /* 5、创建设备 */
    xxxdev.device = device_create(xxxdev.class, NULL, xxxdev.devid, NULL, xxxDEV_NAME);
    if (IS_ERR(xxxdev.device))
    {
        goto destroy_class;
    }

    return 0;
destroy_class:
    class_destroy(xxxdev.class);
del_cdev:
    cdev_del(&xxxdev.cdev);
del_unregister:
    unregister_chrdev_region(xxxdev.devid, xxxDEV_CNT);
free_gpio:
    // gpio_free(xxxdev.gpio_xxx);
    return -EIO;
}

/**
 * @name: xxx_remove
 * @msg: 移除设备
 * @param {platform_device} *dev 
 * @return {*} 0
 * @author: TOTHTOT
 * @date: 
 */
static int xxx_remove(struct platform_device *dev)
{
    cdev_del(&xxxdev.cdev);                             /*  删除cdev */
    /* 函数具体内容 */
    return 0;
}

/* 匹配列表 */
static const struct of_device_id xxx_of_match[] = {
    {.compatible = "alientek,xxx"},
    {/* Sentinel */}};

MODULE_DEVICE_TABLE(of, xxx_of_match);

/* platform驱动结构体 */
static struct platform_driver xxx_driver = {
    .driver = {
        .name = "stm32mp1-xxx",         /* 驱动名字，用于和设备匹配 */
        .of_match_table = xxx_of_match, /* 设备树匹配表 		 */
    },
    .probe = xxx_probe,
    .remove = xxx_remove,
};

/**
 * @name: xxxdriver_init
 * @msg: 驱动模块加载函数
 * @return {*}
 * @author: TOTHTOT
 * @date: 2023-5-27 11:19:26
 */
static int __init xxxdriver_init(void)
{
    return platform_driver_register(&xxx_driver);
}

/**
 * @name: xxxdriver_exit
 * @msg: 驱动模块卸载函数
 * @return {*}
 * @author: TOTHTOT
 * @date: 
 */
static void __exit xxxdriver_exit(void)
{
    platform_driver_unregister(&xxx_driver);
}

module_init(xxxdriver_init);
module_exit(xxxdriver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TOTHTOT");
MODULE_INFO(intree, "Y");
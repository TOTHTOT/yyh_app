/*
 * @Description: linux 驱动开发之字符驱动设备, 可以当做字符设备的模板, 只需要把"hello"替换成需要的设备即可
 * @Author: TOTHTOT
 * @Date: 2023-03-01 14:35:07
 * @LastEditTime: 2023-03-02 17:25:19
 * @LastEditors: TOTHTOT
 * @FilePath: \yyh_app\linux_driver\2_hello_world\hello_device.c
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define HELLO_CHR_DEV_CNT 1
#define HELLO_CHR_DEV_NAME "hello_chr_dev"

struct hello_dev
{
    char dev_name[20];     // 设备名称
    dev_t devid;           // 设备号
    struct cdev cdev;      // cdev
    struct class *class;   // 类
    struct device *device; // 设备
    int major;             // 主设备号
    int minor;             // 次设备号
} hello_dev_st;

/**
 * @name: hello_open
 * @msg: 对应系统调用 open()
 * @param {inode} *inode 传递给驱动的inode
 * @param {file} *filp 设备文件，file结构体有个叫做private_data的成员变量一般在open的时候将private_data指向设备结构体。
 * @return {*}
 * @author: TOTHTOT
 * @date: 2023年3月1日17:36:19
 */
static int hello_open(struct inode *inode, struct file *filp)
{
    int ret = 0;

    return 0;
}

/**
 * @name: hello_read
 * @msg: 对应系统调用 read()
 * @param {file} *filp
 * @param {char __user} *buf
 * @param {size_t} cnt
 * @param {loff_t} *offt
 * @return {*}
 * @author: TOTHTOT
 * @date: 2023年3月1日17:38:51
 */
static size_t hello_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    int ret = 0;

    return 0;
}

/**
 * @name: hello_write
 * @msg: 对应系统调用 write()
 * @param {file} *filp
 * @param {char __user} *buf
 * @param {loff_t} *offt
 * @return {*}
 * @author: TOTHTOT
 * @date: 2023年3月1日17:38:51
 */
static size_t hello_write(struct file *filp, char __user *buf, loff_t *offt)
{
    int ret = 0;

    return 0;
}

/**
 * @name: hello_release
 * @msg: 对应系统调用 close()
 * @param {inode} *inode
 * @param {file} *filp
 * @return {*}
 * @author: TOTHTOT
 * @date: 2023年3月1日17:41:08
 */
static int hello_release(struct inode *inode, struct file *filp)
{
    int ret = 0;

    return 0;
}

/* 设备操作变量, 包含了设备操作函数合集, 填充 fops */
static struct file_operation hello_fops = {
    .owner = THIS_MODULE,
    .open = hello_open,
    .read = hello_read,
    .write = hello_write,
    .release = hello_release,
};

/**
 * @name: hello_init
 * @msg: 驱动入口函数, insmod 时调用, 初始化 hello 字符设备
 * @return {*} 0
 * @author: TOTHTOT
 * @date: 2023年3月1日17:09:37
 */
static int __init hello_init(void)
{
    int ret;

    printk("hello init\n");

    /* 获取设备号 */
    ret = alloc_chrdev_region(&hello_dev_st.devid, 0, HELLO_CHR_DEV_CNT, HELLO_CHR_DEV_NAME);
    if (ret < 0)
    {
        printk("error:register_chrdev err\n");
        return -1;
    }
    hello_dev_st.major = MAJOR(hello_dev_st.devid);
    hello_dev_st.minor = MINOR(hello_dev_st.devid);

    printk("device id: %d, %d\n", hello_dev_st.major, hello_dev_st.minor);

    /* 初始化cdev, 将 cdev 和 fops 关联 */
    hello_dev_st.cdev.owner = THIS_MODULE;
    cdev_init(&hello_dev_st.cdev, &hello_fops);

    /* 添加一个 cdev, 将 cdev 和 devid 关联 */
    ret = cdev_add(&hello_dev_st.cdev, &hello_dev_st.devid, HELLO_CHR_DEV_CNT);
    if (ret < 0)
    {
        unregister_chrdev_region(hello_dev_st.devid, HELLO_CHR_DEV_CNT);
        return -1;
    }

    /* 创建类 class */
    hello_dev_st.class = class_create(THIS_MODULE, HELLO_CHR_DEV_NAME);
    if (IS_ERR(hello_dev_st.class))
    {
        class_destroy(newchrled.class);
    }
    /* 注册字符设备驱动 设备号:HELLO_CHR_DEV_NUM, 名称: HELLO_CHR_DEV_NAME, 设备操作合集 */
    ret = register_chrdev(hello_dev.dev_num, hello_dev.dev_name, &hello_fops);

    return 0;
}

/**
 * @name: hello_exit
 * @msg: 驱动出口函数, rmmod 时调用, 退出 hello 字符设备
 * @return {*} 0
 * @author: TOTHTOT
 * @date: 2023年3月1日17:10:20
 */
static int __exit hello_exit(void)
{
    printk("hello exit\n");
    /* 注销字符设备驱动, 第一个参数:设备号, 第二个:设备名称 */
    unregister_chrdev(hello_st.dev_num, hello_st.dev_name);
    return 0;
}

MODULE_LICENSE("GPL");   // 添加模块 LICENSE 信息
MODULE_AUTHOR("TOTHTOT") // 添加模块作者信息
module_init(hello_init);
module_exit(hello_exit);
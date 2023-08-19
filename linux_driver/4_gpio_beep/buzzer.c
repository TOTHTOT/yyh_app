/*
 * @Description: gpio 方式驱动有源蜂鸣器
 * @Author: TOTHTOT
 * @Date: 2023-03-01 14:24:06
 * @LastEditTime: 2023-08-19 20:55:59
 * @LastEditors: TOTHTOT
 * @FilePath: \yyh_app\linux_driver\4_gpio_beep\buzzer.c
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
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define GPIOBUZZER_DEV_NUM 1
#define GPIOBUZZER_NAME "buzzer" // 显示到 /dev下的设备名字
#define BUZZER_ON 0
#define BUZZER_OFF 1

typedef struct
{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *nd;
    int buzzer_gpio_num; // 申请到的GPIO编号
} gpio_buzzer_dev_t;

gpio_buzzer_dev_t g_buzzer_dev_st;

/**
 * @name: buzzer_open
 * @msg: open函数
 * @param {inode} *inode
 * @param {file} *p_file
 * @return {*}
 * @author: TOTHTOT
 * @date:2023年8月19日
 */
static int buzzer_open(struct inode *inode, struct file *p_file)
{
    p_file->private_data = &g_buzzer_dev_st;
    return 0;
}

static ssize_t buzzer_read(struct file *p_file, char __user *buf, size_t count, loff_t *offt)
{
#define BUZZER_STATE_BUF_SIZE 1
    ssize_t ret = 0;
    gpio_buzzer_dev_t *buzzer_dev = p_file->private_data;
    int buzzer_state = (gpio_get_value(buzzer_dev->buzzer_gpio_num) == BUZZER_ON) ? 1 : 0;  // 亮就返回1

    if (count > BUZZER_STATE_BUF_SIZE)
        count = BUZZER_STATE_BUF_SIZE;

    // printk("driver buzzer state %d\n", buzzer_state);
    if (copy_to_user(buf, (char *)&buzzer_state, count) == 0)
    {
        ret = count;
    }
    else
    {
        ret = -EFAULT;
    }

    return ret;
}

/**
 * @name: buzzer_write
 * @msg: 写入buzzer状态
 * @param {file} *p_file
 * @param {char __user} *buf
 * @param {size_t} count
 * @param {loff_t} *offt
 * @return {0} 成功
 * @return {其他} 失败
 * @author: TOTHTOT
 * @date: 日期
 */
static int32_t buzzer_write(struct file *p_file, const char __user *buf, size_t count, loff_t *offt)
{
    int32_t ret = 0;
    uint8_t data_buf[1];

    gpio_buzzer_dev_t *buzzer_dev = p_file->private_data;

    ret = copy_from_user(data_buf, buf, count);
    if (ret < 0)
    {
        printk("copy_from_user faibuzzer %d\n", ret);
        return -EFAULT;
    }

    switch (data_buf[0])
    {
    case BUZZER_OFF:
        gpio_set_value(buzzer_dev->buzzer_gpio_num, 1);
        break;
    case BUZZER_ON:
        gpio_set_value(buzzer_dev->buzzer_gpio_num, 0);
        break;
    default:
        break;
    }

    return 0;
}

static int32_t buzzer_release(struct inode *p_inode, struct file *p_file)
{
    p_file->private_data = NULL;
    printk("buzzer released\n");
    return 0;
}

static struct file_operations g_buzzer_fops_st = {
    .owner = THIS_MODULE,
    .open = buzzer_open,
    .read = buzzer_read,
    .write = buzzer_write,
    .release = buzzer_release,
};

/**
 * @name: buzzer_init
 * @msg: 模块入口函数
 * @return {*}
 * @author: TOTHTOT
 * @date: 2023年8月19日
 */
static int32_t __init buzzer_init(void)
{
    int32_t ret = 0;
    const char *str;

    /* 从设备树中获取buzzer的IO口信息 */
    g_buzzer_dev_st.nd = of_find_node_by_path("/buzzer_active_gpio");
    if (g_buzzer_dev_st.nd == NULL)
    {
        printk("gpiobuzzer node not find\n");
        return -EINVAL;
    }
    ret = of_property_read_string(g_buzzer_dev_st.nd, "status", &str);
    if (ret < 0)
    {
        printk("read status error\n");
        return -EINVAL;
    }
    if (strcmp(str, "okay") != 0)
        return -EINVAL;
    ret = of_property_read_string(g_buzzer_dev_st.nd, "compatible", &str);
    if (ret < 0)
    {
        printk("read status error\n");
        return -EINVAL;
    }
    if (strcmp(str, "tothtot,buzzer_act") != 0)
    {
        printk("compatible compare error\n");
        return -EINVAL;
    }

    // 获取IO口属性
    g_buzzer_dev_st.buzzer_gpio_num = of_get_named_gpio(g_buzzer_dev_st.nd, "gpio", 0);
    if (g_buzzer_dev_st.buzzer_gpio_num < 0)
    {
        printk("get buzzer gpio faibuzzer\n");
        return -EINVAL;
    }

    printk("gpio_buzzer = %d\n", g_buzzer_dev_st.buzzer_gpio_num);

    ret = gpio_request(g_buzzer_dev_st.buzzer_gpio_num, "BUZZER_GPIO");
    if (ret < 0)
    {
        printk("gpio_request faibuzzer\n");
        return ret;
    }
    // 设置为输出,且为高电平
    ret = gpio_direction_output(g_buzzer_dev_st.buzzer_gpio_num, 1);
    if (ret < 0)
    {
        printk("set buzzer gpio direction error\n");
        return -EINVAL;
    }

    /* 注册字符设备驱动 */
    ret = alloc_chrdev_region(&g_buzzer_dev_st.devid, 0, GPIOBUZZER_DEV_NUM, GPIOBUZZER_NAME);
    if (ret < 0)
    {
        pr_err("%s get alloc_chrdev_region() error, ret = %d\n", GPIOBUZZER_NAME, ret);
        goto free_gpio;
    }
    // 设置设备号
    g_buzzer_dev_st.major = MAJOR(g_buzzer_dev_st.devid);
    g_buzzer_dev_st.minor = MINOR(g_buzzer_dev_st.devid);
    // 初始化 cdev
    g_buzzer_dev_st.cdev.owner = THIS_MODULE;
    cdev_init(&g_buzzer_dev_st.cdev, &g_buzzer_fops_st);
    if (cdev_add(&g_buzzer_dev_st.cdev, g_buzzer_dev_st.devid, GPIOBUZZER_DEV_NUM) < 0)
        goto del_unregister;

    g_buzzer_dev_st.class = class_create(THIS_MODULE, GPIOBUZZER_NAME);
    if (IS_ERR(g_buzzer_dev_st.class))
        goto del_cdev;
    // 创建设备
    g_buzzer_dev_st.device = device_create(g_buzzer_dev_st.class, NULL, g_buzzer_dev_st.devid, NULL, GPIOBUZZER_NAME);
    if (IS_ERR(g_buzzer_dev_st.device))
        goto del_class;
    return 0;

del_class:
    class_destroy(g_buzzer_dev_st.class);
del_cdev:
    cdev_del(&g_buzzer_dev_st.cdev);
del_unregister:
    unregister_chrdev_region(g_buzzer_dev_st.devid, GPIOBUZZER_DEV_NUM);
free_gpio:
    gpio_free(g_buzzer_dev_st.buzzer_gpio_num);

    return -EIO;
}

/**
 * @name: buzzer_exit
 * @msg: 模块出口函数
 * @return {*}
 * @author: TOTHTOT
 * @date: 2023年8月19日
 */
static void __exit buzzer_exit(void)
{
    device_destroy(g_buzzer_dev_st.class, g_buzzer_dev_st.devid);
    class_destroy(g_buzzer_dev_st.class);
    cdev_del(&g_buzzer_dev_st.cdev);
    unregister_chrdev_region(g_buzzer_dev_st.devid, GPIOBUZZER_DEV_NUM);
    gpio_free(g_buzzer_dev_st.buzzer_gpio_num);
}

module_init(buzzer_init);
module_exit(buzzer_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TOTHTOT");
MODULE_INFO(intree, "Y");
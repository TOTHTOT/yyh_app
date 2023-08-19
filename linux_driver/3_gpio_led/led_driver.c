/*
 * @Description: gpio 方式驱动led
 * @Author: TOTHTOT
 * @Date: 2023-03-01 14:24:06
 * @LastEditTime: 2023-08-19 20:02:08
 * @LastEditors: TOTHTOT
 * @FilePath: \yyh_app\linux_driver\3_gpio_led\led_driver.c
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

#define GPIOLED_DEV_NUM 1
#define GPIOLED_NAME "yyh_gpioled" // 显示到 /dev下的设备名字
#define LED_ON 0
#define LED_OFF 1

typedef struct
{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *nd;
    int led_gpio_num; // 申请到的GPIO编号
} gpio_dev_t;

gpio_dev_t g_led_dev_st;

/**
 * @name: led_open
 * @msg: open函数
 * @param {inode} *inode
 * @param {file} *p_file
 * @return {*}
 * @author: TOTHTOT
 * @date:2023年8月19日
 */
static int led_open(struct inode *inode, struct file *p_file)
{
    p_file->private_data = &g_led_dev_st;
    return 0;
}

static ssize_t led_read(struct file *p_file, char __user *buf, size_t count, loff_t *offt)
{
#define LED_STATE_BUF_SIZE 1
    ssize_t ret = 0;
    gpio_dev_t *led_dev = p_file->private_data;
    int led_state = (gpio_get_value(led_dev->led_gpio_num) == LED_ON) ? 1 : 0; // 亮就返回1

    if (count > LED_STATE_BUF_SIZE)
        count = LED_STATE_BUF_SIZE;

    // printk("driver led state %d\n", led_state);
    if (copy_to_user(buf, (char *)&led_state, count) == 0)
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
 * @name: led_write
 * @msg: 写入led状态
 * @param {file} *p_file
 * @param {char __user} *buf
 * @param {size_t} count
 * @param {loff_t} *offt
 * @return {0} 成功
 * @return {其他} 失败
 * @author: TOTHTOT
 * @date: 日期
 */
static int32_t led_write(struct file *p_file, const char __user *buf, size_t count, loff_t *offt)
{
    int32_t ret = 0;
    uint8_t data_buf[1];

    gpio_dev_t *led_dev = p_file->private_data;

    ret = copy_from_user(data_buf, buf, count);
    if (ret < 0)
    {
        printk("copy_from_user failed %d\n", ret);
        return -EFAULT;
    }
    // printk("write = %d\n", data_buf[0]);
    switch (data_buf[0])
    {
    case LED_OFF:
        gpio_set_value(led_dev->led_gpio_num, LED_OFF);
        break;
    case LED_ON:
        gpio_set_value(led_dev->led_gpio_num, LED_ON);
        break;
    default:
        break;
    }

    return 0;
}

static int32_t led_release(struct inode *p_inode, struct file *p_file)
{
    gpio_dev_t *led_dev = p_file->private_data;
    
    gpio_set_value(led_dev->led_gpio_num, LED_OFF);

    p_file->private_data = NULL;
    printk("led released\n");
    return 0;
}

static struct file_operations g_led_fops_st = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
};

/**
 * @name: led_init
 * @msg: 模块入口函数
 * @return {*}
 * @author: TOTHTOT
 * @date: 2023年8月19日
 */
static int32_t __init led_init(void)
{
    int32_t ret = 0;
    const char *str;

    /* 从设备树中获取led的IO口信息 */
    g_led_dev_st.nd = of_find_node_by_path("/gpio_led");
    if (g_led_dev_st.nd == NULL)
    {
        printk("gpioled node not find\n");
        return -EINVAL;
    }
    ret = of_property_read_string(g_led_dev_st.nd, "status", &str);
    if (ret < 0)
    {
        printk("read status error\n");
        return -EINVAL;
    }
    if (strcmp(str, "okay") != 0)
        return -EINVAL;
    ret = of_property_read_string(g_led_dev_st.nd, "compatible", &str);
    if (ret < 0)
    {
        printk("read status error\n");
        return -EINVAL;
    }
    if (strcmp(str, "tothtot,led") != 0)
        return -EINVAL;

    // 获取IO口属性
    g_led_dev_st.led_gpio_num = of_get_named_gpio(g_led_dev_st.nd, "gpio-led", 0);
    if (g_led_dev_st.led_gpio_num < 0)
    {
        printk("get gpio_led failed\n");
        return -EINVAL;
    }

    printk("gpio_led = %d\n", g_led_dev_st.led_gpio_num);

    ret = gpio_request(g_led_dev_st.led_gpio_num, "LED_GPIO");
    if (ret < 0)
    {
        // printk(KERN_ERR, "gpio_request failed\n");
        return ret;
    }
    // 设置为输出,且为物理电平为高电平
    ret = gpio_direction_output(g_led_dev_st.led_gpio_num, 1);
    if (ret < 0)
    {
        printk("set led gpio direction error\n");
        return -EINVAL;
    }

    
    /* 注册字符设备驱动 */
    ret = alloc_chrdev_region(&g_led_dev_st.devid, 0, GPIOLED_DEV_NUM, GPIOLED_NAME);
    if (ret < 0)
    {
        pr_err("%s get alloc_chrdev_region() error, ret = %d\n", GPIOLED_NAME, ret);
        goto free_gpio;
    }
    // 设置设备号
    g_led_dev_st.major = MAJOR(g_led_dev_st.devid);
    g_led_dev_st.minor = MINOR(g_led_dev_st.devid);
    // 初始化 cdev
    g_led_dev_st.cdev.owner = THIS_MODULE;
    cdev_init(&g_led_dev_st.cdev, &g_led_fops_st);
    if (cdev_add(&g_led_dev_st.cdev, g_led_dev_st.devid, GPIOLED_DEV_NUM) < 0)
        goto del_unregister;

    g_led_dev_st.class = class_create(THIS_MODULE, GPIOLED_NAME);
    if (IS_ERR(g_led_dev_st.class))
        goto del_cdev;
    // 创建设备
    g_led_dev_st.device = device_create(g_led_dev_st.class, NULL, g_led_dev_st.devid, NULL, GPIOLED_NAME);
    if (IS_ERR(g_led_dev_st.device))
        goto del_class;
    return 0;

del_class:
    class_destroy(g_led_dev_st.class);
del_cdev:
    cdev_del(&g_led_dev_st.cdev);
del_unregister:
    unregister_chrdev_region(g_led_dev_st.devid, GPIOLED_DEV_NUM);
free_gpio:
    gpio_free(g_led_dev_st.led_gpio_num);

    return -EIO;
}

/**
 * @name: led_exit
 * @msg: 模块出口函数
 * @return {*}
 * @author: TOTHTOT
 * @date: 2023年8月19日
 */
static void __exit led_exit(void)
{
    printk("led exit\n");
    gpio_set_value(g_led_dev_st.led_gpio_num, LED_OFF);
    device_destroy(g_led_dev_st.class, g_led_dev_st.devid);
    class_destroy(g_led_dev_st.class);
    cdev_del(&g_led_dev_st.cdev);
    unregister_chrdev_region(g_led_dev_st.devid, GPIOLED_DEV_NUM);
    gpio_free(g_led_dev_st.led_gpio_num);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TOTHTOT");
MODULE_INFO(intree, "Y");
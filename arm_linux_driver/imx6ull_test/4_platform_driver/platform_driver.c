/* 必须包含的两个头文件 */
#include <linux/module.h>
#include <linux/init.h>
/* 设备号,文件操作 */
#include <linux/fs.h>
/* 杂项设备头文件 */
#include <linux/miscdevice.h>
/* 平台设备头文件 */
#include <linux/platform_device.h>
/* 设备节点创建 */
#include <linux/device.h>
/* ioremap */
#include <asm/io.h>
/* 内存拷贝 */
#include <asm/uaccess.h>
/* gpio子系统 */
#include <linux/of_gpio.h>

#if 1
    #define LOG_TRACE()  printk("\nkdebug: [file]:%s [func]:%s [line]:%d\n", __FILE__, __FUNCTION__, __LINE__)
#else
    #define LOG_TRACE() 
#endif

#define DEVICE_NUM          1

struct dev_info {
    int gpio_beep;              /* gpio编号 */
	struct device_node *node;	/* 设备树节点 */
};

struct dev_info g_dev_list[DEVICE_NUM];

static void deinit_devs(struct dev_info devs[])
{
    gpio_free(devs[0].gpio_beep);
}

static int init_devs(struct dev_info devs[])
{
    /* 根据设备树 获取节点信息 和 GPIO 信息 */
    devs[0].node = of_find_node_by_path("/gpio_beep");
    devs[0].gpio_beep = of_get_named_gpio(devs[0].node, "beep-gpios", 0);
    /* 申请这个GPIO */
    gpio_request(devs[0].gpio_beep, "beep0");
    /* IO设置为输出,默认 输出非有效电平	*/
    gpio_direction_output(devs[0].gpio_beep, 0);

    return 0;
}

static 	int misc_open(struct inode *node, struct file *file)
{
    LOG_TRACE();
    file->private_data = &g_dev_list[0]; // open 的时候 给 file->private_data 赋值
    return 0;
}
static 	ssize_t misc_read(struct file *file, char __user *buf, size_t n, loff_t *offset)
{
    char data;
    unsigned long ret;
    struct dev_info *dev = file->private_data;
    LOG_TRACE();
    data = gpio_get_value(dev->gpio_beep); // GPIO子系统 方式读取电平
    ret = copy_to_user(buf, &data, sizeof(data));
    return ret;
}
static ssize_t misc_write(struct file *file, const char __user *buf, size_t n, loff_t *offset)
{
    char data;
    unsigned long ret;
    struct dev_info *dev = file->private_data;
    LOG_TRACE();
    ret = copy_from_user(&data, buf, sizeof(data));
    gpio_set_value(dev->gpio_beep, !!data); // GPIO子系统 方式输出电平
    return ret;
}
static int misc_release(struct inode *node, struct file *file)
{
    struct dev_info *dev = file->private_data;
    LOG_TRACE();
    gpio_set_value(dev->gpio_beep, 0);
    file->private_data = NULL;
    return 0;
}

/* 文件操作方法集 */
static const struct file_operations op = {
    .owner = THIS_MODULE,
    .read = misc_read,
    .write = misc_write,
    .open = misc_open,
    .release = misc_release,
};
/* 杂项设备 */
struct miscdevice my_misc_dev = {
    .minor = MISC_DYNAMIC_MINOR, // 次设备号，赋值为宏 MISC_DYNAMIC_MINOR ，表示系统自动分配
    .name = "my_beep",	// 设备名称
    .fops = &op,
};

// 与设备匹配之后会执行，必须要实现
static int platform_probe(struct platform_device *dev)
{
    int ret;
    LOG_TRACE();
    init_devs(g_dev_list);
    ret = misc_register(&my_misc_dev);
    return ret;
}
// 移除设备时会执行
static int platform_remove(struct platform_device *dev)
{
    LOG_TRACE();
    misc_deregister(&my_misc_dev);
    deinit_devs(g_dev_list);
    return 0;
}

// 关闭设备时调用
static void platform_shutdown(struct platform_device *dev)
{
    LOG_TRACE();
}
// 进入睡眠时调用
static int platform_suspend(struct platform_device *dev, pm_message_t state)
{
    LOG_TRACE();
    return 0;
}
// 从睡眠模式恢复时调用
static int platform_resume(struct platform_device *dev)
{
    LOG_TRACE();
    return 0;
}

// 与设备树节点 compatible 匹配
struct of_device_id of_id_table[] = {
    {.compatible = "my_beep_test"},
    {}, // 哨兵
};

// 优先匹配id_table名字
static const struct platform_device_id id_table[] = {
    {.name = "gpio_beep", .driver_data = 0},
    {}, // 哨兵
};

/* 平台总线驱动 */
struct platform_driver my_deiver = {\
    .driver = {
        .name = "gpio_beep", // 必须要有，否则可能引发错误
        .of_match_table = of_id_table,
    },
    .probe = platform_probe,
    .remove = platform_remove,
    .shutdown = platform_shutdown,
    .suspend = platform_suspend,
    .resume = platform_resume,
    .id_table = id_table
};

// 加载函数
static int platform_driver_init(void)
{
    LOG_TRACE();
    /* 注册平台驱动 */
    return platform_driver_register(&my_deiver);
}
// 卸载函数
static void platform_driver_exit(void)
{
    LOG_TRACE();
    /* 注销平台驱动 */
    platform_driver_unregister(&my_deiver);
}

/**
 * 平台总线模型使用之后，可以把 驱动(driver) 与 硬件设备(device) 分离开来，两者可以分别独立加载。
 * 编写驱动部分 platform_driver 时不必再关心硬件寄存器地址等信息。
 * 硬件地址寄存器 platform_device 等信息可以通过 驱动模块 或者 设备树 来提供。
 * 由于实现了 driver 与 device 分离，所以 一份 driver 代码可以匹配多个 device。
 * 当 driver 与 device 匹配的时候，执行 probe 函数，driver 从中获取 device 的硬件信息。
 * 原本的 字符设备驱动注册/注销过程 套在 平台总线驱动的probe/remove 过程中 。
*/

/* 必须使用宏指定 加载函数、卸载函数、GPL声明 */
module_init(platform_driver_init);
module_exit(platform_driver_exit);
MODULE_LICENSE("GPL");
// 可选的 作者、版本等信息
MODULE_AUTHOR("pdg");
MODULE_VERSION("v1.0");
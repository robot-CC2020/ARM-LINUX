/* 必须包含的两个头文件 */
#include <linux/module.h>
#include <linux/init.h>
/* 设备号,文件操作 */
#include <linux/fs.h>
/* 字符设备头文件 */
#include <linux/cdev.h>
/* 设备节点创建 */
#include <linux/device.h>
/* ioremap */
#include <asm/io.h>
/* 内存拷贝 */
#include <asm/uaccess.h>

#define LOG_TRACE()  printk("func=%s line=%d\n", __FUNCTION__, __LINE__)


/* BEEP  SNVS_TAMPER1/GPIO5_IO01  low=on */
#define CCM_CCGR5     0x20C407C   // GPIO5时钟 寄存器
#define IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER1     0x229000C   // SNVS_TAMPER1 引脚复用

#define GPIO5_BASE      0x020AC000
#define GPIO5_DR    (GPIO5_BASE + 0)        // GPIO5 输出控制
#define GPIO5_GDIR  (GPIO5_BASE + 4)        // GPIO5 方向设置 [0:输入 1:输出]
#define GPIO5_PSR   (GPIO5_BASE + 8)        // GPIO5 读取状态
#define IO01_BIT        0b10                // IO01 对应的位

#define IO_DATA_ON         1                // 应用层数据 1 表示 设备启动
#define IO_DATA_OFF        0                // 应用层数据 0 表示 设备停止

#define GPIO_DEV_NAME   "my_beep"
#define DEVICE_NUM          1

volatile static u32 __iomem *g_clockReg;
volatile static u32 __iomem *g_muxReg;
volatile static u32 __iomem *g_dataReg;
volatile static u32 __iomem *g_dirReg;
volatile static u32 __iomem *g_statReg;

static dev_t g_dev_num; // 设备号

struct priv_data {
    struct cdev cdev;           // 字符设备
    struct class *class;        // 类
    struct device *dev_node;    // 设备节点
};

struct priv_data g_data;

static void dev_register_remap(void)
{
    g_clockReg = ioremap(CCM_CCGR5, 4);
    g_muxReg = ioremap(IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER1, 4);
    g_dataReg = ioremap(GPIO5_DR, 4);
    g_dirReg = ioremap(GPIO5_GDIR, 4);
    g_statReg = ioremap(GPIO5_PSR, 4);
}

static void dev_register_unremap(void)
{
	iounmap(g_clockReg);
	iounmap(g_muxReg);
	iounmap(g_dataReg);
	iounmap(g_dirReg);
	iounmap(g_statReg);
}

static void dev_config_gpio(void)
{
    *g_muxReg &= ~0x0F;  // 清除复用设置
    *g_muxReg |= 0b101;  // 设置复用 为GPIO5
    *g_dirReg |= IO01_BIT; // 置位，设置为输出
    *g_dataReg |= IO01_BIT; // 置位，输出高电平
}

static char dev_read_data(void)
{
    char data = ((*g_statReg) & IO01_BIT) ? IO_DATA_OFF : IO_DATA_ON;
    return 0;
}

static void dev_write_data(char data)
{
    if (data == IO_DATA_OFF) {
        // 关闭设备
        *g_dataReg |= IO01_BIT; // 置位，输出高电平
    } else {
        // 开启设备
        *g_dataReg &= ~IO01_BIT; // 清除，输出低电平
    }
}

static 	int gpio_open(struct inode *node, struct file *file)
{
    printk("gpio_open\n");
    return 0;
}
static 	ssize_t gpio_read(struct file *file, char __user *buf, size_t n, loff_t *offset)
{   
    char data;
    unsigned long ret;
    data = dev_read_data();
    ret = copy_to_user(buf, &data, sizeof(data));
    return ret;
}
static ssize_t gpio_write(struct file *file, const char __user *buf, size_t n, loff_t *offset)
{
    char data;
    unsigned long ret;
    ret = copy_from_user(&data, buf, sizeof(data));
    dev_write_data(data);
    return ret;
}
static int gpio_release(struct inode *node, struct file *file)
{
    dev_write_data(IO_DATA_OFF);
    printk("gpio_release\n");
    return 0;
}
static const struct file_operations op = {
    .owner = THIS_MODULE,
    .read = gpio_read,
    .write = gpio_write,
    .open = gpio_open,
    .release = gpio_release,
};

// 加载函数
static int gpio_init(void)
{
    int ret;
    printk("gpio_init\n");
    dev_register_remap();
    dev_config_gpio();
    LOG_TRACE();
    alloc_chrdev_region(&g_dev_num, 0, DEVICE_NUM, GPIO_DEV_NAME); // 分配设备号并注册， 次设备号为1个
    LOG_TRACE();
    cdev_init(&g_data.cdev, &op); // 初始化 字符设备，绑定 file_operations
    LOG_TRACE();
    ret = cdev_add(&g_data.cdev, g_dev_num, DEVICE_NUM); // 把字符设备添加到系统中
    LOG_TRACE();

    g_data.class = class_create(THIS_MODULE, "class_beep");
    g_data.dev_node = device_create(g_data.class, NULL, g_dev_num, NULL, "dev_beep");

    return ret;
}
// 卸载函数
static void gpio_exit(void)
{
    printk("gpio_exit\n");

    device_destroy(g_data.class, g_dev_num);
    LOG_TRACE();
    class_destroy(g_data.class);
    LOG_TRACE();

    cdev_del(&g_data.cdev); // 销毁字符设备
    memset(&g_data, 0, sizeof(g_data));
    LOG_TRACE();
    unregister_chrdev_region(g_dev_num, DEVICE_NUM); // 反注册
    g_dev_num = 0;
    LOG_TRACE();

    dev_register_unremap(); // 释放资源
}

/**
 * 最简单的字符设备驱动
 * 需要 申请设备号，注册文件操作方法集，创建 类 和 设备节点
*/

/* 必须使用宏指定 加载函数、卸载函数、GPL声明 */
module_init(gpio_init);
module_exit(gpio_exit);
MODULE_LICENSE("GPL");
// 可选的 作者、版本等信息
MODULE_AUTHOR("pdg");
MODULE_VERSION("v1.0");
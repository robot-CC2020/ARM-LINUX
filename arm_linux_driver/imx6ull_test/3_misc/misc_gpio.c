/* 必须包含的两个头文件 */
#include <linux/module.h>
#include <linux/init.h>
/* 设备号,文件操作 */
#include <linux/fs.h>
/* 杂项设备头文件 */
#include <linux/miscdevice.h>
/* 设备节点创建 */
#include <linux/device.h>
/* ioremap */
#include <asm/io.h>
/* 内存拷贝 */
#include <asm/uaccess.h>


#if 1
    #define LOG_TRACE()  printk("\nkdebug: [file]:%s [func]:%s [line]:%d\n", __FILE__, __FUNCTION__, __LINE__)
#else
    #define LOG_TRACE() 
#endif

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

#define DEVICE_NUM          1

struct gpio_register {
    volatile u32 __iomem *clockReg;
    volatile u32 __iomem *muxReg;
    volatile u32 __iomem *dataReg;
    volatile u32 __iomem *dirReg;
    volatile u32 __iomem *statReg;
    u32 bit;
};

struct dev_info {
    int number;
    struct gpio_register reg;   // GPIO 相关的寄存器
};

struct dev_info g_dev_list[DEVICE_NUM];

static void deinit_devs(struct dev_info devs[])
{
    struct dev_info *dev = &devs[0];
	iounmap(dev->reg.clockReg);
	iounmap(dev->reg.muxReg);
	iounmap(dev->reg.dataReg);
	iounmap(dev->reg.dirReg);
	iounmap(dev->reg.statReg);
    memset(&dev->reg, 0, sizeof(dev->reg));
}

static void init_devs(struct dev_info devs[])
{
    struct dev_info *dev = &devs[0];
    dev->reg.clockReg = ioremap(CCM_CCGR5, 4);
    dev->reg.muxReg = ioremap(IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER1, 4);
    dev->reg.dataReg = ioremap(GPIO5_DR, 4);
    dev->reg.dirReg = ioremap(GPIO5_GDIR, 4);
    dev->reg.statReg = ioremap(GPIO5_PSR, 4);
    dev->reg.bit = IO01_BIT;

    *dev->reg.muxReg &= ~0x0F;  // 清除复用设置
    *dev->reg.muxReg |= 0b101;  // 设置复用 为GPIO5
    *dev->reg.dirReg |= IO01_BIT; // 置位，设置为输出
    *dev->reg.dataReg |= IO01_BIT; // 置位，输出高电平
}

static char dev_read_data(struct dev_info *dev)
{
    volatile u32 *statReg = dev->reg.statReg;
    u32 bit = dev->reg.bit;
    char data = ((*statReg) & bit) ? IO_DATA_OFF : IO_DATA_ON;
    return data;
}
static void dev_write_data(struct dev_info *dev, char data)
{
    volatile u32 *dataReg = dev->reg.dataReg;
    u32 bit = dev->reg.bit;
    if (data == IO_DATA_OFF) {
        // 关闭设备
        *dataReg |= bit; // 置位，输出高电平
    } else {
        // 开启设备
        *dataReg &= ~bit; // 清除，输出低电平
    }
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
    data = dev_read_data(dev);
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
    dev_write_data(dev, data);
    return ret;
}
static int misc_release(struct inode *node, struct file *file)
{
    struct dev_info *dev = file->private_data;
    LOG_TRACE();
    dev_write_data(dev, IO_DATA_OFF);
    file->private_data = NULL;
    return 0;
}

static const struct file_operations op = {
    .owner = THIS_MODULE,
    .read = misc_read,
    .write = misc_write,
    .open = misc_open,
    .release = misc_release,
};
struct miscdevice my_misc_dev = {
    .minor = MISC_DYNAMIC_MINOR, // 次设备号，赋值为宏 MISC_DYNAMIC_MINOR ，表示系统自动分配
    .name = "my_beep",	// 设备名称
    .fops = &op,
};

// 加载函数
static int misc_init(void)
{
    int ret;
    LOG_TRACE();
    init_devs(g_dev_list);
    ret = misc_register(&my_misc_dev);
    return ret;
}
// 卸载函数
static void misc_exit(void)
{
    LOG_TRACE();
    misc_deregister(&my_misc_dev);
    deinit_devs(g_dev_list);
}

/**
 * 杂项设备可以省略 class 和 dev 节点的创建过程，也不需要申请主设备号。
 * 使用杂项设备 可以节省主设备号资源，节省代码量。
 * 杂项设备主设备号固定为10， dev 节点名字由 struct miscdevice 的name字段指定。
 * 使用 文件私有数据 private_data 可以让代码复用性更高。
*/

/* 必须使用宏指定 加载函数、卸载函数、GPL声明 */
module_init(misc_init);
module_exit(misc_exit);
MODULE_LICENSE("GPL");
// 可选的 作者、版本等信息
MODULE_AUTHOR("pdg");
MODULE_VERSION("v1.0");
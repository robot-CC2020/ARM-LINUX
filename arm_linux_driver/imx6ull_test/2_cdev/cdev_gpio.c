/* 必须包含的两个头文件 */
#include <linux/module.h>
#include <linux/init.h>
/* 字符设备头文件 */
#include <linux/cdev.h>
/* 设备节点创建 */
#include <linux/device.h>
/* ioremap */
#include <asm/io.h>
/* BEEP  SNVS_TAMPER1/GPIO5_IO01  low=on */

#define CCM_CCGR5     0x20C407C   // GPIO5时钟 寄存器
#define IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER1     0x229000C   // SNVS_TAMPER1 引脚复用

#define GPIO5_BASE      0x020AC000
#define GPIO5_DR    (GPIO5_BASE + 0)        // GPIO5 输出控制
#define GPIO5_GDIR  (GPIO5_BASE + 4)        // GPIO5 方向设置 [0:输入 1:输出]
#define GPIO5_PSR   (GPIO5_BASE + 8)        // GPIO5 读取状态

volatile static u32 __iomem *g_clockReg;
volatile static u32 __iomem *g_muxReg;
volatile static u32 __iomem *g_dataReg;
volatile static u32 __iomem *g_dirReg;
volatile static u32 __iomem *g_statReg;


static void register_remap(void)
{
    g_clockReg = ioremap(CCM_CCGR5, 4);
    g_muxReg = ioremap(IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER1, 4);
    g_dataReg = ioremap(GPIO5_DR, 4);
    g_dirReg = ioremap(GPIO5_GDIR, 4);
    g_statReg = ioremap(GPIO5_PSR, 4);
}

static void register_unremap(void)
{
	iounmap(g_clockReg);
	iounmap(g_muxReg);
	iounmap(g_dataReg);
	iounmap(g_dirReg);
	iounmap(g_statReg);
}

// struct gpio_dev {
//     struct cdev *dev;       // 字符设备
//     struct class *class;    // 类
// };
// static struct gpio_dev g_dev;

// struct cdev {
// 	struct kobject kobj;
// 	struct module *owner;
// 	const struct file_operations *ops; // 文件操作方法集
// 	struct list_head list;
// 	dev_t dev; // 设备号 12位主设备号 + 20位次设备号
// 	unsigned int count;
// };

// // 注册某个范围的设备号，成功返回0，失败返回负数
// int register_chrdev_region(
//     dev_t from,  // 设备号的起始值，含主、次设备号
//     unsigned count, // 次设备号的数量
//     const char *name // 设备的名称
// );

// // 系统自动分配设备号并注册， 成功返回0，失败返回负数
// int alloc_chrdev_region(
//     dev_t *dev, // 输出分配得到的 主设备号
//     unsigned baseminor, // 次设备号的起始值，一般为0
//     unsigned count, // 要申请的设备号数量
//     const char *name // 设备的名字
// );

// // 注销分配得到的设备号
// int unregister_chrdev_region(
//     dev_t dev,  // 要释放的设备号，含主、次设备号
//     unsigned count, // 设备号的数量
// );

// // 初始化 cdev结构体
// void cdev_init(
//     struct cdev *, // 需要初始化的结构体
//     const struct file_operations * // 对应的文件操作
// );
// // 把字符设备添加到系统中
// int cdev_add(
//     struct cdev *, // 已经初始化的字符设备
//     dev_t, // 对应的设备号
//     unsigned // 设备的数量，它们的次设备号是连续的
// );
// // 把字符设备删除
// void cdev_del(struct cdev *);



// 加载函数
static int gpio_init(void)
{
    printk("gpio_init\n");
    register_remap();
    *g_muxReg &= ~0x0F;  // 清除复用设置
    *g_muxReg |= 0b101;  // 设置复用 为GPIO5
    *g_dirReg |= 0b10; // 置位，设置为输出
    *g_dataReg &= ~0b10; // 清除，输出低电平
    

    return 0;
}
// 卸载函数
static void gpio_exit(void)
{
    printk("gpio_exit\n");
    *g_dataReg |= 0b10; // 置位，输出高电平
    *g_muxReg &= ~0x0F;  // 清除复用设置
    register_unremap();
}
/* 必须使用宏指定 加载函数、卸载函数、GPL声明 */
module_init(gpio_init);
module_exit(gpio_exit);
MODULE_LICENSE("GPL");
// 可选的 作者、版本等信息
MODULE_AUTHOR("pdg");
MODULE_VERSION("v1.0");
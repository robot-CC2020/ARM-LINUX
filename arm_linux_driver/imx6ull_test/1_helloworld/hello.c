/* 必须包含的两个头文件 */
#include <linux/module.h>
#include <linux/init.h>

// 加载函数
static int helloworld_init(void)
{
    printk("helloworld_init\n");
    return 0;
}
// 卸载函数
static void helloworld_exit(void)
{
    printk("helloworld_exit\n");
}
/* 必须使用宏指定 加载函数、卸载函数、GPL声明 */
module_init(helloworld_init);
module_exit(helloworld_exit);
MODULE_LICENSE("GPL");
// 可选的 作者、版本等信息
MODULE_AUTHOR("pdg");
MODULE_VERSION("v1.0");
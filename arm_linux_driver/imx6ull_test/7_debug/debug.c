/* 必须包含的两个头文件 */
#include <linux/module.h>
#include <linux/init.h>
/* 模块传参 */
#include <linux/moduleparam.h>
/* 权限定义 */
#include <linux/stat.h>

int g_select = 0;

module_param(g_select, int, S_IRUGO);

MODULE_PARM_DESC(g_select, "e.g:g_select=1");

// 加载函数
static int helloworld_init(void)
{
    
    printk("helloworld_init\n");
    switch (g_select) {
        case 1:
            /* 最高等级打印 */
            printk(KERN_EMERG "KERN_EMERG print\n");
            break;
        case 2:
            /* 最低等级打印 */
            printk(KERN_DEBUG "KERN_DEBUG print\n");
            break;
        case 3:
            /* 打印 CPU PID 和 函数调用栈 等调试信息 */
            dump_stack();
            break;
        case 4:
            /* 如果condition为真，打印调试信息，并能携带日志 */
            WARN(1, "print g_select=%d\n", g_select);
            break;
        case 5:
            /* 如果condition为真，打印调试信息 */
            WARN_ON(1);
            break;
        case 6:
            /* 如果condition为假，无动作 */
            WARN_ON(0);
            break;
        case 7:
            /* 触发内核oops，引发段错误，输出更详细打印，含寄存器值 */
            BUG();
            break;
        case 8:
            /* 如果condition为真，触发内核oops，引发段错误，输出更详细打印，含寄存器值 */
            BUG_ON(1);
            break;
        case 9:
            /* 如果condition为假，无动作 */
            BUG_ON(0);
            break;
        case 10:
            /* 系统死机并输出打印 */
            panic("print g_select=%d\n", g_select);
            break;
        default:
            break;
    }
    return 0;
}
// 卸载函数
static void helloworld_exit(void)
{
    printk("helloworld_exit\n");
}

/**
 * 驱动程序 可以编译进内核，也可以编译成模块。
 * 如果编译进内核， exit函数一般不会执行。
 * 如果编译成模块，那么可以使用命令动态加载和卸载模块。
*/

/* 必须使用宏指定 加载函数、卸载函数、GPL声明 */
module_init(helloworld_init);
module_exit(helloworld_exit);
MODULE_LICENSE("GPL");
// 可选的 作者、版本等信息
MODULE_AUTHOR("pdg");
MODULE_VERSION("v1.0");
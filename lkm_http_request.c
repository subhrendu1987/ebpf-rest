#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include "http_rest.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Subhrendu");
MODULE_DESCRIPTION("LKM for Policy Fetcher");
MODULE_VERSION("0.1");

struct socket *sock;
char *request;
char *buf;
char *request, *response;
static struct task_struct *thread_st;
/*************************************************************************/
static int http_post_init(void *unused) {
    int ret;
    while (!kthread_should_stop()) {
        ret=http_post();
        if(!ret){
            printk(KERN_ERR "Kernel http_post() Error");
        }
        printk(KERN_INFO "Kernel Thread Running\n");
        ssleep(5);
    }
    printk(KERN_INFO "Kernel Thread Stopping\n");
    return 0;
}
/*************************************************************************/
static int __init init_thread(void) {
    printk(KERN_INFO "Creating Kernel Thread\n");
    thread_st = kthread_run(http_post_init, NULL, "my_kthread");
    if (thread_st) {
        printk(KERN_INFO "Kernel Thread Created Successfully\n");
    } else {
        printk(KERN_ERR "Kernel Thread Creation Failed\n");
    }
    return 0;
}
/*************************************************************************/
// Module cleanup function
static void __exit cleanup_thread(void) {
    printk(KERN_INFO "Cleaning Up\n");
    if (thread_st) {
        kthread_stop(thread_st);
        printk(KERN_INFO "Kernel Thread Stopped\n");
    }
}
/*************************************************************************/
module_init(init_thread);
module_exit(cleanup_thread);

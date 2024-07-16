#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include "http_rest.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Subhrendu");
MODULE_DESCRIPTION("LKM for Policy Fetcher");
MODULE_VERSION("0.1");

#define URL "/v1/data/ebpf/allow"
#define SERVER_PORT 8181

/// $curl --location 'http://192.168.0.103:8181/v1/data/ebpf/allow' --header 'Content-Type: application/json' --data '{"input": {"funcName": "mptm_encap"}}'
/// {"result":true}

//#define SERVER_PORT 8181
////$IP="192.168.0.103"; printf '%02X' ${IP//./ }| sed 's/../& /g' | awk 'BEGIN {printf "0x"} {for(i=NF;i>=1;i--) printf "%s", $i} END {print ""}'
/// #define SERVER_ADDR 0x6700A8C0
#define SERVER_IP "192.168.0.103"
#define SERVER_ADDR 0x6700A8C0


/// $curl --location 'http://127.0.0.1:8181/v1/data/ebpf/allow' --header 'Content-Type: application/json' --data '{"input": {"funcName": "mptm_decap"}}'
/// {"result":false}
//#define SERVER_ADDR "127.0.0.1"
// $IP="127.0.0.1"; printf '%02X' ${IP//./ }| sed 's/../& /g' | awk 'BEGIN {printf "0x"} {for(i=NF;i>=1;i--) printf "%s", $i} END {print ""}'

//#define SERVER_IP "127.0.0.1"
//#define SERVER_ADDR 0x100007f

#define FILE_PATH "/boot/kvstore"
#define BUFFER_SIZE 512

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

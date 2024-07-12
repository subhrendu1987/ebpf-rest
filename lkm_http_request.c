#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("A simple kernel module to send an HTTP POST request");
MODULE_VERSION("0.1");

#define SERVER_PORT 9900
//#define SERVER_ADDR "127.0.0.1"
#define SERVER_ADDR 0x100007f

struct socket *sock;
char *request;

static int __init http_post_init(void) {
    struct sockaddr_in server;
    int ret;

    printk(KERN_INFO "// Allocate memory for the request\n");
    request = kmalloc(256, GFP_KERNEL);
    if (!request) {
        printk(KERN_ERR "Failed to allocate memory for request\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "// Create HTTP POST request\n");
    snprintf(request, 256,
             "POST / HTTP/1.1\r\n"
             "Host: %X:%d\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "{\"input\": {\"funcName\": \"mptm_decap\"}}\r\n",
             SERVER_ADDR, SERVER_PORT, 34);

    printk(KERN_INFO "Create a socket\n");
    ret = sock_create(AF_INET, SOCK_STREAM, 0, &sock);
    if (ret < 0) {
        printk(KERN_ERR "Failed to create socket\n");
        kfree(request);
        return ret;
    }

    printk(KERN_INFO "Set up the server address\n"); 
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    //in4_pton(SERVER_ADDR, -1, addr, -1, NULL);
    //memcpy(&server.sin_addr.s_addr, SERVER_ADDR, sizeof(server.sin_addr.s_addr));
    server.sin_addr.s_addr = SERVER_ADDR;

    printk(KERN_INFO "Connect to the server\n");
    ret = sock->ops->connect(sock, (struct sockaddr *)&server, sizeof(server), 0);
    if (ret < 0) {
        printk(KERN_ERR "Failed to connect to server\n");
        sock_release(sock);
        kfree(request);
        return ret;
    }

    printk(KERN_INFO "Send the HTTP request\n");
    {
        struct msghdr msg;
        struct kvec iov;

        memset(&msg, 0, sizeof(msg));
        iov.iov_base = request;
        iov.iov_len = strlen(request);

        ret = kernel_sendmsg(sock, &msg, &iov, 1, strlen(request));
        if (ret < 0) {
            printk(KERN_ERR "Failed to send HTTP request\n");
        } else {
            printk(KERN_INFO "HTTP request sent successfully\n");
        }
    }

    printk(KERN_INFO "Clean up\n");
    sock_release(sock);
    kfree(request);

    return ret;
}

static void __exit http_post_exit(void) {
    printk(KERN_INFO "HTTP POST module exiting\n");
    sock_release(sock);
    kfree(request);
}

module_init(http_post_init);
module_exit(http_post_exit);

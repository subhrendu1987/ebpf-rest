#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("A simple kernel module to send an HTTP POST request");
MODULE_VERSION("0.1");

#define SERVER_PORT 9900
//#define SERVER_ADDR "127.0.0.1"
// $IP="127.0.0.1"; printf '%02X' ${IP//./ }; echo and Reverse
#define SERVER_ADDR 0x100007f
#define FILE_PATH "/boot/kvstore"
#define BUFFER_SIZE 512
#define REQUEST "{\"input\": {\"funcName\": \"mptm_decap\"}}"

struct socket *sock;
char *request;
char *buf;
char *request, *response;

/*************************************************************************/
static int read_file(const char *path, char *buf, size_t size) {
    struct file *file;
    loff_t pos = 0;
    int bytes_read;
    struct msghdr msg;
    struct kvec iov;

    // Open the file
    file = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(file)) {
        printk(KERN_ERR "Failed to open file: %s\n", path);
        return PTR_ERR(file);
    }

    // Read the file
    bytes_read = kernel_read(file, buf, size - 1, &pos);
    if (bytes_read < 0) {
        printk(KERN_ERR "Failed to read file: %s\n", path);
        filp_close(file, NULL);
        return bytes_read;
    }

    buf[bytes_read] = '\0';  // Null-terminate the buffer

    // Close the file
    filp_close(file, NULL);

    return bytes_read;
}
/*************************************************************************/
static int __init http_post_init(void) {
    struct sockaddr_in server;
    int ret;
    struct kvec iov;
    struct msghdr msg;
    /**********/
    printk(KERN_INFO "// Allocate memory for the request\n");
    request = kmalloc(256, GFP_KERNEL);
    if (!request) {
        printk(KERN_ERR "Failed to allocate memory for request\n");
        return -ENOMEM;
    }
    /**********/
    printk(KERN_INFO "// Allocate memory for the response\n");
    response = kmalloc(256, GFP_KERNEL);
    if (!response) {
        printk(KERN_ERR "Failed to allocate memory for response\n");
        return -ENOMEM;
    }
    /**********/
    printk(KERN_INFO "// Read the file\n");
    {
        printk(KERN_INFO "// Allocate memory for the buffer\n");
        buf = kmalloc(BUFFER_SIZE, GFP_KERNEL);
        if (!buf) {
            printk(KERN_ERR "Failed to allocate memory for buffer\n");
            return -ENOMEM;
        }
        printk(KERN_INFO "// Read the file\n");
        ret = read_file(FILE_PATH, buf, BUFFER_SIZE);
        if (ret >= 0) {
            printk(KERN_INFO "Read from %s:\n%s\n", FILE_PATH, buf);
        }
    }
    /**********/
    printk(KERN_INFO "// Create HTTP POST request\n");
    snprintf(request, 256,
             "POST / HTTP/1.1\r\n"
             "Host: localhost:9900\r\n"
             "User-Agent: LKM\r\n"
             "Accept: */*\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "{\"input\": {\"funcName\": \"mptm_decap\"}}\r\n",
             SERVER_ADDR, SERVER_PORT, 34);
    /**********/
    printk(KERN_INFO "Create a socket\n");
    ret = sock_create(AF_INET, SOCK_STREAM, 0, &sock);
    if (ret < 0) {
        printk(KERN_ERR "Failed to create socket\n");
        kfree(request);
        return ret;
    }
    /**********/
    printk(KERN_INFO "Set up the server address\n"); 
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = SERVER_ADDR;
    /**********/
    printk(KERN_INFO "Connect to the server\n");
    ret = sock->ops->connect(sock, (struct sockaddr *)&server, sizeof(server), 0);
    if (ret < 0) {
        printk(KERN_ERR "Failed to connect to server\n");
        sock_release(sock);
        kfree(request);
        return ret;
    }
    /**********/
    printk(KERN_INFO "Send the HTTP request\n");
    {
        memset(&msg, 0, sizeof(msg));
        iov.iov_base = request;
        iov.iov_len = strlen(request);

        ret = kernel_sendmsg(sock, &msg, &iov, 1, strlen(request));
        if (ret < 0) {
            printk(KERN_ERR "Failed to send HTTP request\n");
            sock_release(sock);
            kfree(request);
            kfree(buf);
            return ret;
        } else {
            printk(KERN_INFO "HTTP request sent successfully\n");
        }
    }
    /**********/
    printk(KERN_INFO "// Wait for the response\n");
    {         
        memset(&msg, 0, sizeof(msg));
        iov.iov_base = response;
        iov.iov_len = BUFFER_SIZE;

        ret = kernel_recvmsg(sock, &msg, &iov, 1, BUFFER_SIZE - 1, 0);
        if (ret < 0) {
            printk(KERN_ERR "Failed to receive HTTP response\n");
            sock_release(sock);
            kfree(request);
            kfree(response);
            kfree(buf);
            return ret;
        } else {
            response[ret] = '\0';  // Null-terminate the buffer
            printk(KERN_INFO "HTTP response received: %s\n", response);
        }
    }
    /**********/
    printk(KERN_INFO "Clean up\n");
    sock_release(sock);
    kfree(request);
    kfree(response);
    kfree(buf);
    return ret;
}
/*************************************************************************/
static void __exit http_post_exit(void) {
    printk(KERN_INFO "HTTP POST module exiting\n");
    sock_release(sock);
    kfree(request);
}
/*************************************************************************/
module_init(http_post_init);
module_exit(http_post_exit);

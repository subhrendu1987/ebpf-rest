#ifndef HTTP_REST_MODULE_H
#define HTTP_REST_MODULE_H


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
#include "kvstore.h"

#ifndef CONFIG_MODULE_H
#include "config.h"
#endif

struct socket *sock;
char *request;
char *buf;
char *request, *response;
static struct task_struct *thread_st;
/*************************************************************************/
/*************************************************************************/
static char * send_rest(char *fName){
    struct sockaddr_in server;
    int ret=0;
    struct kvec iov;
    struct msghdr msg;
    char *querystr;
    int len = snprintf(NULL, 0, "{\"input\": {\"funcName\": \"%s\"}}\r\n", fName) + 1;
    querystr = kmalloc(256, GFP_KERNEL);
    if (!querystr) {
        printk(KERN_ERR "Failed to allocate memory for query string\n");
        return(NULL);
    }
    snprintf(querystr, len, "{\"input\": {\"funcName\": \"%s\"}}\r\n", fName);
    /**********/
    //printk(KERN_INFO "// Allocate memory for the request\n");
    request = kmalloc(256, GFP_KERNEL);
    if (!request) {
        printk(KERN_ERR "Failed to allocate memory for request\n");
        return(NULL);
    }
    /**********/
    //printk(KERN_INFO "// Create HTTP POST request\n");
    snprintf(request, 256,
             "POST %s HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "User-Agent: LKM\r\n"
             "Accept: */*\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "%s"
             ,URL,SERVER_IP,SERVER_PORT,(int)strlen(querystr),querystr);
    printk(KERN_INFO "Request: %s",request);
    /**********/
    //printk(KERN_INFO "Create a socket\n");
    ret = sock_create(AF_INET, SOCK_STREAM, 0, &sock);
    if (ret < 0) {
        printk(KERN_ERR "Failed to create socket\n");
        kfree(request);
        return(NULL);
    }
    /**********/
    //printk(KERN_INFO "Set up the server address\n"); 
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
        return(NULL);
    }
    /**********/
    //printk(KERN_INFO "Send the HTTP request\n");
    {
        memset(&msg, 0, sizeof(msg));
        iov.iov_base = request;
        iov.iov_len = strlen(request);

        ret = kernel_sendmsg(sock, &msg, &iov, 1, strlen(request));
        if (ret < 0) {
            printk(KERN_ERR "Failed to send HTTP request\n");
            sock_release(sock);
            kfree(request);
        } else {
            printk(KERN_INFO "HTTP request sent successfully\n");
        }
    }
    /**********/
    //printk(KERN_INFO "// Wait for the response\n");
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
            return(NULL);
        } else {
            //printk(KERN_INFO "Received byte:%d",ret);
            response[ret] = '\0';  // Null-terminate the buffer
            printk(KERN_INFO "HTTP response received: %s\n", response);
            return(response);
        }
    }
}
/*************************************************************************/
static int http_post(void) {
    int ret=0;
    char *line;
    char *funcName;
    /**********/
    //printk(KERN_INFO "// Allocate memory for the response\n");
    response = kmalloc(256, GFP_KERNEL);
    if (!response) {
        printk(KERN_ERR "Failed to allocate memory for response\n");
        return -ENOMEM;
    }
    /**********/
    //printk(KERN_INFO "// Allocate memory for the file buffer\n");
    buf = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!buf) {
        printk(KERN_ERR "Failed to allocate memory for file buffer\n");
        return -ENOMEM;
    }
    line=search_unresolved(buf,BUFFER_SIZE);
    while(line){ 
        line=search_unresolved(buf,BUFFER_SIZE);
        if(line){
            printk(KERN_INFO "Line:%s",line);
        }else{
            break;
            return(-1);
        }
        funcName=strsep_split(line," ");
        if(funcName){
            printk(KERN_INFO "funcName:%s",funcName);
        }else{
            return(-1);
        }
        /**********/
        response=send_rest(funcName);
        if(!response){
            printk(KERN_ERR "response\n");
            return(-1);
        }
        char *lastLine=extract_data(response);
        if(!lastLine){
            printk(KERN_ERR "Last line\n");
            return(-1);
        }
        printk(KERN_INFO "Response Data:%s\n",lastLine);
        int decision=process_response(lastLine);
        printk(KERN_INFO "decision:%d\n",decision);
        /**********/
        char *newFileString;
        int len = snprintf(NULL, 0, "%s %d\n", funcName,decision) + 1;
        newFileString = kmalloc(len, GFP_KERNEL);
        if (!newFileString) {
            printk(KERN_ERR "Failed to allocate memory for file buffer\n");
            return -ENOMEM;
        }
        snprintf(newFileString, len, "%s %d\n", funcName,decision);
        /**********/
        char *oldFileString;
        len = snprintf(NULL, 0, "%s -1\n", funcName) + 1;
        oldFileString = kmalloc(len, GFP_KERNEL);
        if (!oldFileString) {
            printk(KERN_ERR "Failed to allocate memory for file buffer\n");
            return -ENOMEM;
        }
        snprintf(oldFileString, len, "%s -1\n", funcName);
        modifyKVstore(oldFileString,newFileString);
        kfree(newFileString);
        kfree(oldFileString);
        //write_file(line,funcName,strlen(funcName));
    }
    /**********/
    printk(KERN_INFO "Clean up\n");
    kfree(response);
    kfree(buf);
    return ret;
}
/*************************************************************************/
#endif
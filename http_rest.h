#ifndef MY_MODULE_H
#define MY_MODULE_H


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
#include <linux/delay.h>


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
static int process_response(char *rsp){
    char *result = strstr(rsp, "false");
    if (result) {
        return(0);
    } else {
        char *result = strstr(rsp, "true");
        if(result){
            return(1);
        }
        return(-1);
    }
}
/*************************************************************************/
static char *extract_data(char *str) {
    char *last_line = NULL;
    char *newline = NULL;

    // Check if the input string is NULL or empty
    if (!str || *str == '\0') {
        printk(KERN_ERR "Input string is NULL or empty\n");
        return NULL;
    }

    // Start from the end of the string and move backwards
    for (newline = str + strlen(str) - 1; newline >= str; --newline) {
        if (*newline == '\n' && (newline + 1) < (str + strlen(str)) && *(newline + 1) != '\0') {
            // Check if the next character after the newline is not '\0' and not a newline
            if (*(newline + 1) != '\n' && *(newline + 1) != '\r') {
                last_line = newline + 1;
                break;
            }
        }
    }

    // If no non-empty line is found, return NULL
    if (!last_line) {
        printk(KERN_ERR "No non-empty line found\n");
        return NULL;
    }

    // Optional: Remove trailing newline characters from the last line
    for (newline = last_line + strlen(last_line) - 1; newline >= last_line; --newline) {
        if (*newline == '\n' || *newline == '\r') {
            *newline = '\0';
        } else {
            break;
        }
    }

    return last_line;
}

/*************************************************************************/
static char * strsep_split(char * line, const char * delim){
    char *token;
    char *rest = line;
    //printk(KERN_INFO "Original line: %s\n", line);
    // Tokenize the string using space as delimiter
    while ((token = strsep(&rest, delim)) != NULL) {
        //printk(KERN_INFO "Token: %s\n", token);
        return(token);
    }
    return(NULL); // Non-zero return means that the module couldn't be loaded.
}
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
static ssize_t write_file_content(const char *path, const char *buf, size_t size) {
    printk(KERN_INFO "write_file_content()- path:%s, size:%d\n",path,(int)size);
    struct file *filp;
    loff_t pos = 0;
    ssize_t ret;

    filp = filp_open(path, O_WRONLY | O_TRUNC, 0);
    if (IS_ERR(filp)) {
        printk(KERN_ERR "Failed to open file: %ld\n", PTR_ERR(filp));
        return PTR_ERR(filp);
    }

    ret = kernel_write(filp, buf, size, &pos);
    if (ret < 0) {
        printk(KERN_ERR "Failed to write file: %zd\n", ret);
        filp_close(filp, NULL);
        return ret;
    }

    filp_close(filp, NULL);
    return 0;
}
/*************************************************************************/
static char *replace_string(const char *str, const char *old, const char *new) {
    printk(KERN_INFO "replace_string()- Old:%s, New:%s\n",old,new);
    char *result;
    char *insert_point;
    const char *tmp;
    size_t old_len, new_len, count;
    
    old_len = strlen(old);
    new_len = strlen(new);

    // Count the number of replacements needed
    for (count = 0, tmp = str; (tmp = strstr(tmp, old)); ++tmp) {
        count++;
    }

    result = kmalloc(strlen(str) + (new_len - old_len) * count + 1, GFP_KERNEL);
    if (!result) {
        printk(KERN_ERR "Failed to allocate memory for result string\n");
        return NULL;
    }

    tmp = str;
    insert_point = result;
    while (count--) {
        const char *p = strstr(tmp, old);
        size_t len = p - tmp;
        memcpy(insert_point, tmp, len);
        insert_point += len;
        memcpy(insert_point, new, new_len);
        insert_point += new_len;
        tmp = p + old_len;
    }
    strcpy(insert_point, tmp);
    
    return result;
}
/*************************************************************************/
static ssize_t read_file_content(const char *path, char **buf, size_t *size) {
    printk(KERN_INFO "read_file_content()- path:%s\n",path);
    struct file *file;
    loff_t pos = 0;
    int bytes_read;
    *buf = kmalloc(*size + 1, GFP_KERNEL);
    if (!*buf) {
        printk(KERN_ERR "Failed to allocate memory\n");
        filp_close(file, NULL);
        return -ENOMEM;
    }

    // Open the file
    file = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(file)) {
        printk(KERN_ERR "Failed to open file: %s\n", path);
        return PTR_ERR(file);
    }

    // Read the file
    bytes_read = kernel_read(file, *buf, *size - 1, &pos);
    if (bytes_read < 0) {
        printk(KERN_ERR "Failed to read file: %s\n", path);
        filp_close(file, NULL);
        return bytes_read;
    }
    (*buf)[bytes_read] = '\0';  // Null-terminate the buffer

    // Close the file
    filp_close(file, NULL);

    return bytes_read;
}
/*************************************************************************/
static char * search_unresolved(char *buf,size_t size) {
    printk(KERN_INFO "read_file()-buf:%s,size:%d\n",buf,(int)size);
    int ret1=0;
    char *line;
    ret1 = read_file_content(FILE_PATH, &buf, &size);
    if (ret1 < 0) {
        return(NULL);
    }else{
        printk(KERN_INFO "Read Size:%d\n",(int)ret1);
    }
    char *rest = buf;
    buf[ret1] = '\0';  // Null-terminate the buffer
    // Find -ve entry
    const char delimiter[] = "\n";
    // Tokenize the string using newline as delimiter
    while ((line = strsep(&rest, delimiter)) != NULL) {
        // Check if the line contains "-1"
        if (strstr(line, "-1") != NULL) {
            //printk(KERN_INFO "Line with -1: %s\n", line);
            return line;
        }
    }
    return (NULL);
}
/*************************************************************************/
static int modifyKVstore(char *oldstr,char *newstr) {
    printk(KERN_INFO "modifyKVstore()- oldstr:%s, newstr:%s)\n",oldstr,newstr);
    int ret3;
    char *buf;
    size_t size=BUFFER_SIZE;
    buf = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!buf) {
        printk(KERN_ERR "Failed to allocate memory for file buffer\n");
        return -ENOMEM;
    }
    
    ret3 = read_file_content(FILE_PATH, &buf, &size);
    if (ret3 < 0) {
        return ret3;
    }

    char *modified_buf = replace_string(buf, oldstr, newstr);
    if (!modified_buf) {
        kfree(buf);
        return -ENOMEM;
    }

    ret3 = write_file_content(FILE_PATH, modified_buf, strlen(modified_buf));
    kfree(buf);
    kfree(modified_buf);
    return(ret3);
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
/*************************************************************************/
/*************************************************************************/
#endif // MY_MODULE_H
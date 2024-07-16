#ifndef KVSTORE_MODULE_H
#define KVSTORE_MODULE_H


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

#ifndef CONFIG_MODULE_H
#include "config.h"
#endif

char *buf;
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
#endif
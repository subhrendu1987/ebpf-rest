#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#define FILE_PATH "/tmp/kvstore"
#define OLD_STRING "mptm_decap -1"
#define NEW_STRING "mptm_decap 1"


static int __init replace_string_in_file_init(void);
static void __exit replace_string_in_file_exit(void);

module_init(replace_string_in_file_init);
module_exit(replace_string_in_file_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A kernel module to replace a string in a file with a new string");

static ssize_t read_file_content(const char *path, char **buf, size_t *size) {
    struct file *filp;
    loff_t pos = 0;
    ssize_t ret;

    filp = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(filp)) {
        printk(KERN_ERR "Failed to open file: %ld\n", PTR_ERR(filp));
        return PTR_ERR(filp);
    }

    *size = i_size_read(file_inode(filp));
    *buf = kmalloc(*size + 1, GFP_KERNEL);
    if (!*buf) {
        printk(KERN_ERR "Failed to allocate memory\n");
        filp_close(filp, NULL);
        return -ENOMEM;
    }

    ret = kernel_read(filp, *buf, *size, &pos);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read file: %zd\n", ret);
        kfree(*buf);
        filp_close(filp, NULL);
        return ret;
    }

    (*buf)[*size] = '\0';
    filp_close(filp, NULL);
    return 0;
}

static ssize_t write_file_content(const char *path, const char *buf, size_t size) {
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

static char *replace_string(const char *str, const char *old, const char *new) {
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

static int __init replace_string_in_file_init(void)
{
    char *buf;
    size_t size;
    int ret;

    ret = read_file_content(FILE_PATH, &buf, &size);
    if (ret < 0) {
        return ret;
    }

    char *modified_buf = replace_string(buf, OLD_STRING, NEW_STRING);
    if (!modified_buf) {
        kfree(buf);
        return -ENOMEM;
    }

    ret = write_file_content(FILE_PATH, modified_buf, strlen(modified_buf));
    kfree(buf);
    kfree(modified_buf);

    return ret;
}

static void __exit replace_string_in_file_exit(void)
{
    printk(KERN_INFO "Exiting replace string in file module\n");
}

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>  // Include for mm_segment_t and set_fs
#include <linux/string.h>   // Include for memset and memcpy

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("A simple kernel module to read from /boot/kvstore");
MODULE_VERSION("0.1");

#define FILE_PATH "/boot/kvstore"
#define BUFFER_SIZE 512

static int read_file(const char *path, char *buf, size_t size) {
    struct file *file;
    loff_t pos = 0;
    int bytes_read;

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

static int __init file_reader_init(void) {
    char *buf;
    int ret;

    // Allocate memory for the buffer
    buf = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!buf) {
        printk(KERN_ERR "Failed to allocate memory for buffer\n");
        return -ENOMEM;
    }

    // Read the file
    ret = read_file(FILE_PATH, buf, BUFFER_SIZE);
    if (ret >= 0) {
        printk(KERN_INFO "Read from %s:\n%s\n", FILE_PATH, buf);
    }

    // Free the buffer
    kfree(buf);

    return ret < 0 ? ret : 0;
}

static void __exit file_reader_exit(void) {
    printk(KERN_INFO "File reader module exiting\n");
}

module_init(file_reader_init);
module_exit(file_reader_exit);

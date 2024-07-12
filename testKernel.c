#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.01");

static int __init lkm_example_init(void) {
    printk(KERN_INFO "LKM Example: Module loaded.\n");
    // Initialization code here
    return 0; // Return 0 if successful
}

static void __exit lkm_example_exit(void) {
    printk(KERN_INFO "LKM Example: Module unloaded.\n");
    // Cleanup code here
}

module_init(lkm_example_init);
module_exit(lkm_example_exit);

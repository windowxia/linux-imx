#include <linux/module.h>
#include <linux/gpio/consumer.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>     // 新增

#define DEVICE_NAME "gnss_power"

static struct gpio_desc *gnss_gpio;

static int dev_open(struct inode *inode, struct file *file) {
    return nonseekable_open(inode, file);
}

static ssize_t dev_write(struct file *file, const char __user *buf,
                         size_t count, loff_t *ppos) {
    char kbuf[2];
    if (copy_from_user(kbuf, buf, count)) return -EFAULT;
    
    int value = kbuf[0] - '0';
    gpiod_set_value(gnss_gpio, value); // 注意：低电平有效
    return count;
}

static const struct file_operations fops = {
    .open = dev_open,
    .write = dev_write,
};

static int __init gnss_power_init(void) {
    // 获取GPIO描述符
    gnss_gpio = gpiod_get_from_of_node(NULL, "regulator-gnss-3v3", "en");
    if (IS_ERR(gnss_gpio)) {
        pr_err("Failed to get GPIO\n");
        return PTR_ERR(gnss_gpio);
    }
    
    // 创建设备节点
    if (!class_create(THIS_MODULE, DEVICE_NAME))
        device_create(class_create(THIS_MODULE, DEVICE_NAME), NULL,
                      MKDEV(0, 0), NULL);
    else
        pr_err("Failed to create class\n");
    
    return 0;
}

static void __exit gnss_power_exit(void) {
    device_destroy(class_create(THIS_MODULE, DEVICE_NAME), MKDEV(0, 0));
    class_destroy(THIS_MODULE, DEVICE_NAME);
    gpiod_put(gnss_gpio);
}

module_init(gnss_power_init);
module_exit(gnss_power_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("GNSS Power Control Driver");
#include <linux/module.h>
#include <linux/version.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include "exp_device_data.h"


//
//
// Debugfs is typically mounted with a command like:
//
//    mount -t debugfs none /sys/kernel/debug
//     https://www.kernel.org/doc/Documentation/filesystems/debugfs.txt
//
// This file creates a /sys/kernel/debug/exp_debugfs
//   to check the state : cat exp_debugfs
//   to turn on device   : echo 1 > exp_debugfs
//   to turn off device   : echo 0 > exp_debugfs
//

static struct dentry *exp_debugfs;
#define EXP_DEBUGFS_BUF_SIZE	128
static char exp_debugfs_buf[EXP_DEBUGFS_BUF_SIZE] = "Hello exp debugfs";

static ssize_t exp_debug_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	struct device *dev = (void *)file->private_data;
	struct exp_device_platform_data *platform_data = (void *) dev_get_platdata(dev);
	struct exp_driver_data *pdrvdata = (void *) dev_get_drvdata(dev);

	int len;
	
	if(count > EXP_DEBUGFS_BUF_SIZE)
			count = EXP_DEBUGFS_BUF_SIZE;
	
	// print platfrom device data and driver data
	sprintf(exp_debugfs_buf,"poweron_gpio = %d\ntimer_msec = %d\nstate = %d\ndata1 =%d\ndata2=%d\n", 
			platform_data->poweron_gpio,platform_data->timer_msec,platform_data->state,
			pdrvdata->data1,pdrvdata->data2);
		
	len = copy_to_user(user_buf,exp_debugfs_buf,count);
	
	return (ssize_t)count;
}

static ssize_t exp_debug_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
	struct device *dev = (void *)file->private_data;
	struct exp_device_platform_data *platform_data = (void *) dev_get_platdata(dev);
	struct exp_driver_data *pdrvdata = dev_get_drvdata(dev);
	int len;
	
	if(count > EXP_DEBUGFS_BUF_SIZE)
			count = EXP_DEBUGFS_BUF_SIZE;
		
	len = copy_from_user(exp_debugfs_buf,user_buf,count);

	if(exp_debugfs_buf[0] == '1')
		(* platform_data->enable)(platform_data);

	if(exp_debugfs_buf[0] == '0')
		(* platform_data->disable)(platform_data);
	
	return (ssize_t)count;
}


static const struct file_operations exp_debug_fops = {
	.open = simple_open,
	.read = exp_debug_read,
	.write = exp_debug_write
};

int exp_debugfs_create(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret = 0;
	
	exp_debugfs = debugfs_create_file("exp_debugfs", S_IRUGO, NULL, (void *)dev,&exp_debug_fops);
	if (!exp_debugfs)
		ret = -ENOMEM;	
	
	return ret;
}
EXPORT_SYMBOL(exp_debugfs_create);

int exp_debugfs_remove(struct platform_device *pdev)
{
	debugfs_remove(exp_debugfs);
	return 0;
}
EXPORT_SYMBOL(exp_debugfs_remove);


MODULE_DESCRIPTION("Sample debugfs");

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include "exp_device_data.h"
#include "exp_device_ioctl.h"


/*
**	This file creates a charactor device node
**	/dev/exp_device
**	/sys/class/exp_class/exp_device
**		dev
**		power
**		subsystem
**		uevent
**	/sys/device/virtual/exp_class --> /sys/class/exp_class
*/
	
// store the reference to platrom_device locally
static struct platform_device *exp_platform_device = NULL;

static int exp_open(struct inode *i,struct file *f)
{
  return 0;
}
static int exp_close(struct inode *i,struct file *f)
{
  return 0;
}
static ssize_t exp_read(struct file *f,char __user *buf,size_t len,loff_t *off)
{
  struct exp_driver_data *driver_data = platform_get_drvdata(&exp_platform_device->dev);
  char temp_buf[64];

  // get platform_driver_data
  sprintf(temp_buf,"data1=%d data2=%d",driver_data->data1,driver_data->data2);
  copy_to_user(buf, temp_buf,32);
  return 32;
}
static ssize_t exp_write(struct file *f,const char __user *buf,size_t len,loff_t *off)
{
  return len;
}

static long exp_compat_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
  struct exp_device_platform_data *platform_data = dev_get_platdata(&exp_platform_device->dev);
  struct exp_driver_data *driver_data = platform_get_drvdata(&exp_platform_device->dev);
  struct exp_ioctl_arg *exp_ioctl_arg = (struct exp_ioctl_arg *) arg;

  if(!platform_data)
  	return 0;

  switch(cmd)
  	{
  		case EXP_IOCTL_CMD_GET_TIMER:
			exp_ioctl_arg->timer  = platform_data->timer_msec;
			break;
		case EXP_IOCTL_CMD_SET_TIMER:
			platform_data->timer_msec = exp_ioctl_arg->timer;
			break;
		case EXP_IOCTL_CMD_POWER_ON:
			if(platform_data && platform_data->power_on)
					(* platform_data->power_on)(platform_data);
			break;
		case EXP_IOCTL_CMD_POWER_OFF:
			if(platform_data && platform_data->power_off)
					(* platform_data->power_off)(platform_data);
			break;
		case EXP_IOCTL_CMD_ENABLE:
			if(platform_data && platform_data->enable)
					(* platform_data->enable)(platform_data);
			break;
		case EXP_IOCTL_CMD_DISABLE:
			if(platform_data && platform_data->disable)
					(* platform_data->disable)(platform_data);
			break;
		case EXP_IOCTL_CMD_GET_DRVDATA:
			if(driver_data)
			{
				exp_ioctl_arg->data1 = driver_data->data1;
				exp_ioctl_arg->data2 = driver_data->data2;
			}
		default:
			break;
  	}
  return 0;
}

//
// defined the global variables for the device node
//
static dev_t 		exp_region;
static struct class *exp_class;
static struct cdev 	exp_cdev;

   
static struct file_operations exp_fops =
{
  .owner = THIS_MODULE,
  .open = exp_open,
  .release = exp_close,
  .read = exp_read,
  .write = exp_write,
 // .compat_ioctl = exp_compat_ioctl,
  .unlocked_ioctl = exp_compat_ioctl,
};
 
int __init cdev_node_create(struct platform_device *pdev)
{
  
  if(alloc_chrdev_region(&exp_region, 0, 1, "example") < 0)
    return -1;
  
  if((exp_class = class_create(THIS_MODULE, "exp_class")) == NULL)
  {
    unregister_chrdev_region(exp_region, 1);
    return -1;
  }
  
  if(device_create(exp_class, NULL, exp_region, NULL, "exp_device") == NULL)
  {
    class_destroy(exp_class);
    unregister_chrdev_region(exp_region, 1);
    return -1;
  }

  // region and class created, now initialize the cdev
  cdev_init(&exp_cdev, &exp_fops);
  
  if(cdev_add(&exp_cdev, exp_region, 1) == -1)
  {
    device_destroy(exp_class, exp_region);
    class_destroy(exp_class);
    unregister_chrdev_region(exp_region, 1);
    return -1;
  }

  exp_platform_device = pdev;

  return 0;
}
EXPORT_SYMBOL(cdev_node_create);

void __exit cdev_node_remove(void)
{
  cdev_del(&exp_cdev);
  device_destroy(exp_class, exp_region);
  class_destroy(exp_class);
  unregister_chrdev_region(exp_region, 1);
}
EXPORT_SYMBOL(cdev_node_remove);

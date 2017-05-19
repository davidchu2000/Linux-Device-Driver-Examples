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
**		state
**		count
**		on
**	/sys/device/virtual/exp_class --> /sys/class/exp_class
*/

// 
// Functions to create and remove debugfs in platform_device_driver_debugfs.c
//
extern int exp_debugfs_create(struct platform_device *pdev);
extern int exp_debugfs_remove(struct platform_device *pdev);

// This example device uses dummy vibrator library fucntions as example
extern void vibrator_enable(void);
extern void vibrator_disable(void);

// store the reference to platrom_device locally
static struct platform_device *exp_platform_device = NULL;

static struct exp_cdrv_t {
	unsigned int count;
	unsigned int timer;
	unsigned int state;
	struct kobject *kobj;
	struct mutex sem;
	struct exp_device_platform_data *pdata;
}exp_cdrv;


static struct exp_cdev_t  {
	struct class *class;
	struct device *dev;
	dev_t devno;
	struct cdev chdev;
	struct exp_device_platform_data *pdata;
} exp_cdev;

static int exp_open(struct inode *i,struct file *f)
{
  f->private_data = &exp_cdrv;
  return 0;
}
static int exp_close(struct inode *i,struct file *f)
{
  return 0;
}
static ssize_t exp_read(struct file *f,char __user *buf,size_t len,loff_t *off)
{
  struct exp_device_driver_data *driver_data = platform_get_drvdata(exp_platform_device);
  struct exp_cdrv_t *exp_cdrv = f->private_data;
  
  char temp_buf[64];
  int n_notcopied;

  if(*off !=0)
  	return 0;

  // get platform_driver_data
  sprintf(temp_buf,"platform driver data1=%d data2=%d\ncdev driver state=%d count=%d\n",
  	driver_data->data1,driver_data->data2,
  	exp_cdrv->state,exp_cdrv->count);
  n_notcopied = copy_to_user(buf, temp_buf,strlen(temp_buf));
  
  return strlen(temp_buf)-n_notcopied;
}
static ssize_t exp_write(struct file *f,const char __user *buf,size_t len,loff_t *off)
{
  return len;
}

static long exp_unlocked_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
  struct exp_device_platform_data *platform_data = dev_get_platdata(&exp_platform_device->dev);
  struct exp_device_driver_data *driver_data = platform_get_drvdata(exp_platform_device);
  struct exp_cdrv_t *exp_cdrv = file->private_data;
  struct exp_ioctl_arg *exp_ioctl_arg = (struct exp_ioctl_arg *) arg;
  
  if(!platform_data)
  	return 0;

  switch(cmd)
  	{
		case EXP_IOCTL_CMD_POWER_ON:
			if(platform_data && platform_data->power_on)
					(* platform_data->power_on)(platform_data);
			break;
		case EXP_IOCTL_CMD_POWER_OFF:
			if(platform_data && platform_data->power_off)
					(* platform_data->power_off)(platform_data);
			break;
		case EXP_IOCTL_CMD_ENABLE:
			vibrator_enable();
			exp_cdrv->state = 1;
			exp_cdrv->count++;
			break;
		case EXP_IOCTL_CMD_DISABLE:
			vibrator_disable();
			exp_cdrv->state = 0;
			break;
		case EXP_IOCTL_CMD_GET_PDRVDATA:
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

static struct file_operations exp_cdev_fops =
{
  .owner = THIS_MODULE,
  .open = exp_open,
  .release = exp_close,
  .read = exp_read,
  .write = exp_write,
  .unlocked_ioctl = exp_unlocked_ioctl,
};
static ssize_t exp_attr_on(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct exp_cdrv_t * exp_cdrv = (struct exp_cdrv_t *)dev_get_drvdata(dev);
	
	if(exp_cdrv->state)
		vibrator_disable();
	else
		vibrator_enable();
	
	mutex_lock(&exp_cdrv->sem);
	exp_cdrv->count++;
	exp_cdrv->state ^= 1;
	mutex_unlock(&exp_cdrv->sem);
	return count;
}

static ssize_t exp_attr_show_count(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct exp_cdrv_t *exp_cdrv = (struct exp_cdrv_t *)dev_get_drvdata(dev);
	ssize_t res;
	
	mutex_lock(&exp_cdrv->sem);
	res = snprintf(buf, PAGE_SIZE, "%d\n", exp_cdrv->count);
	mutex_unlock(&exp_cdrv->sem);
	
	return res;
}
static ssize_t exp_attr_store_count(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct exp_cdrv_t * exp_cdrv = (struct exp_cdrv_t *)dev_get_drvdata(dev);
	ssize_t res;
	int the_count;
	char *end;
	
	end = (char *)buf + count;
	the_count = (int)simple_strtol(buf, &end, 10);
	
	mutex_lock(&exp_cdrv->sem);
	exp_cdrv->count = the_count;
	mutex_unlock(&exp_cdrv->sem);
	return count;
}

static ssize_t exp_attr_show_state(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct exp_cdrv_t *exp_cdrv = (struct exp_cdrv_t *)dev_get_drvdata(dev);
	ssize_t res;
	
	mutex_lock(&exp_cdrv->sem);
	res = snprintf(buf, PAGE_SIZE, "%d\n", exp_cdrv->state);
	mutex_unlock(&exp_cdrv->sem);
	
	return res;
}
DEVICE_ATTR(xyz_on, S_IWUSR|S_IWGRP, NULL, exp_attr_on);
DEVICE_ATTR(xyz_count, S_IWUSR|S_IWGRP|S_IRUGO, exp_attr_show_count, exp_attr_store_count);
DEVICE_ATTR(xyz_state, S_IRUGO, exp_attr_show_state, NULL);

static struct device_attribute *exp_cdev_attr_list[] = {
	&dev_attr_xyz_on,
	&dev_attr_xyz_count,
	&dev_attr_xyz_state,
};
int __init cdev_node_create(struct platform_device *pdev)
{
  struct exp_device_platform_data *pdata = pdev->dev.platform_data;
  int num_attributes,i;
  
  if(alloc_chrdev_region(&exp_cdev.devno, 0, 1, "exp_device") < 0)
    return -1;
  
  if((exp_cdev.class = class_create(THIS_MODULE, "exp_class")) == NULL)
  {
    unregister_chrdev_region(exp_cdev.devno, 1);
    return -1;
  }
  
  memset(&exp_cdrv, 0, sizeof(exp_cdrv));
  
  if((exp_cdev.dev = device_create(exp_cdev.class, NULL, exp_cdev.devno, &exp_cdrv, "exp_cdev")) == NULL)
  {
    class_destroy(exp_cdev.class);
    unregister_chrdev_region(exp_cdev.devno, 1);
    return -1;
  }

  exp_cdev.pdata = pdata;
  exp_cdrv.pdata = pdata;
  exp_cdrv.count = 0;
  exp_cdrv.timer = 0;
  exp_cdrv.state = 0;
  exp_cdrv.kobj = &exp_cdev.dev->kobj;
  mutex_init(&exp_cdrv.sem);

  cdev_init(&exp_cdev.chdev, &exp_cdev_fops);
  exp_cdev.chdev.owner = THIS_MODULE;
  if(cdev_add(&exp_cdev.chdev, exp_cdev.devno, 1) == -1)
  {
    device_destroy(exp_cdev.class, exp_cdev.devno);
    class_destroy(exp_cdev.class);
    unregister_chrdev_region(exp_cdev.devno, 1);
    return -1;
  }

  // create file attributes
  num_attributes = (int)(sizeof(exp_cdev_attr_list) / sizeof(exp_cdev_attr_list[0]));
  for (i = 0; i < num_attributes; i++)
	device_create_file(exp_cdev.dev, exp_cdev_attr_list[i]);

  //platform_set_drvdata(pdev,&exp_cdrv); 

  exp_platform_device = pdev;

  return 0;
}
EXPORT_SYMBOL(cdev_node_create);

void __exit cdev_node_remove(pdev)
{
  cdev_del(&exp_cdev.chdev);
  device_destroy(exp_cdev.class, exp_cdev.devno);
  class_destroy(exp_cdev.class);
  unregister_chrdev_region(exp_cdev.devno, 1);
}
EXPORT_SYMBOL(cdev_node_remove);

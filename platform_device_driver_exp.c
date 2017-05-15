#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/slab.h>
#include "exp_device_data.h"


/*
**	This platform device driver will create device structure as:
**		/sys/devices/platform/exp-platform-device
**			driver
**			modalias : platform:exp-platform-device
**			uevent
**			xyz_on
*/


#define     EXP_DEVICE_NAME	 "exp-platform-device"
#define		EXP_DRIVER_HOT_PLUIN	1

// This example device uses dummy vibrator library fucntions as example
extern void vibrator_power_on(void);
extern void vibrator_power_off(void);
extern void vibrator_enable(void);
extern void vibrator_disable(void);


// 
// Functions to create and remove debugfs in platform_device_driver_debugfs.c
//
extern int exp_debugfs_create(struct platform_device *pdev);
extern int exp_debugfs_remove(struct platform_device *pdev);

//
// Character Device Node
// Functions to create and remove cdev node in platform_device_driver_node.c
//
extern int __init cdev_node_create(struct platform_device *pdev);
extern void __exit cdev_node_remove(void);


static void exp_device_power_on(struct exp_device_platform_data* pdata)
{
	vibrator_power_on();
}

static void exp_device_power_off(struct exp_device_platform_data* pdata)
{
	vibrator_power_off();
}

static void exp_device_enable(struct exp_device_platform_data* pdata)
{
	vibrator_enable();
}

static void exp_device_disable(struct exp_device_platform_data* pdata)
{
	vibrator_disable();
}


/* "exp device" platform data */
static struct exp_device_platform_data exp_device_pdata = {
    .poweron_gpio   = 101,
	.timer_msec		= 1000,
	.state			= 0,
    .power_on		= exp_device_power_on,
    .power_off		= exp_device_power_off,
    .enable			= exp_device_enable,
    .disable		= exp_device_disable,
};

static struct resource exp_device_resources[] = {
   {
	   .start  = 0x10000000,
	   .end    = 0x10001000,
	   .flags  = IORESOURCE_MEM,
	   .name   = "io-memory"
   },
   {
	   .start  = 20,
	   .end    = 20,
	   .flags  = IORESOURCE_IRQ,
	   .name   = "irq",
   }
};


static struct platform_device exp_device = {
	.name		= EXP_DEVICE_NAME,
    .id			= PLATFORM_DEVID_NONE,
    .dev.platform_data	= &exp_device_pdata,
    .num_resources = ARRAY_SIZE(exp_device_resources),
	.resource = exp_device_resources,
};


/* Suspend the device. */
static int exp_device_pm_suspend(struct device *dev)
{
    struct exp_driver_data* driver_data = dev_get_drvdata(dev);

    /* Suspend the device here*/

    return 0;
}

/* Resume the device. */
static int exp_device_pm_resume(struct device *dev)
{
    struct exp_driver_data* driver_data = dev_get_drvdata(dev);

    /* Resume the device here*/

    return 0;
}

//
// This section demostrates how to create device arrtibute doe sysfs
// Check : https://www.kernel.org/doc/Documentation/filesystems/sysfs.txt
//
static ssize_t attr_on_func(struct device *dev, struct device_attribute *attr, const char *buf,size_t size)
{
	if (buf != NULL && size != 0) {
		if (buf[0] == '0') {
			exp_device_disable(NULL);
		} else {
			exp_device_enable(NULL);
		}
	}
	return size;
}
static DEVICE_ATTR(xyz_on, 0220, NULL, attr_on_func);


static int exp_driver_probe(struct platform_device *pdev)
{
    struct exp_device_platform_data *exp_device_pdata;
    struct exp_driver_data* driver_data;
	int ret;

    exp_device_pdata = dev_get_platdata(&pdev->dev);

    /* Try to power on the device. */
    if (exp_device_pdata->power_on) {
        exp_device_pdata->power_on(exp_device_pdata);
    }

    /* Allocate the driver data here */
    driver_data = kzalloc(sizeof(struct exp_driver_data), GFP_KERNEL);
    if(!driver_data)
        return -ENOMEM;

    /* Set this driver data in platform device structure */
	driver_data->data2 = driver_data->data1 = 12345;
    platform_set_drvdata(pdev, driver_data);

	// create char device driver node
	cdev_node_create(pdev);

	// create 
	ret = device_create_file(&exp_device.dev, &dev_attr_xyz_on);
	if (ret)
		printk("device_create_file xyz_on fail!\n");
	
	/* added debugfs */
	exp_debugfs_create(pdev);

    return 0;
}

static int exp_driver_remove(struct platform_device *pdev)
{
    struct exp_device_platform_data *exp_device_pdata = dev_get_platdata(&pdev->dev);;
    struct exp_driver_data *driver_data = platform_get_drvdata(pdev);

    // Power Off the device
    if (exp_device_pdata->power_off) {
        exp_device_pdata->power_off(exp_device_pdata);
    }

	// remove char device node
	cdev_node_remove();
	
	// remove debugfs
	exp_debugfs_remove(pdev);

    return 0;
}

// Power management operations
static const struct dev_pm_ops exp_device_pm_ops = {
    .suspend	= exp_device_pm_suspend,
    .resume		= exp_device_pm_resume,
};

//  Driver of example platfrom device
static struct platform_driver exp_driver = {
    .probe      = exp_driver_probe,
    .remove     = exp_driver_remove,
    .driver     = {
        .name	= EXP_DEVICE_NAME,
        .owner	= THIS_MODULE,
        .pm		= &exp_device_pm_ops,
    },
};

/* Module Init */
static int __init exp_driver_init_module(void)
{
    int ret;

	vibrator_power_on();

    platform_device_register(&exp_device);

#ifndef EXP_DRIVER_HOT_PLUIN
    ret = platform_driver_probe(&exp_driver, exp_driver_probe);
#else
    ret =  platform_driver_register(&exp_driver);
#endif

	// check if do 'exp_driver" and "exp_device" bind?
	printk(KERN_INFO "%s exp_device name=%s  exp_driver name=%s\n", __FUNCTION__, exp_device.dev.driver->name, exp_driver.driver.name);

    return ret;
}

/* Module Exit */ 
static void __exit exp_driver_exit_module(void)
{
    platform_driver_unregister(&exp_driver);
}

module_init(exp_driver_init_module);
module_exit(exp_driver_exit_module);

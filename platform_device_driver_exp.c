#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/slab.h>
#include "exp_device_data.h"


/*
**	This platform device driver will create device structure as:
**		/sys/devices/platform/exp_device
**			driver
**			modalias : platform:exp-platform-device
**			uevent
**			
*/


#define     EXP_DEVICE_NAME	 "exp_device"
#define		EXP_DRIVER_HOT_PLUIN	1

//
// Functions to create and remove char device node
//
extern void cdev_node_create(pdev);
extern void cdev_node_remove(pdev);

// 
// Functions to create and remove debugfs in platform_device_driver_debugfs.c
//
extern int exp_debugfs_create(struct platform_device *pdev);
extern int exp_debugfs_remove(struct platform_device *pdev);


// This example device uses dummy vibrator library fucntions as example
extern void vibrator_power_on(void);
extern void vibrator_power_off(void);


static void exp_pdevice_power_on(struct exp_device_platform_data* pdata)
{
	vibrator_power_on();
}

static void exp_pdevice_power_off(struct exp_device_platform_data* pdata)
{
	vibrator_power_off();
}

/* "exp device" platform data */
static struct exp_device_platform_data exp_pdevice_pdata = {
    .poweron_gpio   = 101,
	.state			= 0,
    .power_on		= exp_pdevice_power_on,
    .power_off		= exp_pdevice_power_off,
};

static struct resource exp_pdevice_resources[] = {
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

static struct platform_device exp_pdevice = {
	.name		= EXP_DEVICE_NAME,
    .id			= PLATFORM_DEVID_NONE,
    .dev.platform_data	= &exp_pdevice_pdata,
    .num_resources = ARRAY_SIZE(exp_pdevice_resources),
	.resource = exp_pdevice_resources,
};


/* Suspend the device. */
static int exp_pdevice_pm_suspend(struct device *dev)
{
    struct exp_device_driver_data* driver_data = dev_get_drvdata(dev);

    /* Suspend the device here*/

    return 0;
}

/* Resume the device. */
static int exp_pdevice_pm_resume(struct device *dev)
{
    struct exp_device_driver_data* driver_data = dev_get_drvdata(dev);

    /* Resume the device here*/

    return 0;
}

static int exp_pdriver_probe(struct platform_device *pdev)
{
    struct exp_device_platform_data *exp_pdevice_pdata;
    struct exp_device_driver_data* pdriver_data;
	int ret;

    exp_pdevice_pdata = dev_get_platdata(&pdev->dev);

    /* Try to power on the device. */
    if (exp_pdevice_pdata->power_on) {
        exp_pdevice_pdata->power_on(exp_pdevice_pdata);
    }

    /* Allocate the driver data here */
    pdriver_data = kzalloc(sizeof(struct exp_device_driver_data), GFP_KERNEL);
    if(!pdriver_data)
        return -ENOMEM;

    /* Set this driver data in platform device structure */
	pdriver_data->data2 = pdriver_data->data1 = 12345;
    platform_set_drvdata(pdev, pdriver_data);

	// create char device driver node
	cdev_node_create(pdev);

	// create debugfs for plarfrom device
	exp_debugfs_create(pdev);

    return 0;
}

static int exp_pdriver_remove(struct platform_device *pdev)
{
    struct exp_device_platform_data *exp_pdevice_pdata = dev_get_platdata(&pdev->dev);;
    struct exp_driver_data *pdriver_data = platform_get_drvdata(pdev);

    // Power Off the device
    if (exp_pdevice_pdata->power_off) {
        exp_pdevice_pdata->power_off(exp_pdevice_pdata);
    }

	// remove char device node
	cdev_node_remove(pdev);

	// remove debugfs
	exp_debugfs_remove(pdev);
	
    return 0;
}

// Power management operations
static const struct dev_pm_ops exp_pdevice_pm_ops = {
    .suspend	= exp_pdevice_pm_suspend,
    .resume		= exp_pdevice_pm_resume,
};

//  Driver of example platfrom device
static struct platform_driver exp_pdriver = {
    .probe      = exp_pdriver_probe,
    .remove     = exp_pdriver_remove,
    .driver     = {
        .name	= EXP_DEVICE_NAME,
        .owner	= THIS_MODULE,
        .pm		= &exp_pdevice_pm_ops,
    },
};

/* Module Init */
static int __init exp_pdriver_init_module(void)
{
    int ret;

	vibrator_power_on();

    platform_device_register(&exp_pdevice);

#ifndef EXP_DRIVER_HOT_PLUIN
    ret = platform_driver_probe(&exp_pdriver, exp_pdriver_probe);
#else
    ret =  platform_driver_register(&exp_pdriver);
#endif

	// check if do 'exp_driver" and "exp_device" bind?
	printk(KERN_INFO "%s exp_pdevice name=%s  exp_pdriver name=%s\n", __FUNCTION__, exp_pdevice.dev.driver->name, exp_pdriver.driver.name);

    return ret;
}

/* Module Exit */ 
static void __exit exp_pdriver_exit_module(void)
{
    platform_driver_unregister(&exp_pdriver);
}

module_init(exp_pdriver_init_module);
module_exit(exp_pdriver_exit_module);

/*
** Dummy vibrator device control functions
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/device.h>

extern void vibr_Enable_HW();
extern void vibr_Disable_HW();
extern void vibr_power_set();


void vibrator_enable(void)
{
	vibr_Enable_HW();
}

void vibrator_disable(void)
{
	vibr_Disable_HW();
}

void vibrator_power_on(void)
{
	vibr_power_set();
}

void vibrator_power_off(void)
{
}


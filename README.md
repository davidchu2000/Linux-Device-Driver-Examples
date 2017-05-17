# Linux-Device-Driver-Examples
This repository contains example code for Linux device drivers
List of files:
- platfrom_device_driver_exp.c : The main code for a platfrom device driver "exp" (means example)
- platform_device_driver_node.c: Code for creating a sysfs device node for this platform device.
- platfrom_device_driver_debugfs.c: Code for creating the debug fs for this platfrom device.
- exp_device_data.h: Platform device data and driver data.
- vibrator.c : Dummy control code for a hypothetic vibrator associated with this device.
- Makefile : Platform device driver makefile.
- expdevice.cpp : user-space program to call ioctl.
- exp_device_ioctl.h : ioctl command shared between user-space and kernel-space.
- Android.mk : makefile for user-space code. Used in Android AOSP /sys/core/expdevice directory.

// Copyright 2006-2015 The Android Open Source Project

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <iostream>
#include "exp_device_ioctl.h" 

int main(int argc, char **argv)
{
	int fd;
	unsigned long timer;
	struct exp_ioctl_arg args = {.timer = 300} ;


	if(argc > 3 || argc < 2)
	{
		perror("usage: expdevice [0|1|2|3 timer|4]");
		return -1;
	}


	fd = open("/dev/exp_device", O_RDWR);

	if(fd < 0)
	{
		perror("exp_device: open failed");
		return -1;
	}
 

	switch(*argv[1])
	{
		case '0':
			if (ioctl(fd, EXP_IOCTL_CMD_DISABLE,NULL) == -1)
       		 		perror("expdevice: failed to disable");
			break;
		case '1':
			if (ioctl(fd, EXP_IOCTL_CMD_ENABLE,NULL) == -1)
       		 		perror("expdevice: failed to enable");
			break;
		case '2':
			if (ioctl(fd, EXP_IOCTL_CMD_GET_TIMER,&args) == -1)
       		 		perror("expdevice: get timer failed");
			else
				printf("expdevice: timer = %d\n",args.timer);
			break;
		case '3':
			if(argv[2])
				args.timer = atoi(argv[2]);
			if (ioctl(fd, EXP_IOCTL_CMD_SET_TIMER,&args) == -1)
       		 		perror("expdevice: set timer failed");
			break;
		case '4':
			if (ioctl(fd, EXP_IOCTL_CMD_GET_DRVDATA,&args) == -1)
       		 		perror("expdevice: set timer failed");
			else
				printf("expdevice: driver data1=%d data2=%d\n", args.data1,args.data2);
			break;
		default:
       		 	perror("expdevice: not implemented");
			break;
	}

	close(fd); 

	return 0;
}

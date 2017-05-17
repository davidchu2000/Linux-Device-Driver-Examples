//
// Exp device IOCTL commands
// This file can be used in user space to make ioctl call
//

enum EXP_DEVICE_IOCTL_CMD
{
	EXP_CMD_GET_TIMER = 0,
	EXP_CMD_SET_TIMER,
	EXP_CMD_POWER_ON,
	EXP_CMD_POWER_OFF,
	EXP_CMD_ENABLE,
	EXP_CMD_DISABLE,
	EXP_CMD_GET_DRVDATA
};

struct exp_ioctl_arg
{
    int timer;
	int data1;
	int data2;
};
 
#define EXP_IOCTL_CMD_GET_TIMER 	_IOR('q', EXP_CMD_GET_TIMER, struct exp_ioctl_arg *)
#define EXP_IOCTL_CMD_SET_TIMER 	_IOW('q', EXP_CMD_SET_TIMER, struct exp_ioctl_arg *)
#define EXP_IOCTL_CMD_POWER_ON 		_IO('q', EXP_CMD_POWER_ON)
#define EXP_IOCTL_CMD_POWER_OFF 	_IO('q', EXP_CMD_POWER_OFF)
#define EXP_IOCTL_CMD_ENABLE 		_IO('q', EXP_CMD_ENABLE)
#define EXP_IOCTL_CMD_DISABLE 		_IO('q', EXP_CMD_DISABLE)
#define EXP_IOCTL_CMD_GET_DRVDATA 	_IOR('q', EXP_CMD_GET_DRVDATA, struct exp_ioctl_arg *)

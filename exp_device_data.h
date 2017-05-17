// Exp platform device data.
struct exp_device_platform_data {
	int poweron_gpio;
	int timer_msec;
	int state;
	void (*power_on)(struct exp_device_platform_data* ppdata);
	void (*power_off)(struct exp_device_platform_data* ppdata);
	void (*enable)(struct exp_device_platform_data* ppdata);
	void (*disable)(struct exp_device_platform_data* ppdata);
};


// Exp device driver data structure.
struct exp_driver_data {
    int data1;
    int data2;
};

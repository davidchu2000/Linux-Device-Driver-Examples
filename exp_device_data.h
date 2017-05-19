// Exp platform device data.
struct exp_device_platform_data {
	int poweron_gpio;
	int state;
	void (*power_on)(struct exp_device_platform_data* ppdata);
	void (*power_off)(struct exp_device_platform_data* ppdata);
};


// Exp device driver data structure.
struct exp_device_driver_data {
    int data1;
    int data2;
};

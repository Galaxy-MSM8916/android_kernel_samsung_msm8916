/*

 * Copyright (c) 2010-2011 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.


*/

#include "sim_slot.h"

#ifndef CONFIG_CHECK_SIMSLOT_COUNT_DT
#ifndef SIM_SLOT_PIN
	#error SIM_SLOT_PIN should be have a value. but not defined.
#endif
#endif

#ifdef CONFIG_CHECK_SIMSLOT_COUNT_DT
static int check_simslot_count_dt(struct seq_file *m, void *v)
{
	struct device_node *np = NULL;
	int simslot_pin, retval, support_number_of_simslot, low_is_dual, gpio_value;

	np = of_find_compatible_node(NULL, NULL, "simslot");

	simslot_pin = of_get_named_gpio_flags(np, "samsung,sim-slot", 0, NULL);
	if (simslot_pin < 0)
	{
		pr_err("***** Make a forced kernel panic because can't get pin number from sim-slot node ******\n");
		panic("kernel panic");
		return -EINVAL;
	}
	printk("%s:simslot_pin : %d\n", __func__, simslot_pin);  //temp log for checking GPIO Setting correctly applyed or not
	
	if(of_property_read_bool(np, "low-is-dual"))
		low_is_dual = 1;
	else
		low_is_dual = 0;
	printk("%s:low_is_dual : %d\n", __func__, low_is_dual);

	retval = gpio_request(simslot_pin, "sim-slot");
	if (retval) {
		pr_err("%s:Failed to reqeust GPIO, code = %d.\n",
			__func__, retval);
		panic("kernel panic");
	}

	retval = gpio_direction_input(simslot_pin);
	if (retval){
		pr_err("%s:Failed to set direction of GPIO, code = %d.\n",
			__func__, retval);
		panic("kernel panic");
	}
	
	/* If the value of sim-slot gpio is 'low' in 'dual sim' device, you must set 'low-is-dual' boolean property at simslot node in dt.*/
	
	printk("%s:SIM Check : ", __func__);
	
	if(!low_is_dual) // Daul sim device has high value.
	{
		gpio_value = gpio_get_value(simslot_pin);
		if(gpio_value == HIGH_VALUE)
		{
			printk("DUAL_SIM [%d]\n",gpio_value);
			support_number_of_simslot = DUAL_SIM;
		}
		else if(gpio_value == LOW_VALUE)
		{
			printk("SINGLE_SIM [%d]\n",gpio_value);
			support_number_of_simslot = SINGLE_SIM;
		}
		else
		{
			support_number_of_simslot = -1;
		}
	}
	else // Single sim device has high value.
	{
		gpio_value = gpio_get_value(simslot_pin);
		if(gpio_value == LOW_VALUE)
		{
			printk("DUAL_SIM [%d]\n",gpio_value);
			support_number_of_simslot = DUAL_SIM;
		}
		else if(gpio_value == HIGH_VALUE)
		{
			printk("SINGLE_SIM [%d]\n",gpio_value);
			support_number_of_simslot = SINGLE_SIM;
		}
		else
		{
			support_number_of_simslot = -1;
		}
	}

	gpio_free(simslot_pin);


	if(support_number_of_simslot < 0)
	{
		pr_err("******* Make a forced kernel panic because can't check simslot count******\n");
		panic("kernel panic");
	}

	seq_printf(m, "%u\n", support_number_of_simslot);

	return 0;

}

#else

static int check_simslot_count(struct seq_file *m, void *v)
{
	int retval, support_number_of_simslot;


	printk("%s:SIM_SLOT_PIN : %d\n", __func__, SIM_SLOT_PIN);  //temp log for checking GPIO Setting correctly applyed or not

	retval = gpio_request(SIM_SLOT_PIN,"SIM_SLOT_PIN");

	if (retval) {
			pr_err("%s:Failed to reqeust GPIO, code = %d.\n",
				__func__, retval);
			support_number_of_simslot = retval;
	}
	else
	{
		retval = gpio_direction_input(SIM_SLOT_PIN);

		if (retval){
			pr_err("%s:Failed to set direction of GPIO, code = %d.\n",
				__func__, retval);
			support_number_of_simslot = retval;
		}
		else
		{
			retval = gpio_get_value(SIM_SLOT_PIN);

			/* This codes are implemented assumption that count of GPIO about simslot is only one on H/W schematic
                           You may change this codes if count of GPIO about simslot has change */
			printk("%s:SIM Check : ", __func__);
			switch(retval)
			{
				case SINGLE_SIM_VALUE:
					printk("SINGLE_SIM [%d]\n",retval);
					support_number_of_simslot = SINGLE_SIM;
					break;
				case DUAL_SIM_VALUE :
					printk("DUAL_SIM [%d]\n",retval);
					support_number_of_simslot = DUAL_SIM;
					break;
				default :
					support_number_of_simslot = -1;
					break;
			}
		}
		gpio_free(SIM_SLOT_PIN);
	}

	if(support_number_of_simslot < 0)
	{
		pr_err("******* Make a forced kernel panic because can't check simslot count******\n");
		panic("kernel panic");
	}

	seq_printf(m, "%u\n", support_number_of_simslot);

	return 0;

}
#endif

static int check_simslot_count_open(struct inode *inode, struct file *file)
{
#ifdef CONFIG_CHECK_SIMSLOT_COUNT_DT
	return single_open(file, check_simslot_count_dt, NULL);
#else
	return single_open(file, check_simslot_count, NULL);
#endif
}

static const struct file_operations check_simslot_count_fops = {
	.open	= check_simslot_count_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release= single_release,
};

static int __init simslot_count_init(void)
{
	if(!proc_create("simslot_count",0,NULL,&check_simslot_count_fops))
	{
		pr_err("***** Make a forced kernel panic because can't make a simslot_count file node ******\n");
		panic("kernel panic");
		return -ENOMEM;
	}
	else return 0;
}

late_initcall(simslot_count_init);

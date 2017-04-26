 /*
  * sm5701_irq.c
  *
  * Copyright (c) 2014 Silicon Mitus Co., Ltd
  *
  *  This program is free software; you can redistribute  it and/or modify it
  *  under  the terms of  the GNU General  Public License as published by the
  *  Free Software Foundation;  either version 2 of the  License, or (at your
  *  option) any later version.
  *
  */
#include <linux/module.h>
#include <linux/moduleparam.h> 
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/mfd/sm5701_core.h>
#include <linux/gpio.h>

/*
static irqreturn_t sm5701_irq_thread(int irq, void *data)
{
    //printk("******* %s *******\n",__func__);

	return IRQ_HANDLED;
}
*/
  
int sm5701_irq_init(struct SM5701_dev *sm5701)
{ 
    //printk("******* %s *******\n",__func__);
    
    return 0;
    
}
EXPORT_SYMBOL(sm5701_irq_init);
 
void sm5701_irq_exit(struct SM5701_dev *sm5701)
{
    //printk("******* %s *******\n",__func__);
}
EXPORT_SYMBOL(sm5701_irq_exit); 

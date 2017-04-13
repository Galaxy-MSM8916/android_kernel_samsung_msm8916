
#ifndef _ZINITIX_I2C_H_
#define _ZINITIX_I2C_H_

 extern struct tsp_callbacks *charger_callbacks;
 struct tsp_callbacks {
	 void (*inform_charger)(struct tsp_callbacks *, int);
 };

#endif

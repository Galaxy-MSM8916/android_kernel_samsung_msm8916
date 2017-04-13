#ifndef _HALL_H
#define _HALL_H

//extern struct class *sec_class;
struct hall_platform_data {
	unsigned int rep:1;		/* enable input subsystem auto repeat */
	int gpio_flip_cover;
};
extern struct device *sec_key;

#endif /* _HALL_H */

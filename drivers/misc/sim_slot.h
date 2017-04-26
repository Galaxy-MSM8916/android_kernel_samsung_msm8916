#ifdef CONFIG_CHECK_SIMSLOT_COUNT

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/i2c-gpio.h>
#include <asm/gpio.h>
#ifdef CONFIG_CHECK_SIMSLOT_COUNT_DT
#include <linux/of_gpio.h>
#else
/* below values would be change by H/W schematic of each model */
#ifdef CONFIG_CHECK_SIMSLOT_COUNT_GPIO
#define SIM_SLOT_PIN CONFIG_CHECK_SIMSLOT_COUNT_GPIO
#else
	# CONFIG_CHECK_SIMSLOT_COUNT_GPIO is not defined
#endif
#endif

#define SINGLE_SIM_VALUE 0
#define DUAL_SIM_VALUE 1

#ifdef CONFIG_CHECK_SIMSLOT_COUNT_DT
#define HIGH_VALUE 1
#define LOW_VALUE 0
#endif

enum
{
        NO_SIM = 0,
        SINGLE_SIM,
        DUAL_SIM,
        TRIPLE_SIM
};

#endif

#define CAPACITY_MAX			1000
#define CAPACITY_MAX_MARGIN     30
#define CAPACITY_MIN			0

#if defined(CONFIG_MACH_GT58_USA_TMO) || defined(CONFIG_MACH_GT58_CAN_BMC)
static sec_bat_adc_table_data_t temp_table[] = {
	{700, 900},
	{680, 850},
	{660, 800},
	{640, 750},
	{616, 700},
	{584, 650},
	{569, 620},
	{560, 600},
	{554, 590},
	{547, 580},
	{544, 570},
	{528, 550},
	{513, 530},
	{499, 510},
	{497, 500},
	{483, 490},
	{476, 480},
	{462, 460},
	{454, 450},
	{445, 440},
	{409, 400},
	{363, 350},
	{315, 300},
	{264, 250},
	{212, 200},
	{158, 150},
	{136, 130},
	{104, 100},
	{83, 80},
	{62, 60},
	{51, 50},
	{32, 30},
	{13, 10},
	{-3, 0},
	{-13, -10},
	{-31, -30},
	{-50, -50},
	{-67, -70},
	{-91, -100},
	{-128, -150},
	{-161, -200},
};
#else
static sec_bat_adc_table_data_t temp_table[] = {
	{700, 900},
	{680, 850},
	{660, 800},
	{640, 750},
	{616, 700},
	{590, 650},
	{562, 600},
	{531, 550},
	{497, 500},
	{454, 450},
	{420, 400},
	{371, 350},
	{322, 300},
	{272, 250},
	{218, 200},
	{165, 150},
	{115, 100},
	{56, 50},
	{6, 0},
	{-32, -50},
	{-76, -100},
	{-110, -150},
	{-149, -200},
};
#endif

#if defined(CONFIG_MACH_GT58_USA_TMO) || defined(CONFIG_MACH_GT58_CAN_BMC)
#define TEMP_HIGH_THRESHOLD_EVENT  595
#define TEMP_HIGH_RECOVERY_EVENT   535
#define TEMP_LOW_THRESHOLD_EVENT   (-40)
#define TEMP_LOW_RECOVERY_EVENT    10
#define TEMP_HIGH_THRESHOLD_NORMAL 595
#define TEMP_HIGH_RECOVERY_NORMAL  535
#define TEMP_LOW_THRESHOLD_NORMAL  (-40)
#define TEMP_LOW_RECOVERY_NORMAL   10
#define TEMP_HIGH_THRESHOLD_LPM    600
#define TEMP_HIGH_RECOVERY_LPM     545
#define TEMP_LOW_THRESHOLD_LPM     (-35)
#define TEMP_LOW_RECOVERY_LPM      0
#else
#define TEMP_HIGH_THRESHOLD_EVENT  550
#define TEMP_HIGH_RECOVERY_EVENT   500
#define TEMP_LOW_THRESHOLD_EVENT   (-50)
#define TEMP_LOW_RECOVERY_EVENT    0
#define TEMP_HIGH_THRESHOLD_NORMAL 550
#define TEMP_HIGH_RECOVERY_NORMAL  500
#define TEMP_LOW_THRESHOLD_NORMAL  (-50)
#define TEMP_LOW_RECOVERY_NORMAL   0
#define TEMP_HIGH_THRESHOLD_LPM    550
#define TEMP_HIGH_RECOVERY_LPM     500
#define TEMP_LOW_THRESHOLD_LPM     (-50)
#define TEMP_LOW_RECOVERY_LPM      0
#endif

#if defined(CONFIG_BATTERY_SWELLING)
#define BATT_SWELLING_HIGH_TEMP_BLOCK		450
#define BATT_SWELLING_HIGH_TEMP_RECOV		400
#define BATT_SWELLING_LOW_TEMP_BLOCK		100
#define BATT_SWELLING_LOW_TEMP_RECOV		150
#define BATT_SWELLING_RECHG_VOLTAGE		4150
#endif

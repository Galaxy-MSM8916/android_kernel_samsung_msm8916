#define CAPACITY_MAX			980
#define CAPACITY_MAX_MARGIN     30
#define CAPACITY_MIN			0

int Temperature_fn(void)
{
	return 0;
}

static struct battery_data_t stc3117_battery_data[] = {
	{
		.Vmode= 0,		 /*REG_MODE, BIT_VMODE 1=Voltage mode, 0=mixed mode */
		.Alm_SOC = 1,		/* SOC alm level %*/
		.Alm_Vbat = 3000,	/* Vbat alm level mV*/
		.CC_cnf = 1352,		/* nominal CC_cnf, coming from battery characterisation*/
		.VM_cnf = 1370,		/* nominal VM cnf , coming from battery characterisation*/
		.Rint = 200,		/* nominal internal impedance*/
		.Cnom = 6800,		/* nominal capacity in mAh, coming from battery characterisation*/
		.Rsense = 10,		/* sense resistor mOhms*/
		.RelaxCurrent = 340, /* current for relaxation in mA (< C/20) */
		.Adaptive = 1,	   /* 1=Adaptive mode enabled, 0=Adaptive mode disabled */

		/* Elentec Co Ltd Battery pack - 80 means 8% */
		.CapDerating[6] = 267,				/* capacity derating in 0.1%, for temp = -20C */
		.CapDerating[5] = 144,				/* capacity derating in 0.1%, for temp = -10C */
		.CapDerating[4] = 72,			   /* capacity derating in 0.1%, for temp = 0C */
		.CapDerating[3] = 59,			   /* capacity derating in 0.1%, for temp = 10C */
		.CapDerating[2] = 0,			  /* capacity derating in 0.1%, for temp = 25C */
		.CapDerating[1] = 0,			  /* capacity derating in 0.1%, for temp = 40C */
		.CapDerating[0] = 0,			  /* capacity derating in 0.1%, for temp = 60C */

		.OCVValue[15] = 4290,			 /* OCV curve adjustment */
		.OCVValue[14] = 4180,			 /* OCV curve adjustment */
		.OCVValue[13] = 4084,			 /* OCV curve adjustment */
		.OCVValue[12] = 3984,			 /* OCV curve adjustment */
		.OCVValue[11] = 3959,			 /* OCV curve adjustment */
		.OCVValue[10] = 3899,			 /* OCV curve adjustment */
		.OCVValue[9] = 3838,			 /* OCV curve adjustment */
		.OCVValue[8] = 3802,			 /* OCV curve adjustment */
		.OCVValue[7] = 3775,			 /* OCV curve adjustment */
		.OCVValue[6] = 3758,			 /* OCV curve adjustment */
		.OCVValue[5] = 3739,			 /* OCV curve adjustment */
		.OCVValue[4] = 3698,			 /* OCV curve adjustment */
		.OCVValue[3] = 3688,			 /* OCV curve adjustment */
		.OCVValue[2] = 3682,			 /* OCV curve adjustment */
		.OCVValue[1] = 3617,			 /* OCV curve adjustment */
		.OCVValue[0] = 3300,			 /* OCV curve adjustment */

		/*if the application temperature data is preferred than the STC3117 temperature*/
		.ExternalTemperature = Temperature_fn, /*External temperature fonction, return C*/
		.ForceExternalTemperature = 0, /* 1=External temperature, 0=STC3117 temperature */
	}
};


static sec_bat_adc_table_data_t temp_table[] = {
	{25664, 900},
	{26219, 850},
	{26677, 800},
	{27457, 750},
	{27909, 700},
	{28369, 650},
	{28841, 600},
	{29237, 550},
	{29648, 500},
	{30543, 450},
	{31288, 400},
	{32028, 350},
	{32804, 300},
	{33667, 250},
	{34525, 200},
	{35425, 150},
	{36291, 100},
	{37160, 50},
	{37625, 0},
	{38419, -50},
	{39268, -100},
	{40073, -150},
	{40897, -200},
};

#define TEMP_HIGHLIMIT_THRESHOLD_EVENT		800
#define TEMP_HIGHLIMIT_RECOVERY_EVENT		750
#define TEMP_HIGHLIMIT_THRESHOLD_NORMAL		800
#define TEMP_HIGHLIMIT_RECOVERY_NORMAL		750
#define TEMP_HIGHLIMIT_THRESHOLD_LPM		800
#define TEMP_HIGHLIMIT_RECOVERY_LPM		750

#define TEMP_HIGH_THRESHOLD_EVENT  600
#define TEMP_HIGH_RECOVERY_EVENT   460
#define TEMP_LOW_THRESHOLD_EVENT   (-50)
#define TEMP_LOW_RECOVERY_EVENT    0
#define TEMP_HIGH_THRESHOLD_NORMAL 520
#define TEMP_HIGH_RECOVERY_NORMAL  460
#define TEMP_LOW_THRESHOLD_NORMAL  (-50)
#define TEMP_LOW_RECOVERY_NORMAL   0
#define TEMP_HIGH_THRESHOLD_LPM    520
#define TEMP_HIGH_RECOVERY_LPM     460
#define TEMP_LOW_THRESHOLD_LPM     (-50)
#define TEMP_LOW_RECOVERY_LPM      0

#if defined(CONFIG_BATTERY_SWELLING)
#define BATT_SWELLING_HIGH_TEMP_BLOCK		550
#define BATT_SWELLING_HIGH_TEMP_RECOV		460
#define BATT_SWELLING_LOW_TEMP_BLOCK		100
#define BATT_SWELLING_LOW_TEMP_RECOV		150
#define BATT_SWELLING_RECHG_VOLTAGE		4150
#define BATT_SWELLING_BLOCK_TIME	10 * 60 /* 10 min */
#endif

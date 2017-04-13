#define CAPACITY_MAX			1000
#define CAPACITY_MAX_MARGIN     50
#define CAPACITY_MIN			0


int Temperature_fn(void)
{
	return 0;
}

static struct battery_data_t stc3117_battery_data[] = {
	{
		.Vmode= 0,		 /*REG_MODE, BIT_VMODE 1=Voltage mode, 0=mixed mode */
		.Alm_SOC = 1,		/* SOC alm level %*/
		.Alm_Vbat = 3400,	/* Vbat alm level mV*/
		.CC_cnf = 389,      /* nominal CC_cnf, coming from battery characterisation*/
		.VM_cnf = 384,      /* nominal VM cnf , coming from battery characterisation*/
		.Rint = 195,		/* nominal internal impedance*/
		.Cnom = 1850,       /* nominal capacity in mAh, coming from battery characterisation*/
		.Rsense = 10,       /* sense resistor mOhms*/
		.RelaxCurrent = 93, /* current for relaxation in mA (< C/20) */
		.Adaptive = 1,     /* 1=Adaptive mode enabled, 0=Adaptive mode disabled */

		/* Elentec Co Ltd Battery pack - 80 means 8% */
		.CapDerating[6] = 267,            /* capacity derating in 0.1%, for temp = -20°C */
		.CapDerating[5] = 144,            /* capacity derating in 0.1%, for temp = -10°C */
		.CapDerating[4] = 72,             /* capacity derating in 0.1%, for temp = 0°C */
		.CapDerating[3] = 59,             /* capacity derating in 0.1%, for temp = 10°C */
		.CapDerating[2] = 0,              /* capacity derating in 0.1%, for temp = 25°C */
		.CapDerating[1] = 0,             /* capacity derating in 0.1%, for temp = 40°C */
		.CapDerating[0] = 0,              /* capacity derating in 0.1%, for temp = 60°C */

		.OCVValue[15] = 4363,            /* OCV curve adjustment */
		.OCVValue[14] = 4246,            /* OCV curve adjustment */
		.OCVValue[13] = 4131,            /* OCV curve adjustment */
		.OCVValue[12] = 4028,            /* OCV curve adjustment */
		.OCVValue[11] = 3984,            /* OCV curve adjustment */
		.OCVValue[10] = 3936,            /* OCV curve adjustment */
		.OCVValue[9] = 3844,             /* OCV curve adjustment */
		.OCVValue[8] = 3804,             /* OCV curve adjustment */
		.OCVValue[7] = 3774,             /* OCV curve adjustment */
		.OCVValue[6] = 3757,             /* OCV curve adjustment */
		.OCVValue[5] = 3737,             /* OCV curve adjustment */
		.OCVValue[4] = 3699,             /* OCV curve adjustment */
		.OCVValue[3] = 3676,             /* OCV curve adjustment */
		.OCVValue[2] = 3669,             /* OCV curve adjustment */
		.OCVValue[1] = 3582,             /* OCV curve adjustment */
		.OCVValue[0] = 3400,             /* OCV curve adjustment */

		/*if the application temperature data is preferred than the STC3117 temperature*/
		.ExternalTemperature = Temperature_fn, /*External temperature fonction, return C*/
		.ForceExternalTemperature = 0, /* 1=External temperature, 0=STC3117 temperature */
	}
};


static sec_bat_adc_table_data_t temp_table[] = {
	{25950, 900},
	{26173, 850},
	{26520, 800},
	{26790, 750},
	{27158, 700},
	{27664, 650},
	{27930, 620},
	{28147, 600},
	{28379, 580},
	{28828, 550},
	{29019, 530},
	{29296, 510},
	{29574, 490},
	{29727, 480},
	{30003, 460},
	{30338, 440},
	{30614, 410},
	{30934, 380},
	{31494, 370},
	{32103, 350},
	{32802, 300},
	{33752, 250},
	{34872, 200},
	{35975, 150},
	{36602, 130},
	{37224, 100},
	{37632, 80},
	{38309, 50},
	{38716, 30},
	{39074, 10},
	{39275, 0},
	{39452, -10},
	{39783, -30},
	{40087, -50},
	{40319, -70},
	{40804, -100},
	{41425, -150},
	{41940, -200},
};

#define TEMP_HIGH_THRESHOLD_EVENT  620
#define TEMP_HIGH_RECOVERY_EVENT   490
#define TEMP_LOW_THRESHOLD_EVENT   (-50)
#define TEMP_LOW_RECOVERY_EVENT    1
#define TEMP_HIGH_THRESHOLD_NORMAL 525
#define TEMP_HIGH_RECOVERY_NORMAL  470
#define TEMP_LOW_THRESHOLD_NORMAL  (-50)
#define TEMP_LOW_RECOVERY_NORMAL   5
#define TEMP_HIGH_THRESHOLD_LPM    515
#define TEMP_HIGH_RECOVERY_LPM     490
#define TEMP_LOW_THRESHOLD_LPM     (-30)
#define TEMP_LOW_RECOVERY_LPM      0


#ifndef __GCMD
#define __GCMD
	
#define GCMD_COMMAND_G0                	0
#define GCMD_COMMAND_G1                	1
#define GCMD_COMMAND_G28               	28
#define GCMD_COMMAND_G90               	90
#define GCMD_COMMAND_G91               	91
#define GCMD_COMMAND_G92               	92

#define GCMD_COMMAND_M3             		1003
#define GCMD_COMMAND_M5             		1005
#define GCMD_COMMAND_M92             		1092


#define GCMD_PARAMETR_X                 	'X'
#define GCMD_PARAMETR_Y                 	'Y'
#define GCMD_PARAMETR_Z                 	'Z'

#define GCMD_METRIC_X2STEP_MODE4        	10
#define GCMD_METRIC_Y2STEP_MODE4        	10

#define GCMD_XAXSIS_MOTOR_CH					1
#define GCMD_YAXSIS_MOTOR_CH					0


#define GCMD_POS_MODE_ABSOLUTE				0
#define GCMD_POS_MODE_RELATIVE				1

extern int gMachineInit(void);
extern int gMachine(int cmd,int *key,double *value,int len);

#endif


#include "gcmd.h"
#include <stdio.h>
#include "stepper.h"
#include "LPC17xx.h"
#include "cmsis_os.h"
#include "servo.h"

static double G_x_pos,G_y_pos;
static int	  G_pos_mode;

static int stepMove(double dx,double dy);

static int M3(void);
static int M5(void);
static int G0(double x, double y);
static int G1(double x, double y);
static int G28(void);
static int G28x(void);
static int G28y(void);
static int G28xy(void);
static int G90(void);
static int G91(void);
static int G92x(double x);
static int G92y(double y);
static int G92xy(double x,double y);

int gMachineInit(void){
	G_x_pos=0;
	G_y_pos=0;
	G_pos_mode=GCMD_POS_MODE_ABSOLUTE;
	LPC_TIM0->TCR=2;
	LPC_TIM0->MR0=1000;
	LPC_TIM0->PR=25-1;
	LPC_TIM0->MCR=3;
	LPC_TIM0->TCR=1;
	NVIC_EnableIRQ(TIMER0_IRQn);
	stepperConfig(0,1);
	stepperConfig(1,1);
	servoInit();
	servoOn(0);
	servoMove(0,-45);
	return 0;
}

int gMachine(int cmd,int *key,double *value,int len){
	int isX,isY,gpv0;
	int stat=-1;
	double x=0;
	double y=0;
	isX=0;
	isY=0;
	for(gpv0=0;gpv0<len;gpv0++){
		if(key[gpv0]==GCMD_PARAMETR_X){isX=1;x=value[gpv0];}
		if(key[gpv0]==GCMD_PARAMETR_Y){isY=1;y=value[gpv0];} //in this version only x,y supported and pen location set by g1 and g0
	}

	if		  ((isX==1 || isY==1) && cmd==GCMD_COMMAND_G0) 	 	 G0(x,y);
	else	if((isX==1 || isY==1) && cmd==GCMD_COMMAND_G1)	    G1(x,y);
	else	if(cmd==GCMD_COMMAND_M3)	 M3();	
	else	if(cmd==GCMD_COMMAND_M5)	 M5();	
	else	if(cmd==GCMD_COMMAND_G90)	 G90();	
	else	if(cmd==GCMD_COMMAND_G91)	 G91();
	else	if(cmd==GCMD_COMMAND_G92){
		if(isX && isY)				G92xy(x,y);
		else if(isX)				G92x(x);
		else if(isY)				G92y(y);
		else           			G92xy(0,0);
	}
	else  if(cmd==GCMD_COMMAND_G28){
		if(isX && isY)				G28xy();
		else if(isX)				G28x();
		else if(isY)				G28y();
		else							G28();
	}
	
	return stat;
}

////////////////////////////////////////////////////////////////////////////////

static int M3(void){
	servoMove(0,40);	
	osDelay(3000);
}
static int M5(void){
	servoMove(0,-45);	
	osDelay(3000);
}

static int G0(double x, double y){
	printf("moveG0 %f %f <real move %f %f> ",x,y,x-G_x_pos,y-G_y_pos);

	
	if(G_pos_mode==GCMD_POS_MODE_ABSOLUTE){stepMove(x-G_x_pos,y-G_y_pos); G_x_pos=x; G_y_pos=y;}	
	else											  { G_x_pos+=x; G_y_pos+=y; stepMove(x,y);}	
	printf("<%f><%f>\n",G_x_pos,G_y_pos);
	return 0;
}
static int G1(double x, double y){
	printf("moveG1 %f %f <real move %f %f> ",x,y,x-G_x_pos,y-G_y_pos);

	
	if(G_pos_mode==GCMD_POS_MODE_ABSOLUTE){stepMove(x-G_x_pos,y-G_y_pos); G_x_pos=x; G_y_pos=y;}	
	else											  { G_x_pos+=x; G_y_pos+=y; stepMove(x,y);}		
	printf("<%f><%f>\n",G_x_pos,G_y_pos);
	return 0;
}

static int G28(void){
	G_x_pos=0;
	G_y_pos=0;
	printf("moveG28 <%f><%f>\n",G_x_pos,G_y_pos);
	return 0;
}
static int G28x(void){
	G_x_pos=0;
	printf("moveG28 <%f><%f>\n",G_x_pos,G_y_pos);
	return 0;
}
static int G28y(void){
	G_x_pos=0;
	printf("moveG28 <%f><%f>\n",G_x_pos,G_y_pos);
	return 0;
}
static int G28xy(void){
	G_x_pos=0;
	G_y_pos=0;
	printf("moveG28 <%f><%f>\n",G_x_pos,G_y_pos);
	return 0;
}

static int G90(void){
	G_pos_mode=GCMD_POS_MODE_ABSOLUTE;
	printf("absolute mode selected\n");
	return 0;
}
static int G91(void){
	G_pos_mode=GCMD_POS_MODE_RELATIVE;
	printf("realtive mode selected\n");
	return 0;
}

static int G92xy(double x,double y){
	printf("moveG92 %f %f ",x,y);
	G_x_pos=x;
	G_y_pos=y;
	printf("<%f><%f>\n",G_x_pos,G_y_pos);
	return 0;
}


static int G92x(double x){
	printf("moveG92 %f ",x);
	G_x_pos=x;
	printf("<%f><%f>\n",G_x_pos,G_y_pos);
	return 0;
}
static int G92y(double y){
	printf("moveG92 %f ",y);
	G_y_pos=y;
	printf("<%f><%f>\n",G_x_pos,G_y_pos);
	return 0;	
}


void TIMER0_IRQHandler(void){
	stepperUpdate();
	LPC_TIM0->IR=1;
}

static int stepMove(double dx,double dy){
	int xstep,ystep,xdir,ydir,x,ycnt,yc;
	double M,m;
	if(dx<0)	{xdir=1; dx*=-1;}
	else		xdir=-1;
	
	if(dy<0)	{ydir=-1; dy*=-1;}
	else		ydir=1;

	xstep=(int)(dx*GCMD_METRIC_X2STEP_MODE4+0.5);
	ystep=(int)(dy*GCMD_METRIC_Y2STEP_MODE4+0.5);
	
	if(xstep==0) 		stepperMove(GCMD_YAXSIS_MOTOR_CH,(unsigned int)ystep,ydir);
	else if(ystep==0) stepperMove(GCMD_XAXSIS_MOTOR_CH,(unsigned int)xstep,xdir);
	else{
		//stepperMove(GCMD_YAXSIS_MOTOR_CH,(unsigned int)ystep,ydir);
		//stepperMove(GCMD_XAXSIS_MOTOR_CH,(unsigned int)xstep,xdir);
		M=(double)ystep/(double)xstep;
		m=M;
		ycnt=0;
		for(x=0;x<xstep;x++){
			stepperMove(GCMD_XAXSIS_MOTOR_CH,1,xdir);
			yc=(int)m;
			stepperMove(GCMD_YAXSIS_MOTOR_CH,(unsigned int)yc,ydir);
			m=M+m-yc;
			ycnt+=yc;
			while(stepperIsBusy(GCMD_XAXSIS_MOTOR_CH) || stepperIsBusy(GCMD_YAXSIS_MOTOR_CH)) osDelay(2);
		}
		if(ycnt<ystep) stepperMove(GCMD_YAXSIS_MOTOR_CH,(unsigned int)(ystep-ycnt),ydir);
	}
	while(stepperIsBusy(GCMD_XAXSIS_MOTOR_CH)>0 || stepperIsBusy(GCMD_YAXSIS_MOTOR_CH)>0) osDelay(2);
	osDelay(20);
	return xstep+ystep;
}


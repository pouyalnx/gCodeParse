#include <stdio.h>

extern void StepperConfig(int ch);
extern int  StepperMove(int ch,int steps);
extern int  StepperStop(int ch); 
extern int  StepperIsBusy(int ch);
extern int  StepperEnable(int ch);
extern int  StepperDisable(int ch); 


extern int  IOInit(void);
extern int  IOXIsEnd(void);
extern int  IOYIsEnd(void);
extern int  IOXIsStart(void);
extern int  IOYIsStart(void);
extern void osDelay(unsigned int d);


#define IO_X_CHANNEL                    0
#define IO_Y_CHANNEL                    1

#define GCODE_ERROR_ENDLINE             1
#define GCODE_ERROR_SEMICOLUMN          2
#define GCODE_ERROR_NULL                3
#define GCODE_ERROR_FORMAT              4
#define GCODE_ERROR_UNSUPPORTED_CMD     5
#define LOC(i)  (i-'A')
/////////////////////////////////////////////////////////////////////////////////
void IoDelayMs(unsigned int ms){
    osDelay(ms);
}


/////////////////////////////////////////////////////////////////////////////////
//  unit system it helps to convert between metrics
//  upgrades    1)u can add sepping mode
//
//
/////////////////////////////////////////////////////////////////////////////////
#define UNIT_X2STEP_MM              20.0
#define UNIT_Y2STEP_MM              20.0

static  double __x2unit;
static  double __y2unit;

void unitInit(void){
    __x2unit=UNIT_X2STEP_MM;
    __y2unit=UNIT_Y2STEP_MM;
}

static int  unitX2Step(double x){
    if(x<0){
        x*-=1;
        return int(x*__x2unit+0.5)*-1;
    }
    return int(x*__x2unit+0.5);
}

static int  unitY2Step(double y){
    if(y<0){
        y*-=1;
        return int(y*__y2unit+0.5)*-1;
    }
    return int(y*__y2unit+0.5);
}

static void unitSetMM(void){
    __x2unit=UNIT_X2STEP_MM;
    __y2unit=UNIT_Y2STEP_MM;
}


static void unitSetInch(void){
    __x2unit=UNIT_X2STEP_MM*25.4;
    __y2unit=UNIT_Y2STEP_MM*25.4;
}
////////////////////////////////////////////////////////////////////////////////////
#define POSITIOM_MODE_ABSOLUTE 0
#define POSITIOM_MODE_RELATIVE 1

static double __pos_x;
static double __pos_y;
static int    __pos_mode;


static void positionInit(void){
    __pos_x=0;
    __pos_y=0;
    __pos_mode=POSITIOM_MODE_ABSOLUTE;
}

static void positionSetRelative(void){
    __pos_mode=POSITIOM_MODE_RELATIVE;
}

static void positionSetAbsolute(void){
    __pos_mode=POSITIOM_MODE_ABSOLUTE;
}

static void positionSetX(double x){
    __pos_x=x;
}

static void positionSetY(double y){
    __pos_y=y;
}

static double positionMoveX(double x,int apply_on_mem){
    double val;
    if(__pos_mode==POSITIOM_MODE_ABSOLUTE){
        val=x-__pos_x;
        if(apply_on_mem) __pos_x=x;
    }
    else{
        val=x;
        if(apply_on_mem) __pos_x+=x;
    }
    return val;
}

static double positionMoveY(double y,int apply_on_mem){
    double val;
    if(__pos_mode==POSITIOM_MODE_ABSOLUTE){
        val=y-__pos_y;
        if(apply_on_mem) __pos_y=y;
    }
    else{
        val=y;
        if(apply_on_mem) __pos_y+=y;
    }
    return val;
}


///////////////////////////////////////////////////////////////////////////////////

int G20(char **out){
    unitSetInch();
    if(*out!=NULL) (*out)+=sprintf(*out,", unit=inch");
    return 0;
}

int G21(char **out){
    unitSetMM();
    if(*out!=NULL) (*out)+=sprintf(*out,", unit=mm");
    return 0;
}


/////////////////////////////////////////////////////////////////////////////////////
//  improve for future use limited step for example 50cm per hardware and if it can
//  reach value raise hardware error
/////////////////////////////////////////////////////////////////////////////////////
int G28(int *key,int *value,char **out){
    int flg_none_selected=1;
    if(key[LOC('X')]){
        while(IOXIsStart()==0){
            StepperMove(IO_X_CHANNEL,-1);
            while(StepperIsBusy(IO_X_CHANNEL)) IoDelayMs(1);
        }
        positionSetX(0);
        if(*out!=NULL) (*out)+=sprintf(*out,", X=0");
        flg_none_selected=0;
    }

    if(key[LOC('Y')]){
        while(IOYIsStart()==0){
            StepperMove(IO_Y_CHANNEL,-1);
            while(StepperIsBusy(IO_Y_CHANNEL)) IoDelayMs(1);
        }
        positionSetY(0);
        if(*out!=NULL) (*out)+=sprintf(*out,", Y=0");
        flg_none_selected=0;
    }

    if(flg_none_selected){
        
        while(IOXIsStart()==0){
            StepperMove(IO_X_CHANNEL,-1);
            while(StepperIsBusy(IO_X_CHANNEL)) IoDelayMs(1);
        }

        while(IOYIsStart()==0){
            StepperMove(IO_Y_CHANNEL,-1);
            while(StepperIsBusy(IO_Y_CHANNEL)) IoDelayMs(1);
        }        
        
        positionSetX(0);
        positionSetY(0);
        if(*out!=NULL) (*out)+=sprintf(*out,", X=0, Y=0");
    }

    return 0;
}

int G90(char **out){
    positionSetAbsolute();
    if(*out!=NULL) (*out)+=sprintf(*out,", mode=absolute");
    return 0;
}

int G91(char **out){
    positionSetRelative();
    if(*out!=NULL) (*out)+=sprintf(*out,", mode=relative");
    return 0;
}


int G92(int *key,int *value,char **out){
    int flg_none_selected=1;
    
    if(key[LOC('X')]){
        positionSetX(value[LOC('X')]);
        if(*out!=NULL) (*out)+=sprintf(*out,", X=%f",value[LOC('X')]);
        flg_none_selected=0;
    }
    if(key[LOC('Y')]){
        positionSetY(value[LOC('Y')]);
        if(*out!=NULL) (*out)+=sprintf(*out,", Y=%f",value[LOC('Y')]);
        flg_none_selected=0;
    }
    
    if(flg_none_selected)){
        positionSetX(value[LOC('X')]);
        if(*out!=NULL) (*out)+=sprintf(*out,", X=%f",value[LOC('X')]);
        
        positionSetY(value[LOC('Y')]);
        if(*out!=NULL) (*out)+=sprintf(*out,", Y=%f",value[LOC('Y')]);
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
int Gparse(int command,int *key,int *value,char **out){
    int upcode=command>>24;
    int code=command&0xffffff;
    int stat=0;
    if(upcode=='G'){
        switch (code)
        {
        case 20:
            stat=G20(out);
            break;
        case 21:
            stat=G21(out);
            break; 
        case 90:
            stat=G90(out);
            break; 
        case 91:
            stat=G91(out);
            break;        
        case 92:
            stat=G92(key,value,out);
            break;
        default:
            stat=GCODE_ERROR_UNSUPPORTED_CMD;
            if(*out!=NULL) (*out)+=sprintf(*out,", ERROR_UNSUPPORTED_CMD");
            break;
        }
    }
    else if(upcode=='M'){


    }
    return stat;
}


void Ginit(void){
    positionInit();
    unitInit();
    IOInit();
    StepperConfig(IO_X_CHANNEL);
    StepperConfig(IO_Y_CHANNEL);
    StepperEnable(IO_X_CHANNEL);
    StepperEnable(IO_Y_CHANNEL);
}

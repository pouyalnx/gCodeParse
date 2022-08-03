#include "gcode.h"
#include <stdio.h>


static int getCmd(char **data){
    char *ptr;
    int opcode;
    ptr=*data;
    while(*ptr==' ') ptr++;

    if(*ptr=='\0'){
        *data=ptr;
        return GCODE_ERROR_NULL;
    }
    else if(*ptr=='\n'){
        *data=ptr;
        return GCODE_ERROR_ENDLINE;
    }
    else if(*ptr==';'){
        *data=ptr;
        return GCODE_ERROR_SEMICOLUMN;
    }
    else if(*ptr=='G'){
        ptr++;
        opcode=0;
        while(*ptr>='0' && *ptr<='9'){
            opcode=10*opcode+*ptr-'0';
            ptr++;
        }
        *data=ptr+1;
        if(opcode==0)  return GCODE_COMMAND_G0;
        if(opcode==1)  return GCODE_COMMAND_G1;
        if(opcode==28) return GCODE_COMMAND_G28;
        if(opcode==90) return GCODE_COMMAND_G90;
        if(opcode==91) return GCODE_COMMAND_G91;
        if(opcode==92) return GCODE_COMMAND_G92;
    }
    else if(*ptr=='M'){
        ptr++;
        opcode=0;
        while(*ptr>='0' && *ptr<='9'){
            opcode=10*opcode+*ptr-'0';
            ptr++;
        }
        *data=ptr+1;
		  if(opcode==3)	return GCODE_COMMAND_M3;
		  if(opcode==5)	return GCODE_COMMAND_M5;
        if(opcode==92)  return GCODE_COMMAND_M92;
    }
    return GCODE_ERROR_UNSUPPORTED_CMD;
}


static int getPar(char **data,double *parameter){
    int flg;
    double val;
    double dig=0.1;
    char *ptr;
    int par;
    int sign=1;
    ptr=*data;
    while(*ptr==' ') ptr++;   
    if(*ptr=='\0'){
        *data=ptr;
        return GCODE_ERROR_NULL;
    }
    else if(*ptr=='\n'){
        *data=ptr;
        return GCODE_ERROR_ENDLINE;
    }
    else if(*ptr==';'){
        *data=ptr;
        return GCODE_ERROR_SEMICOLUMN;
    }
    
    par=*ptr;
    ptr++;
    val=0;
    flg=0;
    sign=1;
    if(*ptr=='-') {sign=-1;ptr++;}

    while((*ptr>='0' && *ptr<='9' )|| *ptr=='.'){
        if(*ptr=='.'){
            if(flg==0) flg=1;
            else return GCODE_ERROR_FORMAT;
        }
        else if(flg==0) val=10*val+*ptr-'0';
        else{      val+=(*ptr-'0')*dig; dig/=10;}        
        ptr++;
    }    
    *data=ptr;
    
    *parameter=val*sign;
    
    return par;
}


static int nextLine(char **data){
    char *ptr;
    ptr=*data;
    while(*ptr!='\n' && *ptr!='\0') ptr++;
    *data=ptr;
    if(*ptr=='\0') return GCODE_ERROR_NULL;
    //while(*ptr=='\n')    ptr++; //going nextLine
    if(*ptr=='\n')    ptr++; //going nextLine just go one next line no need to go through all empty lines
    *data=ptr;
    return GCODE_ERROR_ENDLINE;
}


int gcodeRun(char *data,int run,char *out,int maxlen){
    int     cmd;
    int     par;
    double   parf;
    int     ptr;
    int     keys[GCODE_LIMIT_MAX_ARGS];
    double   values[GCODE_LIMIT_MAX_ARGS];

    while(*data!='\0'){
        cmd=getCmd(&data);
        if(cmd==GCODE_ERROR_NULL) return 0;
        else if (cmd==GCODE_ERROR_SEMICOLUMN)       nextLine(&data);
        else if (cmd==GCODE_ERROR_ENDLINE)          nextLine(&data);
        else if (cmd==GCODE_ERROR_UNSUPPORTED_CMD ) nextLine(&data);
        else if (cmd==GCODE_ERROR_FORMAT)           nextLine(&data);    
        else{
            ptr=0;
            while(1){
                par=getPar(&data,&parf);
                if(par==GCODE_ERROR_SEMICOLUMN)      {nextLine(&data);break;}
                else if(par==GCODE_ERROR_ENDLINE)    {nextLine(&data);break;}
                else if(par==GCODE_ERROR_FORMAT)     {nextLine(&data);break;}
                else if(par==GCODE_ERROR_NULL)       {nextLine(&data);break;}
                else{
                    keys[ptr]=par;
                    values[ptr]=parf;
                    ptr++;
                    if(ptr==GCODE_LIMIT_MAX_ARGS) {nextLine(&data);break;}
                }
            }


			#ifdef __GCMD
                if(run){
					gMachine(cmd,keys,values,ptr);
				}
			#endif
				
            printf("(%d -",cmd);
            for(int i=0;i<ptr;i++) printf("(%c,%f)",keys[i],values[i]);
            printf(")\n");
        }
    }
	 return 0;
}

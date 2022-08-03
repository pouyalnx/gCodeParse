#include <stdio.h>


#define LOC(i)  (i-'A')

#define GCODE_ERROR_ENDLINE             1
#define GCODE_ERROR_SEMICOLUMN          2
#define GCODE_ERROR_NULL                3
#define GCODE_ERROR_FORMAT              4
#define GCODE_ERROR_UNSUPPORTED_CMD     5


static double value[26];
static int    key[26];



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int getCmd(char **data){
    char *ptr;
    int op;
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

    else if(*ptr>='A' && *ptr<='Z'){
        op=(*ptr)<<24;
        ptr++;
        opcode=0;
        if(*ptr<'0' || *ptr>'9') return GCODE_ERROR_UNSUPPORTED_CMD;
        while(*ptr>='0' && *ptr<='9'){
            opcode=10*opcode+*ptr-'0';
            ptr++;
        }
        *data=ptr;
        if(*ptr!=' ' && *ptr!='\n' && *ptr!='\0' && *ptr!=';') return GCODE_ERROR_UNSUPPORTED_CMD;
        if(opcode&0xff000000)                                  return GCODE_ERROR_UNSUPPORTED_CMD;
        return opcode|op;
    }
    return GCODE_ERROR_UNSUPPORTED_CMD;
}

static int getPar(char **data,double *parameter){
    int flg;
    double val;
    double dig=0.1;
    char *ptr;
    int sign=1;
    int id;
    ptr=*data;
    *parameter=0;
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
    else if((*ptr>='A' && *ptr<='Z') || (*ptr>='a' && *ptr<='z')){
        id=*ptr;
        ptr++;
        if(*ptr==' ' || *ptr=='\n' || *ptr=='\0' || *ptr==';') return id;

        if(*ptr!='-' && *ptr!='.' && (*ptr<'0' || *ptr>'9')) return GCODE_ERROR_FORMAT;

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
            else{           val+=(*ptr-'0')*dig; dig/=10;}        
            ptr++;
        }    
        *data=ptr;
        *parameter=val*sign;

        if(*ptr==' ' || *ptr=='\n' || *ptr=='\0' || *ptr==';') return id;
    }
    return GCODE_ERROR_FORMAT;
}


static int nextLine(char **data){
    char *ptr;
    ptr=*data;
    while(*ptr!='\n' && *ptr!='\0') ptr++;
    *data=ptr;
    if(*ptr=='\0') return GCODE_ERROR_NULL;
    if(*ptr=='\n')    ptr++; //going nextLine just go one next line no need to go through all empty lines
    *data=ptr;
    return GCODE_ERROR_ENDLINE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

int gCodeExecLine(char **gcode,int run,char *out){
    int     cmd;
    int     par;
    double   parf;
    
    if(out!=NULL) *out=0;
    for(int i='A';i<='Z';i++) key[LOC(i)]=0;
    cmd=getCmd(gcode);
    if      (cmd==GCODE_ERROR_NULL)              cmd=0;
    else if (cmd==GCODE_ERROR_SEMICOLUMN)        cmd=0;
    else if (cmd==GCODE_ERROR_ENDLINE)           cmd=0;
    else if (cmd==GCODE_ERROR_UNSUPPORTED_CMD)   {cmd=GCODE_ERROR_UNSUPPORTED_CMD; if(out!=NULL)  out+=sprintf(out,"UNSUPPORTED_CMD");}
    else if (cmd==GCODE_ERROR_FORMAT)            {cmd=GCODE_ERROR_FORMAT;          if(out!=NULL)  out+=sprintf(out,"CMD_FORMAT");}  
    else{
        while(1){
            par=getPar(gcode,&parf);
            if(par==GCODE_ERROR_SEMICOLUMN)      break;
            else if(par==GCODE_ERROR_ENDLINE)    break;
            else if(par==GCODE_ERROR_NULL)       break;
            else if(par==GCODE_ERROR_FORMAT)     {cmd=GCODE_ERROR_FORMAT;if(out!=NULL) out+=sprintf(out,"PARAMETER_FORMAT");break;}
            else if(par>='A' && par<='Z')        {key[LOC(par)]=1; value[LOC(par)]=parf;}
            else                                 {cmd=GCODE_ERROR_FORMAT;if(out!=NULL) out+=sprintf(out,"UNSUPPORTED_PARAMETER");break;}
            }
        }        
    nextLine(gcode);
    
    if(cmd&0xff000000){ if(out!=NULL) out+=sprintf(out,", OK");}
    else              { if(out!=NULL) out+=sprintf(out,", ERROR");} 
    
    if(out!=NULL) out+=sprintf(out,"\n");
    return cmd;
}


int gCodeExec(char *gcode,int run,char *buff){
    int stat;
    puts(gcode);
    while(*gcode!='\0') {
        stat=gCodeExecLine(&gcode,run,buff);
        if(buff!=NULL)
            printf("%s",buff);
    }
    return stat;
}

char resp[512]="\n";
char cmd[]="M5 S0 \n"
"G90 \n"
"G21 \n"
"G1 F3000 \n"
"Gz1 X150.4727 Y93.6107 \n"
"G4 P0 \n"
"M5 Sc0 \n"
"G1 F3000 \n"
"G1 X147770...0.6874 Y92.5728 \n"
"G4 P=0 \n"
"M3 S2/55 \n"
"G4 P0 \n"
"G1 F750 \n";

int main(){
    char *s;
    int upcode;
    s=cmd;  //error in many versions u cant send address of array holder variable
    printf("lnx embroidery 2022\n");
    //printf("%s\n",cmd);
    //printf("cmd=%d &cmd=%d *cmd=%d\n",s,&s,*s);

    //while(*s!='\0'){
    //upcode=gCodeExecLine(&s,1,resp);
    //printf("retcode=%d\n",upcode);
    //printf("%s\n",resp);
//
 //   printf("<<<<<<%c%d>>>>>>>\n",upcode>>24,upcode&0xffffff);
  //  for(int i='A';i<='Z';i++)
   //     if(key[LOC(i)]) printf("%c[%d]=%f\n",i,key[LOC(i)],value[LOC(i)]);
   // printf("-------------------------------------------------------------\n");
   // }


    gCodeExec(s,1,resp);
    return 0;

}
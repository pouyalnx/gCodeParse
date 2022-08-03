#ifndef __GCODE
#define __GCODE


#define GCODE_LIMIT_MAX_ARGS            8

#define GCODE_COMMAND_G0                0
#define GCODE_COMMAND_G1                1
#define GCODE_COMMAND_G28               28
#define GCODE_COMMAND_G90               90
#define GCODE_COMMAND_G91               91
#define GCODE_COMMAND_G92               92
#define GCODE_COMMAND_M3             	1003
#define GCODE_COMMAND_M5             	1005
#define GCODE_COMMAND_M92             1092


#define GCODE_PARAMETR_X                 'X'
#define GCODE_PARAMETR_Y                 'Y'
#define GCODE_PARAMETR_Z                 'Z'


#define GCODE_ERROR_ENDLINE             -1
#define GCODE_ERROR_SEMICOLUMN          -2
#define GCODE_ERROR_NULL                -3
#define GCODE_ERROR_FORMAT              -4
#define GCODE_ERROR_UNSUPPORTED_CMD     -5




/*

*/
extern int gcodeRun(char *data,int run,char *out,int maxlen);

#endif

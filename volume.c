/*
 * =====================================================================================
 *
 *       Filename:  volume.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/29/11 14:34:23
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Tom Xue (), tom.xue@nokia.com
 *        Company:  Nokia
 *
 * =====================================================================================
 */
#include <stdio.h>

char myCmd[500] = "dbus-send --print-reply --type=method_call --dest=com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer /com/nokia/mafw/renderer/gstrenderer com.nokia.mafw.extension.get_extension_property string:volume|awk '/nt/ {print $3}' > volume_value";

//#define half_period 	30	//40, 50, 60: about Vpp=89V Vamp=88V and 58us/period
				//30: about Vpp=80V Vamp=78V and 42us/period
				//20: about Vpp=60V Vamp=54V and 30us/period
				//10: about Vpp=46V Vamp=28V and 16us/period
//#define times		100	//repeate times of period
//#define sleep_time	2000	//3000: about 3ms

int main(int argc,char *argv[])
{
    FILE *streamCmd;
    int bufCmd[1];
    int volume;

	while(1){
    streamCmd = popen(myCmd,"r");
	usleep(100000);
    //memset(bufCmd, 0, sizeof(bufCmd));
    //fread(bufCmd, sizeof(int), sizeof(bufCmd), streamCmd);
    //volume = atoi(bufCmd);
    //printf("Current volume of the phone is %d\n",volume);
	}
}

#include <sys/types.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <memory.h>

//run on N900, to drive XTI_RX(UART1_RX, J5602.7, ETK_D10) GPIO_24
#define GPIO_BASE 			0x48002000
//GPIO_24 register address, the resigter is 32-bit
#define GPIO_24_OFFSET_LOWER 		0x5f0
#define GPIO_27_OFFSET_HIGHER		0x5f4
#define GPIO_29_OFFSET_HIGHER		0x5f8
#define GPIO_120_OFFSET_LOWER		0x144
#define GPIO_122_OFFSET_LOWER		0x148
#define GPIO_124_OFFSET_LOWER		0x14c

//General-Purpose Interface Integration Figure, GPIO5: GPIO_[159:128]
#define GPIO24  0x01000000			//GPIO24
#define GPIO27	0x08000000			//GPIO27
#define GPIO29	0x20000000			//GPIO29
#define GPIO120 0x01000000
#define GPIO122	0x04000000
#define GPIO124	0x10000000

#define GPIO1_BASE 		0x48310000	//P3606
#define GPIO4_BASE		0x49054000	//P3606 GPIO[127:96]
#define GPIO1_OE_OFFSET 	0x034		//P3606, Output Data Enable Register
#define GPIO4_OE_OFFSET		0x034
#define GPIO1_DATAOUT_OFFSET	0x03c		//P3606, Data Out register
#define GPIO4_DATAOUT_OFFSET	0x03c

#define INT *(volatile unsigned int*)

/*
 * IEN  - Input Enable
 * IDIS - Input Disable
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 */

#define IEN     (1 << 8)
#define IDIS    (0 << 8)
#define PTU     (1 << 4)
#define PTD     (0 << 4)
#define EN      (1 << 3)
#define DIS     (0 << 3)

#define M0      0
#define M1      1
#define M2      2
#define M3      3
#define M4      4
#define M5      5
#define M6      6
#define M7      7 

//void below means the pointer points to byte data, if e.g. unsigned int *map_base
//then should be: INT(map_base+GPIO_138_OFFSET_LOWER/4) = padconf;
void *map_base;  
int fd,i,j,loop_times;
unsigned int padconf;
char myCmd[500] = "dbus-send --print-reply --type=method_call --dest=com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer /com/nokia/mafw/renderer/gstrenderer com.nokia.mafw.extension.get_extension_property string:volume|awk '/nt/ {print $3}'";

//#define half_period 	30	//40, 50, 60: about Vpp=89V Vamp=88V and 58us/period
				//30: about Vpp=80V Vamp=78V and 42us/period
				//20: about Vpp=60V Vamp=54V and 30us/period
				//10: about Vpp=46V Vamp=28V and 16us/period
//#define times		100	//repeate times of period
//#define sleep_time	2000	//3000: about 3ms

int main(int argc,char *argv[])
{
    FILE *theVolumeValue;
    int bufCmd[1];
    int volume;
    int half_period = 50;
    int times = 100;
    int sleep_time = 2000;

    /*theVolumeValue = fopen("volume_value","r");
    memset(bufCmd, 0, sizeof(bufCmd));
    fread(bufCmd, sizeof(int), sizeof(bufCmd), theVolumeValue);
    volume = atoi(bufCmd);
    printf("Current volume of the phone is %d\n",volume);
    */
    loop_times = 0;

    if((fd=open("/dev/mem",O_RDWR | O_SYNC))==-1){
        perror("open error!\n");
        return(-1);
    }

    printf("fd=%d\n",fd);

    //Set the pinmux to select the GPIO signal
    map_base = mmap(0,0x200,PROT_READ | PROT_WRITE,MAP_SHARED,fd,GPIO_BASE);
    printf("GPIO_BASE map_base=%p\n",map_base);
    //GPIO120
    padconf = INT(map_base+GPIO_120_OFFSET_LOWER);
    padconf &= 0xFFFF0000; //[15:0]=GPIO_120  - Clear register bits [15:0]
    padconf |= 0x0000001c; //[15:0]=GPIO_120  - Select mux mode 4 for gpio
    INT(map_base+GPIO_120_OFFSET_LOWER) = padconf; 
    printf("GPIO_120_OFFSET_LOWER - The register value is set to: 0x%x = 0d%u\n", padconf,padconf);    
    //GPIO122
    padconf = INT(map_base+GPIO_122_OFFSET_LOWER);
    padconf &= 0xFFFF0000; //[15:0]=GPIO_122  - Clear register bits [15:0]
    padconf |= 0x0000001c; //[15:0]=GPIO_122  - Select mux mode 4 for gpio
    INT(map_base+GPIO_122_OFFSET_LOWER) = padconf; 
    printf("GPIO_122_OFFSET_LOWER - The register value is set to: 0x%x = 0d%u\n", padconf,padconf); 
    //GPIO124
    padconf = INT(map_base+GPIO_124_OFFSET_LOWER);
    padconf &= 0xFFFF0000; //[15:0]=GPIO_124  - Clear register bits [15:0]
    padconf |= 0x0000001c; //[15:0]=GPIO_124  - Select mux mode 4 for gpio
    INT(map_base+GPIO_124_OFFSET_LOWER) = padconf; 
    printf("GPIO_124_OFFSET_LOWER - The register value is set to: 0x%x = 0d%u\n", padconf,padconf); 
    munmap(map_base,0x200);


    //GPIO120/122/124 - GPIO4: Set the OE and DATAOUT registers
    map_base = mmap(0,0x40,PROT_READ | PROT_WRITE,MAP_SHARED,fd,GPIO4_BASE);
    printf("GPIO4_BASE map_base=%p\n",map_base);    
    //OE
    padconf = INT(map_base+GPIO4_OE_OFFSET);
    padconf &= ~(GPIO120 | GPIO122 | GPIO124);  // Set GPIO_120/122/124 to output
    INT(map_base+GPIO4_OE_OFFSET) = padconf; 
    printf("GPIO4_OE_OFFSET - The register value is set to: 0x%x = 0d%u\n", padconf,padconf);
    //DATAOUT
    padconf = INT(map_base+GPIO4_DATAOUT_OFFSET);
    padconf |=  (GPIO120 | GPIO122 | GPIO124);  //Set GPIO_120/122/124 high
    //padconf &= ~(GPIO120 | GPIO122 | GPIO124);  //Set GPIO_120/122/124 low
    INT(map_base+GPIO4_DATAOUT_OFFSET) = padconf; 
    printf("GPIO4_DATAOUT_OFFSET - The register value is set to: 0x%x = 0d%u\n", padconf,padconf);


    //Hello world!
    while(1){
	//padconf ^=  GPIO24;  // Toggle GPIO_24
	//INT(map_base+GPIO1_DATAOUT_OFFSET) = padconf;
	for(j=0;j<times;j++){
		for(i=0;i<half_period;i++){
    		padconf |=  (GPIO120 | GPIO122 | GPIO124);  //Set GPIO_120/122/124 high
    		INT(map_base+GPIO4_DATAOUT_OFFSET) = padconf;
		printf("padconf = 0x%x\n", padconf);
		}

		for(i=0;i<half_period;i++){
		padconf &= ~(GPIO120 | GPIO122 | GPIO124);  //Set GPIO_120/122/124 low
		INT(map_base+GPIO4_DATAOUT_OFFSET) = padconf; 
		printf("padconf = 0x%x\n", padconf);
		}
	}

	usleep(sleep_time);
/*   
	loop_times = loop_times + 1;
	if(loop_times == 10){ //below part consume about 20ms, so influence the performance of this program 
		loop_times = 0;
		theVolumeValue = fopen("volume_value","r");
    		//memset(bufCmd, 0, sizeof(bufCmd));
    		fread(bufCmd, sizeof(int), sizeof(bufCmd), theVolumeValue);
    		volume = atoi(bufCmd);
		if(volume<0 || volume <100)
			volume = 50;
		times = volume;	
		half_period = (int)(volume*0.4);
		sleep_time = (int)(20000-volume*190);
	}*/
    } 	

    close(fd);
    munmap(map_base,0x40);
}

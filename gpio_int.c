/*
 * =====================================================================================
 *
 *       Filename:  gpio_interrupt.c
 *
 *    Description:  basic interrupt trial program
 *
 *        Version:  1.0
 *        Created:  11/27/11 10:04:25
 *       Revision:  none
 *       Compiler:  gcc
 *       Platform:  BB-xM-RevC, Ubuntu 3.0.4-x3
 *
 *         Author:  Tom Xue (), tom.xue@nokia.com
 *        Company:  Nokia
 *
 * =====================================================================================
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/interrupt.h> //request_irq 
#include <linux/gpio.h>	//OMAP_GPIO_IRQ
#include <plat/mux.h>	//omap_cfg_reg
#include <linux/irq.h>	//IRQ_TYPE_LEVEL_LOW

#define times		20
#define half_period	15	

/*
Test result on Adapter Board No.1:

times: 10 		pulse width 1.8ms
times: 20		pulse width 3.5ms
times: 40		pulse width 7.2ms
half_period: 50  	pulse amplitude 119.8V
half_period: 25		pulse amplitude 143.8V
half_period: 20		pulse amplitude 151.9V
half_period: 17		pulse amplitude 153.9V
half_period: 15		pulse amplitude 156.0V
half_period: 10		pulse amplitude 156.0V
half_period: 8		pulse amplitude 143.6V
half_period: 7		pulse amplitude 129.2V
half_period: 5		pulse amplitude 99.28V
half_period: 4		pulse amplitude 77.51V
half_period: 3		pulse amplitude 60.82V
*/

MODULE_LICENSE("GPL");

//MMC2_DAT6, P9-pin5, system default GPIO input with pullup
#define OMAP3_GPIO138           138
#define OMAP3_GPIO139		139
int irq;

static irqreturn_t my_interrupt(){
	int i, j;
	//printk(KERN_ALERT "my_interrupt executed!\n");
	for(j=0;j<times;j++){
		for(i=0;i<half_period;i++){
		gpio_direction_output(OMAP3_GPIO139, 1);
		}

		for(i=0;i<half_period;i++){
		gpio_direction_output(OMAP3_GPIO139, 0);
		}
	}
}

static int hello_init(void) {
	int ret;

	ret = gpio_request(OMAP3_GPIO139, "OMAP3_GPIO139");
	if(ret < 0)
		printk(KERN_ALERT "gpio_request of GPIO139 failed!\n");
	gpio_direction_output(OMAP3_GPIO139, 0);

	//below function: Sets the Omap MUX and PULL_DWN registers based on the table and judge 'cpu_class_is_omap1'
	//omap_cfg_reg(OMAP3_GPIO138);
	ret = gpio_request(OMAP3_GPIO138, "OMAP3_GPIO138");
	if(ret < 0)
		printk(KERN_ALERT "gpio_request of GPIO139 failed!\n");
		
	gpio_direction_input(OMAP3_GPIO138);	
	
	irq = OMAP_GPIO_IRQ(OMAP3_GPIO138);	//irq33 <-> GPIO module 5: includes gpio_138
	printk(KERN_ALERT "OMAP_GPIO_IRQ success! The irq = %d\n", irq);
	
	irq_set_irq_type(irq, IRQ_TYPE_EDGE_RISING);
	enable_irq(gpio_to_irq(OMAP3_GPIO138));
	
	ret = request_irq(irq, my_interrupt, IRQF_DISABLED, "my_interrupt_proc", NULL);
	if (ret==0)
        printk(KERN_ALERT "request_irq success!\n");
    else
        printk(KERN_ALERT "request_irq fail!\n"); 
		
    printk(KERN_ALERT "Hello, Tom Xue! From inside kernel driver!\n");
    return 0;
}

static void hello_exit(void)
{
    disable_irq(irq);
    free_irq(irq, NULL);
    gpio_free(OMAP3_GPIO139);
    gpio_free(OMAP3_GPIO138);
    printk(KERN_INFO "Goodbye, Tom Xue! From inside kernel driver!\n");
}

module_init(hello_init);
module_exit(hello_exit);



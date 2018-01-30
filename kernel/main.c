#include <stdio.h>
#include <stdio.h>
#include <s3c24xx.h>
#include "serial.h"
#include "lcdlib.h"
#include "lcddrv.h"
#include "framebuffer.h"
#include "command.h"
int main() {
	nand_init();
	Port_Init();
	uart0_init();							// ������115200��8N1(8������λ����У��λ��1��ֹͣλ)
	timer_init();

	Lcd_Port_Init();						// ����LCD����
	Tft_Lcd_Init(MODE_TFT_16BIT_480272);	// ��ʼ��LCD������
	Lcd_PowerEnable(0, 1);					// ����LCD_PWREN��Ч�������ڴ�LCD�ĵ�Դ
	Lcd_EnvidOnOff(1);						// ʹ��LCD����������ź�
	{
		ClearScr(0x00);						// ����
		lcd_set_text_color(0xffffff);
		lcd_set_background_color(0x00);
		printf("Initializing UART...\n");
		printf("Initializing GPIO ports...\n");
		printf("Initializing the LCD controller...\n");

		init_page_map();
		kmalloc_init();

		init_yaffs_fs();

		enable_irq();
		//usb_init_slave();
		task_init();
		//cmd_loop();
		while (1);
	}
	Lcd_EnvidOnOff(0);
	return 0;
}

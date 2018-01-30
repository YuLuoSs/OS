#include "s3c24xx.h"
#include "serial.h"
#include "framebuffer.h"
#define TXD0READY   (1<<2)
#define RXD0READY   (1)

#define PCLK            50000000    // init.c�е�clock_init��������PCLKΪ50MHz
#define UART_CLK        PCLK        //  UART0��ʱ��Դ��ΪPCLK
#define UART_BAUD_RATE  115200      // ������
#define UART_BRD        ((UART_CLK  / (UART_BAUD_RATE * 16)) - 1)

/*
 * ��ʼ��UART0
 * 115200,8N1,������
 */
void uart0_init(void)
{
    GPHCON  |= 0xa0;    // GPH2,GPH3����TXD0,RXD0
    GPHUP   = 0x0c;     // GPH2,GPH3�ڲ�����

    ULCON0  = 0x03;     // 8N1(8������λ���޽��飬1��ֹͣλ)
    UCON0   = 0x05;     // ��ѯ��ʽ��UARTʱ��ԴΪPCLK
    UFCON0  = 0x00;     // ��ʹ��FIFO
    UMCON0  = 0x00;     // ��ʹ������
    UBRDIV0 = UART_BRD; // ������Ϊ115200
}



/*
 * ����һ���ַ�
 */
static void __serial_putc(unsigned char c)
{
    while (!(UTRSTAT0 & TXD0READY));/* �ȴ���ֱ�����ͻ������е������Ѿ�ȫ�����ͳ�ȥ */
    UTXH0 = c;/* ��UTXH0�Ĵ�����д�����ݣ�UART���Զ��������ͳ�ȥ */
}

/*
 * �����ַ�
 */
static unsigned char __serial_getc(void)
{
    while (!(UTRSTAT0 & RXD0READY));/* �ȴ���ֱ�����ջ������е������� */
    return URXH0;/* ֱ�Ӷ�ȡURXH0�Ĵ��������ɻ�ý��յ������� */
}

void serial_putc(unsigned char c)
{
    __serial_putc(c);
	if(c=='\b'){
		__serial_putc(' ');
		__serial_putc('\b');
	}
		
}

unsigned char serial_getc(void)
{
    return __serial_getc();
}
/*
 * init.c: ����һЩ��ʼ��
 */ 

#include "s3c24xx.h"
 
void disable_watch_dog(void);
void clock_init(void);
void memsetup(void);
void copy_steppingstone_to_sdram(void);
void clean_bss(void);

/*
 * �ر�WATCHDOG������CPU�᲻������
 */
void disable_watch_dog(void)
{
    WTCON = 0;  // �ر�WATCHDOG�ܼ򵥣�������Ĵ���д0����
}

#define FCLK        200000000
#define HCLK        100000000
#define PCLK        50000000
#define S3C2410_MPLL_200MHZ     ((0x5c<<12)|(0x04<<4)|(0x00))
#define S3C2440_MPLL_200MHZ     ((0x5c<<12)|(0x01<<4)|(0x02))
#define S3C2440_MPLL_400MHZ     ((0x5c<<12)|(0x01<<4)|(0x01))
#define S3C2440_UPLL_48MHZ      ((0x38<<12)|(0x02<<4)|(0x02))
#define S3C2440_UPLL_96MHZ      ((0x38<<12)|(0x02<<4)|(0x01))
/*
 * ����MPLLCON�Ĵ�����[19:12]ΪMDIV��[9:4]ΪPDIV��[1:0]ΪSDIV
 * �����¼��㹫ʽ��
 *  S3C2410: MPLL(FCLK) = (m * Fin)/(p * 2^s)
 *  S3C2410: MPLL(FCLK) = (2 * m * Fin)/(p * 2^s)
 *  ����: m = MDIV + 8, p = PDIV + 2, s = SDIV
 * ���ڱ������壬Fin = 12MHz
 * ����CLKDIVN�����Ƶ��Ϊ��FCLK:HCLK:PCLK=1:4:8��
 * FCLK=400MHz,HCLK=100MHz,PCLK=50MHz
 */
void clock_init(void)
{
    // LOCKTIME = 0x00ffffff;   // ʹ��Ĭ��ֵ����
    //CLKDIVN  = 0x03;            // FCLK:HCLK:PCLK=1:2:4, HDIVN=1,PDIVN=1
	CLKDIVN  = 0x05;            // FCLK:HCLK:PCLK=1:4:8
    /* ���HDIVN��0��CPU������ģʽӦ�ôӡ�fast bus mode����Ϊ��asynchronous bus mode�� */
	__asm__ volatile (
		"mrc    p15, 0, r1, c1, c0, 0\n"        /* �������ƼĴ��� */ 
		"orr    r1, r1, #0xc0000000\n"          /* ����Ϊ��asynchronous bus mode�� */
		"mcr    p15, 0, r1, c1, c0, 0\n"        /* д����ƼĴ��� */
		:::"r1"
    );
	/*
	����ͬʱ���� MPLL �� UPLL ��ֵʱ��������������� UPLL ֵ������ MPLL ֵ������Լ��Ҫ 7 �� NOP �ļ����
	*/
	//UPLLCON = S3C2440_UPLL_48MHZ;
	
	MPLLCON = S3C2440_MPLL_400MHZ;  /* ���ڣ�FCLK=400MHz,HCLK=100MHz,PCLK=50MHz */    
}
/*
 * ����ICACHE
 */
void enable_ICACNE(void)
{
    __asm__ volatile (
		"mrc    p15, 0, r0, c1, c0, 0\n"		/* �������ƼĴ��� */ 
		"orr    r0, r0, #(1<<12)\n"
		"mcr    p15, 0, r0, c1, c0, 0\n"	/* д����ƼĴ��� */
		:::"r0"
    );
}
/*
 * ���ô洢��������ʹ��SDRAM
 */
void memsetup(void)
{
    volatile unsigned long *p = (volatile unsigned long *)MEM_CTL_BASE;

    /* �������֮����������ֵ����������ǰ���ʵ��(����mmuʵ��)����������ֵ
     * д�������У�����ΪҪ���ɡ�λ���޹صĴ��롱��ʹ��������������ڱ����Ƶ�
     * SDRAM֮ǰ�Ϳ�����steppingstone������
     */
    /* �洢������13���Ĵ�����ֵ */
    p[0] = 0x22011110;     //BWSCON
    p[1] = 0x00000700;     //BANKCON0
    p[2] = 0x00000700;     //BANKCON1
    p[3] = 0x00000700;     //BANKCON2
    p[4] = 0x00000700;     //BANKCON3  
    p[5] = 0x00000700;     //BANKCON4
    p[6] = 0x00000700;     //BANKCON5
    p[7] = 0x00018005;     //BANKCON6
    p[8] = 0x00018005;     //BANKCON7
    
                                    /* REFRESH,
                                     * HCLK=12MHz:  0x008C07A3,
                                     * HCLK=100MHz: 0x008C04F4
                                     */ 
    p[9]  = 0x008C04F4;
    p[10] = 0x000000B1;     //BANKSIZE
    p[11] = 0x00000030;     //MRSRB6
    p[12] = 0x00000030;     //MRSRB7
}

void copy_steppingstone_to_sdram(void)
{
    unsigned int *pdwSrc  = (unsigned int *)0;
    unsigned int *pdwDest = (unsigned int *)0x30000000;
    
    while (pdwSrc < (unsigned int *)4096)
    {
        *pdwDest = *pdwSrc;
        pdwDest++;
        pdwSrc++;
    }
}

void clean_bss(void)
{
    extern int __bss_start, __bss_end;
    int *p = &__bss_start;
    
    for (; p < &__bss_end; p++)
        *p = 0;
}

int is_boot_from_nor_flash(void) {
	volatile int *p = (volatile int *)0;
	int val;

	val = *p;
	*p = 0x12345678;
	if (*p == 0x12345678) {
		/* д�ɹ�, ��nand���� */
		*p = val;
		return 0;
	} else {
		/* NOR�������ڴ�һ��д */
		return 1;
	}
}

int copy_code_to_sdram(unsigned char *buf,unsigned int *addr , unsigned int len)
{
    extern void nand_read_ll(unsigned char *buf,unsigned int addr , unsigned int len);
	int i = 0;

	/* �����NOR���� */
	if (is_boot_from_nor_flash()) {
		while (i < len) {
			buf[i] = addr[i];
			i++;
		}
	} else {
		//nand_init();
		nand_read_ll(buf, (unsigned int)addr, len);
	}
    return 0;
}

/*
 * ����ҳ��
 */
void create_page_table(void)
{

/* 
 * ���ڶ���������һЩ�궨��
 */ 
#define MMU_FULL_ACCESS     (3 << 10)   /* ����Ȩ�� */
#define MMU_DOMAIN          (0 << 5)    /* �����ĸ��� */
#define MMU_SPECIAL         (1 << 4)    /* ������1 */
#define MMU_CACHEABLE       (1 << 3)    /* cacheable */
#define MMU_BUFFERABLE      (1 << 2)    /* bufferable */
#define MMU_SECTION         (2)         /* ��ʾ���Ƕ������� */
#define MMU_SECDESC         (MMU_FULL_ACCESS | MMU_DOMAIN | MMU_SPECIAL | \
                             MMU_SECTION)
#define MMU_SECDESC_WB      (MMU_FULL_ACCESS | MMU_DOMAIN | MMU_SPECIAL | \
                             MMU_CACHEABLE | MMU_BUFFERABLE | MMU_SECTION)
#define MMU_SECTION_SIZE    0x00100000
#define MUM_TLB_BASE_ADDR	0x30000000
#define MUM_SECTION_PADDR_BASE_MASK	(0xfff00000)
#define PHYSICAL_MEM_ADDR	0x30000000
#define VIRTUAL_MEM_ADDR	0x30000000
#define MEM_MAP_SIZE		0x4000000
#define PHYSICAL_IO_ADDR	0x48000000
#define VIRTUAL_IO_ADDR		0x48000000
#define IO_MAP_SIZE			0x18000000

    unsigned long virtuladdr, physicaladdr;
    volatile unsigned long *mmu_tlb_base = (volatile unsigned long *)MUM_TLB_BASE_ADDR;
    
    /*
     * Steppingstone����ʼ�����ַΪ0����һ���ֳ������ʼ���е�ַҲ��0��
     * Ϊ���ڿ���MMU���������е�һ���ֵĳ���
     * ��0��1M�������ַӳ�䵽ͬ���������ַ
     */
    virtuladdr = 0;
    physicaladdr = 0;
    mmu_tlb_base[virtuladdr >> 20] = (physicaladdr & MUM_SECTION_PADDR_BASE_MASK) | \
                                            MMU_SECDESC_WB;

    /*
     * 0x48000000������Ĵ�������ʼ�����ַ��
     * �������ַ0x48000000��0x5FFFFFFFӳ�䵽�����ַ0x48000000��0x5FFFFFFF�ϣ�
     */
    virtuladdr = VIRTUAL_IO_ADDR;
    physicaladdr = PHYSICAL_IO_ADDR;
	while (virtuladdr < VIRTUAL_IO_ADDR+IO_MAP_SIZE)
    {
        mmu_tlb_base[virtuladdr >> 20] = (physicaladdr & MUM_SECTION_PADDR_BASE_MASK) | \
                                            MMU_SECDESC;
        virtuladdr += MMU_SECTION_SIZE;
        physicaladdr += MMU_SECTION_SIZE;
    }
	
    /*
     * SDRAM�������ַ��Χ��0x30000000��0x33FFFFFF��
     * �������ַ0x30000000��0x33FFFFFFӳ�䵽�����ַ0x30000000��0x33FFFFFF�ϣ�
     * �ܹ�64M���漰64����������
     */
    virtuladdr = VIRTUAL_MEM_ADDR;
    physicaladdr = PHYSICAL_MEM_ADDR;
    while (virtuladdr < VIRTUAL_MEM_ADDR+MEM_MAP_SIZE)
    {
        mmu_tlb_base[virtuladdr >> 20] = (physicaladdr & MUM_SECTION_PADDR_BASE_MASK) | \
                                                MMU_SECDESC_WB;
        virtuladdr += MMU_SECTION_SIZE;
        physicaladdr += MMU_SECTION_SIZE;
    }
}

/*
 * ����MMU
 */
void mmu_init(void)
{
    unsigned long ttb = 0x30000000;

	__asm__ (
		"mov    r0, #0\n"
		"mcr    p15, 0, r0, c7, c7, 0\n"    /* ʹ��ЧICaches��DCaches */
		
		"mcr    p15, 0, r0, c7, c10, 4\n"   /* drain write buffer on v4 */
		"mcr    p15, 0, r0, c8, c7, 0\n"    /* ʹ��Чָ�����TLB */
		
		"mov    r4, %0\n"                   /* r4 = ҳ���ַ */
		"mcr    p15, 0, r4, c2, c0, 0\n"    /* ����ҳ���ַ�Ĵ��� */
		
		"mvn    r0, #0\n"                   
		"mcr    p15, 0, r0, c3, c0, 0\n"    /* ����ʿ��ƼĴ�����Ϊ0xFFFFFFFF��
											 * ������Ȩ�޼�� 
											 */    
		/* 
		 * ���ڿ��ƼĴ������ȶ�����ֵ������������޸ĸ���Ȥ��λ��
		 * Ȼ����д��
		 */
		"mrc    p15, 0, r0, c1, c0, 0\n"    /* �������ƼĴ�����ֵ */
		
		/* ���ƼĴ����ĵ�16λ����Ϊ��.RVI ..RS B... .CAM
		 * R : ��ʾ����Cache�е���Ŀʱʹ�õ��㷨��
		 *     0 = Random replacement��1 = Round robin replacement
		 * V : ��ʾ�쳣���������ڵ�λ�ã�
		 *     0 = Low addresses = 0x00000000��1 = High addresses = 0xFFFF0000
		 * I : 0 = �ر�ICaches��1 = ����ICaches
		 * R��S : ������ҳ���е�������һ��ȷ���ڴ�ķ���Ȩ��
		 * B : 0 = CPUΪС�ֽ���1 = CPUΪ���ֽ���
		 * C : 0 = �ر�DCaches��1 = ����DCaches
		 * A : 0 = ���ݷ���ʱ�����е�ַ�����飻1 = ���ݷ���ʱ���е�ַ������
		 * M : 0 = �ر�MMU��1 = ����MMU
		 */
		
		/*  
		 * ���������Ҫ��λ����������Ҫ��������������    
		 */
											/* .RVI ..RS B... .CAM */ 
		"bic    r0, r0, #0x3000\n"          /* ..11 .... .... .... ���V��Iλ */
		"bic    r0, r0, #0x0300\n"          /* .... ..11 .... .... ���R��Sλ */
		"bic    r0, r0, #0x0087\n"          /* .... .... 1... .111 ���B/C/A/M */

		/*
		 * ������Ҫ��λ
		 */
		"orr    r0, r0, #0x0002\n"          /* .... .... .... ..1. ���������� */
		"orr    r0, r0, #0x0004\n"          /* .... .... .... .1.. ����DCaches */
		"orr    r0, r0, #0x1000\n"          /* ...1 .... .... .... ����ICaches */
		"orr    r0, r0, #0x0001\n"          /* .... .... .... ...1 ʹ��MMU */
		
		"mcr    p15, 0, r0, c1, c0, 0\n"    /* ���޸ĵ�ֵд����ƼĴ��� */
		: /* ����� */
		: "r" (ttb)
		: "r0","r4"
	);
}

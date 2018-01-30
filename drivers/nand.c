#include "nand.h"
/* NAND FLASH������ */
#define NFCONF (*((volatile unsigned long *)0x4E000000))
#define NFCONT (*((volatile unsigned long *)0x4E000004))
#define NFCMMD (*((volatile unsigned char *)0x4E000008))
#define NFADDR (*((volatile unsigned char *)0x4E00000C))
#define NFDATA (*((volatile unsigned char *)0x4E000010))
#define NFSTAT (*((volatile unsigned char *)0x4E000020))

#define CMD_READ1	0x00				//ҳ����������1
#define CMD_READ2	0x30				//ҳ����������2
#define CMD_READID	0x90				//��ID����
#define CMD_WRITE1	0x80				//ҳд��������1
#define CMD_WRITE2	0x10				//ҳд��������2
#define CMD_ERASE1	0x60				//�������������1
#define CMD_ERASE2	0xd0				//�������������2
#define CMD_STATUS	0x70				//��״̬����
#define CMD_RESET	0xff				//��λ
#define CMD_RANDOMREAD1	0x05			//�������������1
#define CMD_RANDOMREAD2	0xE0			//�������������2
#define CMD_RANDOMWRITE	0x85			//����д����

static void nand_reset(void);
static void delay_u(int time){
	for(volatile int i=0;i<36;i++)
		for(volatile int i=0;i<time;i++);
}
void nand_init(void) {
#define TACLS   0
#define TWRPH0  1
#define TWRPH1  0
	/* ����ʱ�� */
	NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
	/* ʹ��NAND Flash������, ��ʼ��ECC, ��ֹƬѡ */
	NFCONT = (1<<4)|(1<<1)|(1<<0);
	nand_reset();
}

static void nand_select(void) {
	NFCONT &= ~(1<<1);
}

static void nand_deselect(void) {
	NFCONT |= (1<<1);
}

static void nand_cmd(unsigned char cmd) {
	volatile int i;
	NFCMMD = cmd;
	for (i = 0; i < 10; i++);
}

static void nand_addr(unsigned int addr) {
	unsigned int col  = addr % 2048;
	unsigned int page = addr / 2048;
	volatile int i;

	NFADDR = col & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR = (col >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = page & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 16) & 0xff;
	for (i = 0; i < 10; i++);
}

static void nand_page(unsigned int page) {
	volatile int i;

	NFADDR  = page & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 16) & 0xff;
	for (i = 0; i < 10; i++);
}

static void nand_col(unsigned int col) {
	volatile int i;

	NFADDR = col & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR = (col >> 8) & 0xff;
	for (i = 0; i < 10; i++);
}

static void nand_wait_ready(void) {
	while (!(NFSTAT & 1));
}

static unsigned char nand_read_data(void) {
	return NFDATA;
}

static void nand_write_data(unsigned char data) {
	NFDATA = data;
}

/* ��λ */
static void nand_reset(void) {
	nand_select();
	nand_cmd(CMD_RESET);  // ��λ����
	nand_wait_ready();
	nand_deselect();
}

int nand_is_bad_block(unsigned int block_number) {
	unsigned int col  = 2048;
	unsigned int page = BLOCK_TO_PAGE(block_number);
	unsigned char val1,val2;

	
	nand_select();		/* 1. ѡ�� */
	nand_cmd(CMD_READ1);		/* 2. ����������00h */

	/* 3. ������ַ(��5������) */
	nand_col(col);
	nand_page(page);

	
	nand_cmd(CMD_READ2);		/* 4. ����������30h */
	nand_wait_ready();/* 5. �ж�״̬ */

	/* 6. ������ */
	val1 = nand_read_data();
	val2 = nand_read_data();
	/* 7. ȡ��ѡ�� */
	nand_deselect();

	if ((val1 == 0xff)&&(val2 == 0xff))
		return 0;  
	else
		return 1;/* bad blcok */
}


void nand_read(unsigned char *buf,unsigned int addr , unsigned int len) {
	int col ;
	int i = 0;

	while (i < len) {
		/* һ��blockֻ�ж�һ�� */
		if (!(addr & 0x1FFFF)) { //�����µ�block
			if(nand_is_bad_block(addr)){
				addr += (128*1024);  /* ������ǰblock */
				continue;
			}
		}

		nand_select();			/* 1. ѡ�� */
		nand_cmd(CMD_READ1);	/* 2. ����������00h */
		nand_addr(addr);		/* 3. ������ַ(��5������) */
		nand_cmd(CMD_READ2);	/* 4. ����������30h */
		nand_wait_ready();		/* 5. �ж�״̬ */

		/* 6. ������ */
		for (col=0 ; (col < 2048) && (i < len); col++) {
			buf[i] = nand_read_data();
			i++;
			addr++;
		}
		
		nand_deselect();/* 7. ȡ��ѡ�� */

	}
}

int nand_read_with_oob(unsigned int page,unsigned char *buf,unsigned int buf_len,unsigned char *oob,unsigned int oob_len) {
	int i;
	nand_select();
	nand_cmd(CMD_READ1);
	nand_addr(page<<PAGE_SHIFT);
	nand_cmd(CMD_READ2);
	nand_wait_ready();
	//TODO:�Ż�,�����
	if(buf){
		for (i=0 ; i < 2048; i++) {
			if(i<buf_len)
				buf[i] = nand_read_data();
			else
				nand_read_data();
		}
	}else{
		for (i=0 ; i < 2048; i++) {
			nand_read_data();
		}
	}
	
	nand_read_data();
	nand_read_data();
	
	for (i=0; i<16; i++) {
		nand_read_data();
	}
	
	nand_read_data();
	nand_read_data();
	
	if(oob){
		for (i=0 ; i < oob_len; i++) {
			oob[i] = nand_read_data();
		}
	}
	nand_deselect();
	return 1;
}

int nand_read_page(unsigned int page,unsigned char *buf,unsigned int buf_len,unsigned char *spare,unsigned int spare_len) {
	int i;
	nand_select();
	nand_cmd(CMD_READ1);
	nand_addr(page<<PAGE_SHIFT);
	nand_cmd(CMD_READ2);
	nand_wait_ready();
	//TODO:�Ż�,�����
	if(buf){
		for (i=0 ; i < 2048; i++) {
			if(i<buf_len)
				buf[i] = nand_read_data();
			else
				nand_read_data();
		}
	}else{
		for (i=0 ; i < 2048; i++) {
			nand_read_data();
		}
	}
	
	if(spare){
		for (i=0 ; i < spare_len; i++) {
			spare[i] = nand_read_data();
		}
	}
	nand_deselect();
	return 1;
}

int nand_write(unsigned char *buf,unsigned int addr , unsigned int len) {
	int i;
	unsigned char stat;

	int temp = nand_is_bad_block(addr);	//�жϸÿ��Ƿ�Ϊ����
	if(temp)
		return -1;				//�ǻ��飬����

	nand_select();				//��nandflashƬѡ
	nand_cmd(CMD_WRITE1);		//ҳд��������1
	nand_addr(addr);

	//д��һҳ����
	for (i = 0; (i < 2048) && (i < len); i++) {
		nand_write_data(buf[i]);
	}

	nand_cmd(CMD_WRITE2);		//ҳд��������2
	delay_u(1000);				//��ʱһ��ʱ�䣬�Եȴ�д�������
	nand_cmd(CMD_STATUS);		//��״̬����

	//�ж�״ֵ̬�ĵ�6λ�Ƿ�Ϊ1�����Ƿ���æ��������������NF_DETECT_RB();��ͬ
	do {
		stat = nand_read_data();
	} while(!(stat&0x40));

	nand_deselect();			//�ر�nandflashƬѡ

	if (stat & 0x1) {
		return -2;				//д����ʧ��
	} else
		return 1;				//д�����ɹ�
}

int nand_write_with_oob(unsigned int page,unsigned char *buf,unsigned int buf_len,unsigned char *oob,unsigned int oob_len) {
	int i;
	unsigned char stat;

	nand_select();				//��nandflashƬѡ
	nand_cmd(CMD_WRITE1);		//ҳд��������1
	nand_addr(page<<PAGE_SHIFT);

	//д��һҳ����
	if(buf){
		for (i = 0; i < buf_len; i++) {
			nand_write_data(buf[i]);
		}
	}
	if (buf_len < 2048) {
		//������oob��
		int col = 2048;
		nand_cmd(CMD_RANDOMWRITE); // ҳ�����д����
		nand_col(col);
	}
	nand_write_data(0xff);
	nand_write_data(0xff);
	
	for (i=0; i<16; i++) {
		nand_write_data(0xff);
	}
	nand_write_data(0xff);
	nand_write_data(0xff);
	
	if(oob){
		for (i = 0; i < oob_len; i++) {
			nand_write_data(oob[i]);
		}
	}
	
	nand_cmd(CMD_WRITE2);		//ҳд��������2
	delay_u(1000);				//��ʱһ��ʱ�䣬�Եȴ�д�������
	nand_cmd(CMD_STATUS);		//��״̬����

	//�ж�״ֵ̬�ĵ�6λ�Ƿ�Ϊ1�����Ƿ���æ��������������NF_DETECT_RB();��ͬ
	do {
		stat = nand_read_data();
	} while(!(stat&0x40));

	nand_deselect();			//�ر�nandflashƬѡ

	if (stat & 0x1) {
		return -2;				//д����ʧ��
	} else
		return 1;				//д�����ɹ�
}

int nand_write_page(unsigned int page,unsigned char *buf,unsigned int buf_len,unsigned char *spare,unsigned int spare_len) {
	int i;
	unsigned char stat;

	nand_select();				//��nandflashƬѡ
	nand_cmd(CMD_WRITE1);		//ҳд��������1
	nand_addr(page<<PAGE_SHIFT);

	//д��һҳ����
	if(buf){
		for (i = 0; i < buf_len; i++) {
			nand_write_data(buf[i]);
		}
	}
	if (buf_len < 2048) {
		//������oob��
		int col = 2048;
		nand_cmd(CMD_RANDOMWRITE); // ҳ�����д����
		nand_col(col);
	}

	if(spare){
		for (i = 0; i < spare_len; i++) {
			nand_write_data(spare[i]);
		}
	}
	
	nand_cmd(CMD_WRITE2);		//ҳд��������2
	delay_u(1000);				//��ʱһ��ʱ�䣬�Եȴ�д�������
	nand_cmd(CMD_STATUS);		//��״̬����

	//�ж�״ֵ̬�ĵ�6λ�Ƿ�Ϊ1�����Ƿ���æ��������������NF_DETECT_RB();��ͬ
	do {
		stat = nand_read_data();
	} while(!(stat&0x40));

	nand_deselect();			//�ر�nandflashƬѡ

	if (stat & 0x1) {
		return -2;				//д����ʧ��
	} else
		return 1;				//д�����ɹ�
}
int nand_mark_bad_block(unsigned int block_number){
	unsigned char oob[2]={0xff,0xff};
	// ÿ��block��һҳspare��0, 1�ֽڷ�0xff���Ϊ�û�
	return nand_write_page(BLOCK_TO_PAGE(block_number), 0, 2048, oob, 2);
}

int nand_erase_block(unsigned int block_number) {
	unsigned char stat;

	nand_select();					//��Ƭѡ
	nand_cmd(CMD_ERASE1);			//������������1
	//д��3����ַ���ڣ���A18��ʼд��
	nand_page(block_number<<6);
	nand_cmd(CMD_ERASE2);			//������������2
	delay_u(10000);					//��ʱһ��ʱ��
	nand_cmd(CMD_STATUS);			//��״̬����
	
	//�ж�״ֵ̬�ĵ�6λ�Ƿ�Ϊ1�����Ƿ���æ��������������NF_DETECT_RB();��ͬ
	do {
		stat = nand_read_data();
	} while(!(stat&0x40));

	nand_deselect();				//�ر�nandflashƬѡ

	//�ж�״ֵ̬�ĵ�0λ�Ƿ�Ϊ0��Ϊ0�����������ȷ��������� 
	if (stat & 0x1) {
		return -2;					//��������ʧ��
	} else
		return 1;					//���������ɹ�
}

void nand_print(unsigned int page) {
	char buf[2048]={0};
	char spare[64]={0};
	nand_read_page(page,buf,2048,spare,64);
	printf("NAND BLOCK:%d PAGE:%d :\n\r",PAGE_TO_BLOCK(page),(page)&0x3f);
	printf("MAIN DATA:\n\r");
	for(int i=0;i<2048;i++){
		printf("%02X ",buf[i]&0xff);
		if(i%16==15)
			printf("\n\r");
	}
	printf("SPARE DATA:\n\r");
	for(int i=0;i<64;i++){
		printf("%02X ",spare[i]&0xff);
		if(i%16==15)
			printf("\n\r");
	}
	
}


#include "s3c24xx.h"

//extern void I2CIntHandle(void);
void ClearPending(unsigned bit);
void (*isr_handle_array[50])(void);

void enable_irq(void) {
	__asm__ volatile (
	    "mrs r4,cpsr\n"
	    "bic r4,r4,#0x80\n"
	    "msr cpsr,r4\n"
	    :::"r4"
	);
}
void disable_irq(void) {
	__asm__ volatile (
	    "mrs r4,cpsr\n"
	    "orr r4,r4,#0x80\n"
	    "msr cpsr,r4\n"
	    :::"r4"
	);
}
//TODO:INTMS��ΪINTMSK
void INTMS_set(unsigned int offset) {
	INTMSK &= ~(1 << offset);
}
void INTMS_clr(unsigned int offset) {
	INTMSK |= (1 << offset);
}
void Dummy_isr(void) {
	printf("IRQ HANDLE!\n\r");
	while (1);
}

void init_irq(void) {
	int i = 0;
	for (i = 0; i < sizeof(isr_handle_array) / sizeof(isr_handle_array[0]); i++) {
		isr_handle_array[i] = Dummy_isr;
	}

	INTMOD = 0x0;	      // �����ж϶���ΪIRQģʽ
	INTMSK = BIT_ALLMSK;  // �����������ж�

//	isr_handle_array[ISR_IIC_OFT]  = I2CIntHandle;
}

void IRQ_Handle(void) {
	unsigned long oft = INTOFFSET;
	//printf("INTOFFSET:%d\n",oft);

	//TODO:�ж�Ƕ��ʱ���жϿ�������
	/* �����жϷ������ */
	isr_handle_array[oft]();
	//���ж�
	ClearPending(INTOFFSET);
}
void set_irq_handler(int offset, int (*hander)(void)) {
	//TODO:����
	if (hander != 0)
		isr_handle_array[offset] = hander;
	else
		isr_handle_array[offset] = Dummy_isr;
}
//���ж�
void ClearPending(unsigned oft) {
	if (oft == 4)
		EINTPEND = 1 << 7;  //EINT4-7����IRQ4��ע��EINTPEND[3:0]����δ�ã�����Щλд��1���ܵ���δ֪���
	if (oft == 5)
		EINTPEND = 1 << 11; // EINT8_23����IRQ5
	SRCPND = 1 << oft;
	INTPND = 1 << oft;
}

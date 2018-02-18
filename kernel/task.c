#include <sys/types.h>
#include <s3c24xx.h>
#include <stdio.h>
#include <syscall.h>
#include "interrupt.h"
#include <sched.h>
#include <fcntl.h>
#include <trace.h>


PCB task[NR_TASK];								// ���̶���(PCB����)�����֧��62������
PCB *__current_task = NULL;						// ������ǰִ�н���ָ��
long runningCount = 0;							// �������н��̸���
extern void __switch_to(struct cpu_context_save *prev, struct cpu_context_save *next);
// �����ⲿ�����������л�����
void switch_to(PCB *pcur, PCB *pnext) {
	__switch_to(&(pcur->context), &(pnext->context));
}
int cmd_loop();
extern void *kmalloc(unsigned int size);
extern void kfree(void *addr);
static void task0();
static void task1();
/*****************************************************************************
* ���̶��г�ʼ������
*****************************************************************************/
void sched_init(void) {
	PCB *p = &task[0];					// 0�Ž���Ϊ�ں˽���
	int i;
	/* ѭ��Ϊÿ������PCB��ʼ�� */
	for (i = 0; i < NR_TASK; i++, p++) {
		p->pid = -1;					// pid = -1����ʾδ����pid
		p->state = TASK_UNALLOCATE;		// ���ó�ʼ����״̬Ϊδ����״̬
		p->count = 0;					// ����ʱ��Ƭ����Ϊ0����ʾû��ʱ��Ƭ
		p->priority = 0;				// ��ʼ�������ȼ�Ϊ0
	}
	CURRENT_TASK() = NULL;					// ��ǰ���н���Ϊ0�Ž���
	printf("kernel:sched_init, all task init OK\n");
}
void schedule(void) {
	/*
	* max�������浱ǰ���̶�����������ȼ�����count
	* p_tsk���浱ǰ����PID����
	*/
	long max = -1;						// max��ʼֵΪ-1���������ж�
	long i = 0,  next = 0;				// next����������ȼ�PID
	PCB *p_tsk = NULL;					// ��ʱ���̽ṹ��ָ��

	// ���ֻ��һ��������Ϊ0�Ž���,������ʱ��Ƭ,����
	if (runningCount == 1 && CURRENT_TASK()->pid == 0) {
		CURRENT_TASK()->count = CURRENT_TASK()->priority;
		return ;
	}
	// ���̵���ѭ��

	trace(KERN_DEBUG, "kernel:schedule\n");
	while (1) {
		/*
		*  ѭ���ҳ����̶��������״̬������ȼ����̣�Ҳ����countֵ�����̣�
		*  countԽ��˵���䱻ִ��ʱ��Խ�̣�CPU����Խ�ߣ�
		*  ͬʱ������PID�����̶��������±꣩��next��
		*  0�Ž��̲�����Ƚ�
		*/
		for (i = 1; i < NR_TASK; i++) {
			if ( (task[i].state == TASK_RUNNING) && (max < (long)task[i].count) ) {
				max = (long)task[i].count;
				next = i;
			}
		}

		// ���maxΪ��0������ѭ����˵��ѡ���˵��Ƚ���
		// ���maxΪ0��˵��countֵ������countΪ0��˵��ȫ�����̷���ʱ��Ƭ��ִ���꣬
		// ��Ҫ���·��䣬ִ��break����for���
		// ���maxΪ-1˵��û�о���״̬���̿ɱ����ȣ��˳�ѭ��������ִ��0����

		if (max) break;		// max = 0ʱ��ѡ���½��̣�����ѭ��
		// max = 0�������̶�����countֵ���Ϊ0��ȫ������ʱ��Ƭ�þ�����Ҫ���·���
		for (i = 1; i < NR_TASK; i++) {
			if ( task[i].state == TASK_RUNNING ) {
				// ʱ��Ƭ��Ϊ��Ĭ�����ȼ�
				task[i].count = task[i].priority;
			}
		}
	}
	// ��ǰ����Ϊѡ�����̣�˵����ǰ�������ȼ�������ߣ����ؼ���ִ��
	if (CURRENT_TASK() == &task[next])
		return;
	// ��ЧPID
	if (task[next].pid < 0)
		return;
	// ���浱ǰ���̸�����p_tsk����ѡ����������Ϊ��ǰ���н���
	p_tsk = CURRENT_TASK();
	CURRENT_TASK() = &task[next];
	trace(KERN_DEBUG, "__switch_to\n");
	trace(KERN_DEBUG, "\nold task id =%d", p_tsk->pid);
	trace(KERN_DEBUG, "\nnew task id =%d", next);
	trace(KERN_DEBUG, "\n");
	// �����������л�����
	switch_to(p_tsk, &task[next]);
}
/*****************************************************************************
* ��ʱ��������
* ��Ҫ���ڽ���ʱ��Ƭ�����˯��ʱ�䴦��ÿ�ζ�ʱ���жϲ����󣬵��øú������Խ���ʱ��Ƭ����
* �ݼ����������ʱ��Ƭ���꣬����е��ȣ�����û�������������˯��״̬����
* �ú�����˯��ʱ�䵽��󣬽��份��
*****************************************************************************/
void do_timer(void) {
	int i = 0;
	// û�е�ǰ���̣�˵�����̻�δ����������
	if (!CURRENT_TASK()) {
		trace(KERN_WARNING, "kernel:leaving do_timer,hasn't init task\n");
		return;
	}
	// �ݼ�˯�߽��̣�˯��ʱ�䵽�ˣ�����״̬��Ϊ����̬
	for (i = 1; i < NR_TASK; i++) {
		if (task[i].state == TASK_SLEEPING) {			// �����˯��ʱ��
			if (!(--task[i].timer)) {
				task[i].state = TASK_RUNNING;		// ���˯��ʱ��Ϊ0��������
				runningCount++;
			}
		}
	}
	// �Ե�ǰִ�н���ʱ��Ƭ�ݼ���ÿ10ms�ݼ�һ��
	if (CURRENT_TASK()->count) {
		CURRENT_TASK()->count--;
	}
	// �����ǰ����ʱ��Ƭ�Ѿ����꣬��ǰ����״̬Ϊ�Ǿ���̬�����Ե����½���
	if ((CURRENT_TASK()->state != TASK_RUNNING) || CURRENT_TASK()->count <= 0 ) {
		// �����ں˿ռ�ִ�н��̲��ᱻ��ռ���
		//if(is_in_user_space())
		schedule();
	}
}
/*****************************************************************************
* ɱ�����̺���
* ������int pid��0�����ɱ��һ�����̣�pid��Ϊ0��ɱ��ָ������id��Ϊpid����
*****************************************************************************/
void kthread_kill(int pid) {
	int i;
	if (pid == 0)
		return;
	trace(KERN_DEBUG, "kernel:kill_task\r\n");
	for (i = 1; i < NR_TASK; i++) {
		if (task[i].state != TASK_UNALLOCATE) {
			if (pid == task[i].pid) {
				task[i].pid = -1;
				task[i].state = TASK_UNALLOCATE;
				task[i].count = 0;
				task[i].priority = 0;
				runningCount--;
				break;
			}
		}
	}
	// ɱ�����̺����µ���
	schedule();
}
void debug_task() {
	printf("debug\n");
	int *ad = 0x33d00000 - 14 * 4;
	for (int i = 0; i < 14; i++) {
		printf("%X\n", ad[i]);
	}
	printf("\n");
	struct cpu_context_save *context = &(CURRENT_TASK()->context);
	trace(KERN_DEBUG, "cpsr:%X,sp:%X,pc:%X\n", context->cpsr, context->sp, context->pc);
}
void task_start(__u32 sp, __u32 pc) {
	trace(KERN_DEBUG, "sp:%X,pc:%X\n", sp, pc);
	trace(KERN_DEBUG, "task_start\n");
	asm(
	    "mov sp,%0\n"
	    "mov pc,%1\n"
	    :
	    :"r"(sp), "r"(pc)
	);
}
void task_timer_init() {
	int delay_time = 10000;
	//��ʱ�����ƼĴ��� 0 ��TCFG0��
	TCFG0 |= (24); //��ʱ�� 0��1 ��Ԥ��Ƶֵ
	//��ʱ�����ƼĴ��� 1 ��TCON��
	TCON &= (~(15 << 8)); //���8~11λ
	TCON |= (1 << 11); //��ʱ�� 1��϶ģʽ���Զ����أ�
	TCON |= (1 << 9); //��ʱ�� 1�ֶ����� TCNTB1
	//TCONB1:��ʱ�� 1  ��������Ĵ���
	TCNTB1 = delay_time;

	TCON |= (1 << 8); //����
	TCON &= ~(1 << 9); //��ʱ�� 1 ȡ���ֶ�����
}
void task_init() {
	sched_init();
	void *sp = kmalloc(512);
	if (sp) {
		kthread_create(task0, sp, 5);
		if (task[0].state != TASK_RUNNING) {
			trace(KERN_CRIT, "task init failed!");
			while (1);
		} else {
			CURRENT_TASK() = &task[0];
		}
	}
	sp = kmalloc(2048);
	if (sp)
		kthread_create(cmd_loop, sp, 5);
	task_timer_init();
	set_irq_handler(INT_TIMER1, do_timer);
	INTMS_set(INT_TIMER1);
	task_start(CURRENT_TASK()->context.sp, CURRENT_TASK()->context.pc);
}
static void task0() {
	trace(KERN_DEBUG, "task0:init\n");
	while (1);
}

__u32 get_cpsr() {
	int ret;
	asm (
	    "mrs %0,cpsr"
	    :"=r"(ret)
	);
	return ret;
}
void kthread_exit() {
	if (CURRENT_TASK()) {
		CURRENT_TASK()->state = TASK_UNALLOCATE;
		kfree(CURRENT_TASK()->stack);
	}
}
/*******************************************************************************
* �����½��̺���
**********************************************************************************/
int kthread_create(unsigned long pc, void *data, long priority) {
	unsigned long i, pid = -1;

	printf("kernel:kthread_create\n");
	/*
	* ����û������Ƿ���ϳ�����ù�������û������һ��ָ��Ϊ��ldr	r0, [sp]
	* ���Ӧ������Ϊ:0xe59d0000��������ص��ǷǷ�����������˳�
	*/
	// Ϊ�½�����ѡ����pid
	for (i = 0; i < NR_TASK; i++) {
		if ((task[i].state == TASK_UNALLOCATE) ) {
			pid = i;
			break;
		}
	}
	// ���û�п���Pid�������˳�
	if (pid == -1) {
		trace(KERN_CRIT, "task has to max number!\r\n");
		return -1;
	}
	void *sp = kmalloc(512);
	if (!sp) {
		trace(KERN_CRIT, "memory allocation failed for the new task!");
		return -1;
	}
	// ������̴������˹����в��ܱ��жϴ��
	//OS_ENTER_CRITICAL();
	// -----------------����Ϊ����ִ��ʱ������������-----------------------------------
	runningCount++;
	task[pid].pid = pid;					// �½���PID
	task[pid].state = TASK_RUNNING;			// �½���ִ��״̬
	task[pid].count = 5;					// �½���ʱ��Ƭ
	task[pid].priority = priority;			// �½������ȼ�
	task[pid].context.cpsr = get_cpsr();
	task[pid].context.sp = sp; // SPջָ��
	task[pid].stack = sp;
	task[pid].context.lr = kthread_exit;	// LR���ص�ַ
	task[pid].context.pc = pc;				// PC
	task[pid].pwd = get_root_inode();				// PWD
	// ���ж�
	//OS_EXIT_CRITICAL();
	return pid;
}

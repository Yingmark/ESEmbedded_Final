#include <stdint.h>
#include <stdio.h>
#include "reg.h"
#include "blink.h"
#include "usart.h"
#include "asm_func.h"

#define TASK_NUM 4
#define PSTACK_SIZE_WORDS 256 //user stack size = 4 kB

static uint32_t *psp_array[TASK_NUM];

void setup_systick(uint32_t ticks);

void init_task(unsigned int task_id, uint32_t *task_addr, uint32_t *psp_init)
{
	*(psp_init-1) = (uint32_t)0x1000000;	  //xPSR (bit 24, T bit, has to be 1 in Thumb state)
	*(psp_init-2) = (uint32_t)task_addr; 	  //Return Address is being initialized to the task entry
	psp_array[task_id] = psp_init-16;	  //initialize psp_array (stack frame: 8 + r4 ~ r11: 8)
}

void task0(void)
{
	printf("[Task0] Start in unprivileged thread mode.\r\n");
	printf("[Task0] Control: 0x%x \r\n\n", (unsigned int)read_ctrl());

	blink(LED_BLUE); //should not return
}

void task1(void)
{
	printf("[Task1] Start in unprivileged thread mode.\r\n");
	printf("[Task1] Control: 0x%x \r\n\n", (unsigned int)read_ctrl());

	blink(LED_GREEN); //should not return
}

void task2(void)
{
	printf("[Task2] Start in unprivileged thread mode.\r\n");
	printf("[Task2] Control: 0x%x \r\n\n", (unsigned int)read_ctrl());

	blink(LED_ORANGE); //should not return
}

void task3(void)
{
	printf("[Task3] Start in unprivileged thread mode.\r\n");

	int f1=0,f2=1,f3,i=3;
    //printf("%d\r%d",f1,f2);
	printf("%d \r\n\n", f1);
	printf("%d \r\n\n", f2);
    while(i<=30)
    {
        f3=f1+f2;
        printf("%d \r\n\n",f3);

		for (int t = 0; t < 500000; t++)
			;

		//printf("[Task3] f3: 0x%x \r\n\n", (unsigned int)f3);
        f1=f2;
        f2=f3;
        i=i+1;
		//printf("[Task3] i: 0x%x \r\n\n", (unsigned int)i);
	}
}

int main(void)
{
	init_usart1();

	uint32_t user_stacks[TASK_NUM][PSTACK_SIZE_WORDS];
	while(1)
	{
		init_task(0, (uint32_t *)task0, user_stacks[0] + PSTACK_SIZE_WORDS);
		init_task(1, (uint32_t *)task1, user_stacks[1] + PSTACK_SIZE_WORDS);
		init_task(2, (uint32_t *)task2, user_stacks[2] + PSTACK_SIZE_WORDS);
		init_task(3, (uint32_t *)task3, user_stacks[3] + PSTACK_SIZE_WORDS);
	
		setup_systick(168e6 / 8 / 100); //10 ms
		start_user((uint32_t *)task0, user_stacks[0]);
	}

}

void setup_systick(uint32_t ticks)
{
	// set reload value
	WRITE_BITS(SYST_BASE + SYST_RVR_OFFSET, SYST_RELOAD_23_BIT, SYST_RELOAD_0_BIT, ticks - 1);

	// uses external reference clock
	CLEAR_BIT(SYST_BASE + SYST_CSR_OFFSET, SYST_CLKSOURCE_BIT);

	// enable systick exception
	SET_BIT(SYST_BASE + SYST_CSR_OFFSET, SYST_TICKINT_BIT);

	// enable systick
	SET_BIT(SYST_BASE + SYST_CSR_OFFSET, SYST_ENABLE_BIT);
}

uint32_t *sw_task(uint32_t *psp)
{
	static unsigned int curr_task_id = 0;

	psp_array[curr_task_id] = psp; //save current psp

	if (++curr_task_id > TASK_NUM - 1) //get next task id
		curr_task_id = 0;

	return psp_array[curr_task_id];//return next psp
}


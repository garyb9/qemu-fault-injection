#define USE_STDPERIPH_DRIVER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//#include <memory.h>
#include <stddef.h>
#include "stm32f10x.h"

#define MAX_SIZE 50

uint16_t period_value = (uint16_t) 2 * (36000000 / 65535);


void busyLoop(uint32_t delay)
{
  while(delay) delay--;
}



unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

unsigned long hash2(unsigned char *str, int size)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++) && size>0){
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        size--;
    }
    return hash;
}


void TIM2_IRQHandler(void)
{
    /* Note that I think we could have used TIM_GetFlagStatus
       and TIM_ClearFlag instead of TIM_GetITStatus and TIM_ClearITPendingBit.
       These routines seem to overlap quite a bit in functionality. */

    /* Make sure the line has a pending interrupt
     * which should always be true.
     *  */
    if(TIM_GetITStatus(TIM2, USART_IT_TXE) != RESET) {
        /* Toggle the LED */
        GPIOC->ODR ^= 0x00001000;
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

void init_timer(void) {
    /* Configure peripheral clock. */
    /* Let's leave PCLK1 at it's default setting of 36 MHz. */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* Time base configuration */
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 5 * period_value - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = 65535;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // Enable the update interrupt
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    /* Enable the timer IRQ in the NVIC module (so that the TIM2 interrupt
     * handler is enabled). */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable the timer
    TIM_Cmd(TIM2, ENABLE);
}

void send_string(char str[], int size){
    int i=0;
    while(i<size){
        send_byte(str[i]);
        i++;
    }

    send_byte('\n');
}



void *ft_memcpy (void *dest, const void *src, size_t len)
{
  char *d = dest;
  const char *s = src;
  while (len--)
    *d++ = *s++;
  return dest;
}

void *ft_memset (void *dest, int val, size_t len)
{
  unsigned char *ptr = dest;
  while (len-- > 0)
    *ptr++ = val;
  return dest;
}


int main(void)
{
    int i=0, timeout_flag = 0, is_return;
    uint8_t b;
    uint16_t curr_raw_count;

    char user_pass[MAX_SIZE];

    const char *ret = "return";

    unsigned long user_hash,
                  GOD_HASH = hash2("123456789",50),
                  WHO_HASH = hash2("whoami", 50);

    init_led();

    init_timer();

    init_rs232();
    //enable_rs232();
    USART_Cmd(USART2, ENABLE);

    i=0;
    is_return = 0;

    while(1) {
            //printf("instruction count: %d", instruction_count);
        /* Loop until the USART2 has received a byte. */
        while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET){
            // FlagStatus will be not be equal to RESET when IRQ is on

            if(is_return) break;

            curr_raw_count = TIM_GetCounter(TIM2);

            if (timeout_flag==1 && curr_raw_count>500){
                user_pass[i]='\0';
                user_hash = hash2(user_pass,i-1);
		asm("prog_begin:");
                for(int j = 0, k = 0; j<MAX_SIZE;j++){
                    if(user_pass[j] == '\0') break;
                    else{
                        if(user_pass[j] == ret[k]) k++;
                        else k = 0;

                        if(k == 6) is_return = 1;

                    }
                }

                asm("before_if:");
                if((user_hash==GOD_HASH)){
                    //hashes are equal
                    send_string("SUCCESS",strlen("SUCCESS"));
                }
                else{
                        asm("inside_else:");
                    //hashes are not equal
                    send_string("FAIL",strlen("FAIL"));
                }

                strcpy(user_pass,""); // clean buffer
                i=0; timeout_flag=0; // initialize counters
            }

        }

        if(is_return) break;

        /* Capture the received byte and print it out. */
        b = (USART_ReceiveData(USART2) & 0x7F); // This function set the USART Flag to RESET after receiving data
        send_byte(b);

        user_pass[i]=b;
        i++;

        TIM_Cmd(TIM2, ENABLE); // resets timer
        timeout_flag=1;


    }
	asm("prog_end:");
    return(0);
}

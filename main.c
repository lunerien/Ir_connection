#include <avr/io.h>
#include "HD44780.h"
#include <stdlib.h>
#include <avr/interrupt.h>

void clear_tab(uint16_t* tab){
	uint8_t i = 0;
	for(i = 0;i<64;i++)
		tab[i] = 0;
}

ISR(TIMER1_COMPA_vect)
{
	static uint8_t transmit_flag = 1;
	static uint8_t flag = 0;
	static uint16_t counter = 0;
	static uint8_t state = 1;
	static uint8_t index = 0;
	static uint16_t memory[64];
	static char bufor[64];
	static uint16_t transmit_time = 0;
	static uint16_t transmit_index = 0;
	static uint8_t start_flag = 0;
	LCD_Clear();
//	LCD_WriteText("KOT"); // wypisz "KOT"

	if(!(PIND & (1<<PD0)))
		flag = !flag;

	if(!(PIND & 1<<PD7) && start_flag == 0)
		counter = 0;
		start_flag = 1;

//	if(start_flag == 0 && !(PIND & 1<<PD7))
//		clear_tab(memory);

	if(state == 0 && counter == 70)
		start_flag = 0;

	if(!flag){
		LCD_WriteText("IR Recieve Mode");

		if(start_flag){
			if(state == 1){
				if(!(PIND & 1<<PD7) && counter < 300)//gdy odebrano cos na porcie (stan 1)
				{
					counter++;
					//state = 1;
				}
				else
				{
					memory[index] = counter;
					if(index > 64){
						index = 0;
					}
					index++;
					counter = 1;
					state = 0;
				}
			}
			else{
				if((PIND & 1<<PD7) && counter < 300)//gdy odebrano cos na porcie (stan 0)
				{
					counter++;
					//state = 0;
				}
				else
				{
					memory[index] = counter;
					if(index > 64){
						index = 0;
					}
					index++;
					counter = 1;
					state = 1;
				}
			}
			LCD_GoTo('8','0');
			uint8_t i;
			for(i = 0;i<16;i++){
				itoa(memory[i],bufor,10);
				LCD_WriteText(bufor);
			}

		}else{
			LCD_GoTo('8','0');
			LCD_WriteText("Waiting");
		}

//
//
	}
	else{

		LCD_WriteText("IR Transmit");
		while(transmit_index < 64){
			LCD_GoTo('8','0');
			itoa(transmit_index,bufor,10);
			LCD_WriteText(bufor);
			itoa(transmit_time,bufor,10);
			LCD_WriteText(bufor);
			if(transmit_time<memory[transmit_index]){
				if(transmit_flag == 1)
					PORTC = 0xFF;
				else
					PORTC = 0x00;
				transmit_time++;
			}
			else{
				transmit_flag = ! transmit_flag;
				transmit_index++;
				transmit_time = 0;
				if(transmit_index >= 63){
					transmit_index = 0;
				}
			}
		}
	}









}


int main(){
	DDRC = 0xFF;
	DDRD &= ~(1<<PD7);//port PD7 do odczytu
	DDRD |= 1<<PD5;
	TCCR1B |= 1<<WGM12;
	OCR1A = 781;//przerwanie co ~0.005s

	TIMSK |= 1<<OCIE1A;
	TCCR1B |= 1<<CS10 | 1<<CS12;

	LCD_Initalize();

	sei();

	while(1);

	return 0;
}





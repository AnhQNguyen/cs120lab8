/*	Author: anguy589
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #8  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1; //start count, down to 0. default 1 ms
unsigned long _avr_timer_cntcurr = 0;// current interal count of 1 ms ticks


void TimerOn() {

	TCCR1B = 0x0B; //AVR timer/counter controller register
	OCR1A = 125; // AVR output compare register
	TIMSK1 = 0x02; // AVR timer interrupt mask register

	TCNT1 = 0; //initialize AVR counter

         //TimerISR will be called every _avr_timer_cntcurr milliseconds
	_avr_timer_cntcurr = _avr_timer_M;

	SREG |= 0x80;//enable global interrupts
}

void TimerOff() {
	TCCR1B = 0x00; //timer off
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;

	}
}

//set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void set_PWM(double frequency) {
	static double current_frequency;

	if(frequency != current_frequency) {
		if(!frequency) { TCCR3B &= 0x08;}
		else { TCCR3B |= 0x03;}

		if(frequency < 0.954) { OCR3A = 0xFFFF;}
		else if(frequency > 31250) {OCR3A = 0x0000;}
		else {OCR3A = (short) (8000000 / (128 * frequency)) - 1;}
	
		TCNT3 = 0;
		current_frequency = frequency;
	}

}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;

}

enum sstates {smstart, wait,c4, d4, e4, stop } state;
unsigned char b1 = 0x00;
unsigned char b2 = 0x00;
unsigned char b3 = 0x00;


void soundTick() {
	b1 = ~PINA & 0x01;
	b2 = ~PINA & 0x02;
	b3 = ~PINA & 0x04;

	switch(state) {
		case smstart:
			state = wait;
			break;
		case wait:
			if(b1 && !b2 && !b3) {
				state = c4;
			}
			else if(!b1 && b2 && !b3) {
				state = d4;
			}
			else if(!b1 && !b2 && b3) {
				state = e4;
			}
			break;
		case c4:
			if(b1) {
				state = c4;
			}
			else {
				state = wait;       
			}
			break;
		case d4:
			if(b2) {
				state = d4;
			
			}
			else {
				state = wait;
			}
			break;
		case e4:
			if(b3) {
				state = e4;
				
			}
			else {
				state = wait;
			}
			break;
		
		default:
			break;

	
	}

	switch(state) {
		case smstart:
			break;
		case wait:
			set_PWM(0);
			break;
		case c4:
			set_PWM(261.63);
			break;
		case d4:
			set_PWM(293.66);
			break;
		case e4:
			set_PWM(329.63);
			break;
		default:
			break;
			
	}




}
//C4 = 261.63
//D4 = 293.66
//E4 = 329.63
int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;

	TimerSet(200);
	TimerOn();
	PWM_on();
	set_PWM(0);
	state = smstart;

    /* Insert your solution below */
    while (1) {
	soundTick();
    
		while(!TimerFlag);
		TimerFlag = 0;
    }

    return 1;
}

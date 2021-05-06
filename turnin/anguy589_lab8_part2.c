/*	Author: anguy589
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #8  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *      Demo Link: https://drive.google.com/file/d/1NCtcOiXPD8vqUcDcBv4lB2pgA5lOEh0U/view?usp=sharing
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

enum sstates {smstart, wait, onoff, inc, dec } state;
unsigned char b1 = 0x00; //on and off
unsigned char b2 = 0x00; //up
unsigned char b3 = 0x00; //down
unsigned char flag = 0; //helper onoff
unsigned char i = 0; //arr accessor

// C4, D, E, F, G, A, B, C5
double arrNotes[8] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};
void soundTick() {
	b1 = ~PINA & 0x01;
	b2 = ~PINA & 0x02;
	b3 = ~PINA & 0x04;



	switch(state) {
		case smstart:
			state = wait;
		case wait:
			if(b1) {
				state = onoff;
			}
			else if(b2) {
				state = inc;
			}
			else if(b3) {
				state = dec;
			}
			break;
		case onoff:
			if(!b1) {
				state = wait;
			}
			break;
	
		case inc:
			if(!b2) {
				state = wait;
			}
			break;
		case dec:
			if(!b3) {
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
			break;
		case onoff:
			if(!flag) {
				set_PWM(0);
				flag = 1;
				
			}
			else if(flag) {
				set_PWM(arrNotes[i]);
				flag = 0;
				
			}
			break;
		case inc:
			if(i < 8) {
				if(!flag) {set_PWM(arrNotes[i]);}
				i++;
			}

			
			break;
		case dec:
			if(i > 0){
				if(!flag) {set_PWM(arrNotes[i]);}
				i--;
			}
			else if(i == 0) {
				if(!flag) {set_PWM(arrNotes[i]);}
			}
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

	TimerSet(500);
	TimerOn();
	
	PWM_on();
	set_PWM(0);
	
	

    /* Insert your solution below */
    while (1) {
	soundTick();
    
		while(!TimerFlag);
		TimerFlag = 0;
    }

    return 1;
}

// UART1.c
// Runs on TM4C123
// Use UART1 to implement bidirectional data transfer to and from 
// another microcontroller in Lab 9.  This time, interrupts and FIFOs
// are used.
// Student names: change this or look very silly
// Last modification date: change this to the last modification date or look very silly
// Last Modified: 1/17/2021 

/* Lab 9 code 
 http://users.ece.utexas.edu/~valvano/
*/

// U1Rx (VCP receive) connected to PC4
// U1Tx (VCP transmit) connected to PC5
// When running on one board connect PC4 to PC5

#include <stdint.h>
#include "Fifo.h"
#include "UART1.h"
#include "../inc/tm4c123gh6pm.h"


// Initialize UART1
// Baud rate is 1000 bits/sec
// Interrupt on Rx hardware FIFO 1/2 full
// Busy-wait for Tx
void UART1_Init(void){
 // write this
	SYSCTL_RCGCUART_R |= 0x0002; 
	SYSCTL_RCGCGPIO_R |= 0x0003; 
//	while((SYSCYL_PRGPIO_R&0X01) == 0){};
	UART1_CTL_R &= ~0x0002; 
	UART1_IBRD_R = 5000;
	UART1_FBRD_R = 0;
	UART1_LCRH_R = 0x0070;
	UART1_CTL_R = 0x0301; 
	GPIO_PORTC_AFSEL_R |= 0x30; 
	GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R & 0xFFFFFF00) + 0x00000022;
	GPIO_PORTC_DEN_R |= 0x30; 	
	GPIO_PORTC_AMSEL_R &= ~0x30; 
}

//------------UART1_InChar------------
// Wait for new input, interrupt driven
// then return ASCII code
// Checks the software receive FIFO, not the hardware
// Input: none
// Output: char read from UART
char UART1_InChar(void){
	while((UART1_FR_R&0x0010) != 0);
	return ((char)(UART1_DR_R&0xFF));
}

// Lab 9
// check software RxFifo
uint32_t UART1_InStatus(void){
   // write this
  return 0; // replace this line
}

//------------UART1_InMessage------------
// Accepts ASCII characters from the serial port
//    and adds them to a string until ETX is received
//    or until max length of the string is reached.
// Input: pointer to empty buffer of 8 characters
// Output: Null terminated string
// Note: strips off STX CR and ETX
void UART1_InMessage(char *bufPt){
  // write this
}
//------------UART1_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
// Busy-wait synchronization
// Interesting note for Lab9: it will never wait
void UART1_OutChar(char data){
	while((UART1_FR_R&0x0020) != 0);
	UART1_DR_R = data;
}

// hardware RX FIFO goes from 7 to 8 or more items
// Interrupts after receiving entire message

#define PF1       (*((volatile uint32_t *)0x40025008))
void UART1_Handler(void){
  PF1 ^= 0x02;
  // write this
}





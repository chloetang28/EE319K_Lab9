// Lab9.c
// Runs on TM4C123
// Student names: change this or look very silly
// Last modification date: change this to the last modification date or look very silly
// Last Modified: 1/11/2021 

// Analog Input connected to PD2, analog channel 5
// displays on SSD1306
// PF3, PF2, PF1 are heartbeats
// UART1 on PC4-5
// * Start with where you left off in Lab8. 
// * Get Lab8 code working in this project.
// * Understand what parts of your main have to move into the UART1_Handler ISR
// * Rewrite the SysTickHandler to sample at 1 Hz
// * Implement the s/w Fifo on the receiver end 
//    (we suggest implementing and testing this first)
// When running on one board connect PC4 to PC5

#include <stdint.h>
#include "SSD1306.h"
#include "TExaS.h"
#include "ADC.h"
#include "print.h"
#include "../inc/tm4c123gh6pm.h"
#include "UART1.h"
#include "Fifo.h"
#define PC54                  (*((volatile uint32_t *)0x400060C0)) // bits 5-4
#define PF321                 (*((volatile uint32_t *)0x40025038)) // bits 3-1
// TExaSdisplay logic analyzer shows 7 bits 0,PC5,PC4,PF3,PF2,PF1,0 
void LogicAnalyzerTask(void){
  UART0_DR_R = 0x80|PF321|PC54; // sends at 10kHz
}


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

// use these three for debugging profile
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
// PF1 toggled in UART ISR (receive data)
// PF2 toggled in SysTick ISR (1 Hz sampling)
// PF3 toggled in main program when outputing to OLED


uint32_t Data;      // 12-bit ADC
uint32_t Position;  // 32-bit fixed-point 0.001 cm
int32_t TxCounter = 0;

// Initialize Port F so PF1, PF2 and PF3 are heartbeats
void PortF_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x20;      // activate port F
  while((SYSCTL_PRGPIO_R&0x20) != 0x20){};
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 (built-in LED)
  GPIO_PORTF_PUR_R |= 0x10;
  GPIO_PORTF_DEN_R |=  0x1E;   // enable digital I/O on PF
}

// main1 used to test FIFO
uint32_t Status[20];             
const uint32_t CorrectStatus[20]={
  0,1,1,1,1,1,1,0,1,2,1,1,0,3,4,5,6,7,8,0};
uint32_t ErrorCount;   // entries 1 2 3 4 5 6 7 8 should be 1 2 3 4 5 6 7 8
int main1(void){ // Make this main to test Fifo
  Fifo_Init();      // Assuming a Fifo of size 7, that can hold 6 elements
  ErrorCount = 0;
  for(;;){
    Status[0]  = Fifo_Get();  // should fail,    empty
    Status[1]  = Fifo_Put(1); // should succeed, 1 
    Status[2]  = Fifo_Put(2); // should succeed, 1 2
    Status[3]  = Fifo_Put(3); // should succeed, 1 2 3
    Status[4]  = Fifo_Put(4); // should succeed, 1 2 3 4
    Status[5]  = Fifo_Put(5); // should succeed, 1 2 3 4 5
    Status[6]  = Fifo_Put(6); // should succeed, 1 2 3 4 5 6
    Status[7]  = Fifo_Put(7); // should fail,    1 2 3 4 5 6 
    Status[8]  = Fifo_Get();  // should succeed, 2 3 4 5 6
    Status[9]  = Fifo_Get();  // should succeed, 3 4 5 6
    Status[10] = Fifo_Put(7); // should succeed, 3 4 5 6 7
    Status[11] = Fifo_Put(8); // should succeed, 3 4 5 6 7 8
    Status[12] = Fifo_Put(9); // should fail,    3 4 5 6 7 8 
    Status[13] = Fifo_Get();  // should succeed, 4 5 6 7 8
    Status[14] = Fifo_Get();  // should succeed, 5 6 7 8
    Status[15] = Fifo_Get();  // should succeed, 6 7 8
    Status[16] = Fifo_Get();  // should succeed, 7 8
    Status[17] = Fifo_Get();  // should succeed, 8
    Status[18] = Fifo_Get();  // should succeed, empty
    Status[19] = Fifo_Get();  // should fail,    empty
    for(int i=0; i<20; i++){
      if(Status[i] != CorrectStatus[i]){
        ErrorCount++;
      }
    }
  }
}


// main2 used to test UART1
// Connect PC5=PC4
// Use queue class in receiver interrupt
// UART1 receiver interrupt to 1/2 full 
// UART1 transmitter is busy wait
// PF1 toggles in UART ISR
// PF3 toggles in main
int main2(void){
  char OutData = '0'; 
  char InData;
  uint32_t time=0;
  DisableInterrupts();
  TExaS_Init(&LogicAnalyzerTask);
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear(); 
  PortF_Init();
  UART1_Init();       // enable UART
  EnableInterrupts();
  while(1){           
    time++;
    if((time%100000)==0){
      UART1_OutChar(STX);
      UART1_OutChar('1');
      UART1_OutChar('.');
      UART1_OutChar('2');
      UART1_OutChar(OutData);
      if(OutData == '9'){
        OutData = '0';
      }else{
        OutData++;
      }
      UART1_OutChar(' ');
      UART1_OutChar(CR);
      UART1_OutChar(ETX);
    }
    if(UART1_InStatus()){
      InData = UART1_InChar();
      SSD1306_OutChar(InData);
      PF3 ^= 0x08;
    }
  }
}

// **************SysTick_Init*********************
// Initialize Systick periodic interrupts
// Input: interrupt period
//        Units of period are 12.5ns
//        Maximum is 2^24-1
//        Minimum is determined by length of ISR
// Output: none
void SysTick_Init(uint32_t period){
    // write this
	NVIC_ST_CTRL_R = 0;    // disable SysTick
	NVIC_ST_RELOAD_R = period - 1;   // max reload value
	NVIC_ST_CURRENT_R = 0; // write to CURRENT
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; // priority
	NVIC_ST_CTRL_R = 7; // ENABLE, INTEN, CLK SRC	
}

// Get fit from excel and code the convert routine with the constants
// from the curve-fit
uint32_t Convert(uint32_t input){
// from lab 8
   return (156*input)/4096+27;
}


// final main program for bidirectional communication
// Sender sends using SysTick Interrupt, 5Hz sampling
// Receiver receives using RX
int main(void){  // valvano version
  DisableInterrupts();  
  TExaS_Init(&LogicAnalyzerTask);
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  ADC_Init(SAC_32);  // turn on ADC, set channel to 5
  PortF_Init();
  UART1_Init();       // initialize UART, 1000 bits/sec
	LCD_OutFix(0);
	SSD1306_OutString(" cm");
	SysTick_Init(8000000);
	Fifo_Init();
  // other initialization
//Enable SysTick Interrupt by calling SysTick_Init()
  EnableInterrupts();


  while(1){ 
		while(Fifo_Get() != STX){};			// wait for message
    PF3 ^= 0x08;       // Heartbeat
      // write this
	  SSD1306_OutClear();
	  SSD1306_SetCursor(10,2);
		Position = Convert(Data);
		LCD_OutFix(Position);
		
  }
}


void SysTick_Handler(void){ 
  // write this
	int32_t digit; 
	int32_t ascii; 
	
  PF1 ^= 0x02;     // Heartbeat
	Data = ADC_In();
	Data= Convert(Data); 
	UART1_OutChar(STX); 
	
	digit = Data/1000; 
	ascii = digit + 0x30; 
	UART1_OutChar(ascii); 
	
	UART1_OutChar(0x2E);
	
	Data = Data-(digit*1000); 
	digit = Data/100; 
	ascii = digit + 0x30; 
	UART1_OutChar(ascii);
	
	Data = Data-(digit*100);
	digit = Data/10;
	ascii = digit + 0x30; 
	UART1_OutChar(ascii);
	
	Data = Data-(digit*10);
	ascii = digit + 0x30; 
	UART1_OutChar(ascii);
	
	UART1_OutChar(0x0D);
	UART1_OutChar(ETX);
	TxCounter++;
}


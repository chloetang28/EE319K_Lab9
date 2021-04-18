// FiFo.c
// Runs on TM4C123
// Provide functions that implement the Software FiFo Buffer
// Last Modified: 1/17/2021 
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly
#include <stdint.h>

// Declare state variables for FiFo
//        size, buffer, put and get indexes

#define SIZE 16
uint32_t static PutI;
uint32_t static GetI;
int32_t static FIFO[SIZE];

// *********** FiFo_Init**********
// Initializes a software FIFO of a
// fixed size and sets up indexes for
// put and get operations
void Fifo_Init() {
	PutI = GetI = 0;
}

// *********** FiFo_Put**********
// Adds an element to the FIFO
// Input: Character to be inserted
// Output: 1 for success and 0 for failure
//         failure is when the buffer is full
uint32_t Fifo_Put(char data) {
  //Complete this routine
	if(((PutI+1)%SIZE) == GetI){
		return 0;
	}
	FIFO[PutI] = data;
	PutI = (PutI+1)%SIZE;
  return(1);
}

// *********** Fifo_Get**********
// Gets an element from the FIFO
// Input: none
// Output: removed character from FIFO
//         0 failure is when the buffer is empty
char Fifo_Get(void){char data;
  //Complete this routine
  if(PutI == GetI){
		return 0; 
	}
	data = FIFO[GetI];
	GetI = (GetI+1)%SIZE;
	return data;
}

// *********** Fifo_Status**********
// returns number of elements in the FIFO
// Input: none
// Output: number of entries in FIFO
//         0 failure is when the FIFO is empty
uint32_t Fifo_Status(void){
  //Complete this routine
	uint8_t count = 0;
	uint32_t PutIndex; 
	if(PutI == GetI){
		return count;
	}
	while(PutI != GetI){
		count++;
		PutIndex--;
	}
	return count;
}




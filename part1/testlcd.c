#include <8051.h>
#include "lcdlib.h"
#include "keylib.h"
#include "buttonlib.h"
#include "preemptive.h"
__idata __at (0x30) char empty;
__idata __at (0x31) char full;
__idata __at (0x32) char mutex;
__idata __at (0x33) char turn1;
__idata __at (0x34) char turn2;
//__idata __at (0x35) char buffer_available;
__idata __at (0x36) char shared_buffer;
__idata __at (0x37) char k_ch;
__idata __at (0x38) char ch;

void Producer1(){

    while (1) {
        if(AnyButtonPressed() ){
            ch = ButtonToChar();

            SemaphoreWait(empty);
            //SemaphoreWait(turn2);
            SemaphoreWait(mutex);
            __critical{
                shared_buffer = ch;
                //buffer_available = 0;
            }
            SemaphoreSignal(mutex);
            //SemaphoreSignal(turn2);
            SemaphoreSignal(full);

            while(AnyButtonPressed()) {}

        }

    }
 }
 void Producer2(){
    //buffer_available == 1;
    while (1) {
        if(AnyKeyPressed()){

            k_ch = KeyToChar();

            SemaphoreWait(empty);
            //SemaphoreWait(turn2);
            SemaphoreWait(mutex);
            __critical{
                shared_buffer = k_ch;
                //buffer_available = 0;
            }
            SemaphoreSignal(mutex);
            //SemaphoreSignal(turn2);
            SemaphoreSignal(full);
            while(AnyKeyPressed()) {}
        }
        //if(!AnyKeyPressed()){buffer_available = 1;}
    }
 }
void Consumer() {
    while (1) {
        while(!LCD_ready()){}
        //while(shared_buffer == '\0'){}
        SemaphoreWait(full);
        SemaphoreWait(mutex);
        __critical{
            LCD_write_char(shared_buffer);

            //shared_buffer = '\0';
            //buffer_available = 0;
            //while(!LCD_ready()){}
        }
        SemaphoreSignal(mutex);
        SemaphoreSignal(empty);
        //delay(40);
        //while(!LCD_ready()){}

    }
}

void main(){
    LCD_Init();
    Init_Keypad();
    SemaphoreCreate(empty,1);
    SemaphoreCreate(full, 0);
    SemaphoreCreate(mutex, 1);
    //SemaphoreCreate(turn1, 0);
    SemaphoreCreate(turn2, 1);

    ThreadCreate(&Producer1);
    ThreadCreate(&Producer2);

    //ThreadCreate(&Producer2);
    Consumer(); // like exec()
}

void _sdcc_gsinit_startup(void) {
  __asm
    ljmp  _Bootstrap
  __endasm;
}

void _mcs51_genRAMCLEAR(void) {}
void _mcs51_genXINIT(void) {}
void _mcs51_genXRAMCLEAR(void) {}

void timer0_ISR(void) __interrupt(1) {
  __asm
    ljmp _myTimer0Handler
  __endasm;
}

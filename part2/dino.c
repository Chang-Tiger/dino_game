#include <8051.h>
#include "lcdlib.h"
#include "keylib.h"
#include "preemptive.h"
__idata __at (0x2f) char change_pos;
__idata __at (0x30) char empty;
__idata __at (0x31) char full;
__idata __at (0x32) char mutex;
__idata __at (0x33) char score;
__idata __at (0x34) char at0;
__idata __at (0x35) char level;
__idata __at (0x36) char shared_buffer;
__idata __at (0x37) char k_ch;
__idata __at (0x3a) char pos_c0[2];//0x3a~0x3b
__idata __at (0x3c) char pos_c1[3];//0x3c~0x3e


char intToChar(int num) {
    return (char)(num + '0');
}
void shift_screen1(){

    char i = 2;
    at0 = 2;

    while(i--){

        while(!LCD_ready()){}
        LCD_cursorGoTo(0, pos_c0[i]);
        LCD_write_char(' ');
        //pos_c0[i] = (pos_c0[i] == 0) ? 14 : (pos_c0[i] - 1);
        if(pos_c0[i]!=0){pos_c0[i]--;}
        else {pos_c0[i] = 14; score++;}//don't use 15

        if(pos_c0[i] == 0){at0 = 0;}

        while(!LCD_ready()){}
        LCD_cursorGoTo(0, pos_c0[i]);
        LCD_write_char('\0');
    }
    i = 3;
    while(i--){
        while(!LCD_ready()){}
        LCD_cursorGoTo(1, pos_c1[i]);
        LCD_write_char(' ');
        //pos_c1[i] = (pos_c1[i] == 0) ? 14 : (pos_c1[i] - 1);
        if(pos_c1[i]!=0){pos_c1[i]--;}
        else {pos_c1[i] = 14; score++;}


        if(pos_c1[i] == 0){at0 = 1;}

        while(!LCD_ready()){}
        LCD_cursorGoTo(1, pos_c1[i]);
        LCD_write_char('\0');
    }
}
void end_scene(){
    LCD_cursorGoTo(0,8);
    char s = 7;
    while(s--){
        LCD_write_char(' ');
    }
    LCD_cursorGoTo(1,1);
    s = 14;
    while(s--){
        LCD_write_char(' ');
    }
    LCD_cursorGoTo(0, 0);
    LCD_write_char('G');
    LCD_write_char('A');
    LCD_write_char('M');
    LCD_write_char('E');
    LCD_write_char(' ');
    LCD_write_char('O');
    LCD_write_char('V');
    LCD_write_char('E');
    LCD_write_char('R');
    s = intToChar(--score);
    LCD_cursorGoTo(1, 0);
    LCD_write_char(s);

}
void render_task(){
    int k;
    while (shared_buffer != 3) {
        SemaphoreWait(empty);
        SemaphoreWait(mutex);

        shift_screen1();

        SemaphoreSignal(mutex);
        SemaphoreSignal(full);
        k = level;
        delay(200);
        delay(200);
        delay(200);
        while(k--) { delay(200);}
        while(!LCD_ready()){}
    }
    end_scene();

    while(1){}

 }
 void keypad_ctrl(){
    while (1) {
        if(AnyKeyPressed()){
            k_ch = KeyToChar();
            if(k_ch == '8' & shared_buffer == 0){
                SemaphoreWait(empty);
                SemaphoreWait(mutex);
                while(!LCD_ready()){}
                LCD_cursorGoTo(shared_buffer, 0);

                shared_buffer = 1;

                LCD_write_char(' ');
                while(!LCD_ready()){}
                LCD_cursorGoTo(shared_buffer, 0);
                LCD_write_char('\2');

                SemaphoreSignal(mutex);
                SemaphoreSignal(full);
                while(AnyKeyPressed()) {}
            }else if(k_ch == '2' & shared_buffer == 1){
                SemaphoreWait(empty);
                SemaphoreWait(mutex);
                while(!LCD_ready()){}
                LCD_cursorGoTo(shared_buffer, 0);

                shared_buffer = 0;

                LCD_write_char(' ');
                while(!LCD_ready()){}
                LCD_cursorGoTo(shared_buffer, 0);
                LCD_write_char('\2');
                SemaphoreSignal(mutex);
                SemaphoreSignal(full);
                while(AnyKeyPressed()) {}
            }else{
                while(AnyKeyPressed()) {}
            }
        }
    }
 }
void game_control() {

    while (1) {
        SemaphoreWait(full);
        SemaphoreWait(mutex);
        //判斷state
        if(at0 == shared_buffer){
            shared_buffer = 3;
        }
        SemaphoreSignal(mutex);
        SemaphoreSignal(empty);


    }
}
void initial_screen(){
    score = 0;
    LCD_cursorGoTo(0, 0);
    LCD_write_char('\2');
    shared_buffer = 0;
    LCD_cursorGoTo(0, 3);
    LCD_write_char('\0');
    pos_c0[0] = 3;
    LCD_cursorGoTo(0, 11);
    LCD_write_char('\0');
    pos_c0[1] = 11;
    LCD_cursorGoTo(1, 5);
    LCD_write_char('\0');
    pos_c1[0] = 5;
    LCD_cursorGoTo(1, 7);
    LCD_write_char('\0');
    pos_c1[1] = 7;
    LCD_cursorGoTo(1, 13);
    LCD_write_char('\0');
    pos_c1[2] = 13;
    //LCD_cursorGoTo(0, 0);
}
void main(){
    const char dinosaur[] = {0x07, 0x05, 0x06, 0x07, 0x14, 0x17, 0x0E, 0x0A};
    const char cactus[] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};
    LCD_Init();
    Init_Keypad();
    LCD_set_symbol(0x10, dinosaur); // bitmap for dinosaur starts at 0x10
    LCD_set_symbol(0x00, cactus); // bitmap for cactus starts at 0x20
    //LCD_set_symbol(0x10, dinosaur); // bitmap for dinosaur starts at 0x10
    SemaphoreCreate(empty,1);
    SemaphoreCreate(full, 0);
    SemaphoreCreate(mutex, 1);
    initial_screen();

    while(1){
        if(AnyKeyPressed()){

            if(KeyToChar() == '#'){
                while(AnyKeyPressed()) {}
                //LCD_write_char('1');
                while(1){
                    if(AnyKeyPressed()){
                        level = KeyToChar() - '0';
                        level = (level > 9) ? 0 : level;
                        while(AnyKeyPressed()) {}
                        break;
                    }
                }
                break;

            }else{
                while(AnyKeyPressed()) {}
            }
        }
    }



    ThreadCreate(&game_control);
    ThreadCreate(&keypad_ctrl);

    render_task();
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

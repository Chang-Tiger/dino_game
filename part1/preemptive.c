#include <8051.h>

#include "preemptive.h"


#define MAXTHREADS 4
#define SP_INIT 0x40

//__idata __at (0x20) char current_thread_id;
__idata __at (0x1f) char current_thread_id;
__idata __at (0x21) char threads_num;
__idata __at (0x22) char threads_available; // 1: in use
// __idata __at (0x23) int reserved;
__idata __at (0x24) char stack_pointers[MAXTHREADS]; // 0x24~0x27

__idata __at (0x28) char i;
__idata __at (0x29) char tmp;
__idata __at (0x2A) char tmp_;
#define SAVESTATE {    \
            __asm \
            push ACC \
            push B \
            push DPL \
            push DPH \
            push PSW \
            __endasm; \
            stack_pointers[current_thread_id] = SP; \
        }

#define RESTORESTATE { \
    SP = stack_pointers[current_thread_id]; \
    __asm \
    pop PSW \
    pop DPH \
    pop DPL \
    pop B \
    pop ACC \
    __endasm; \
}


extern void main(void);

void Bootstrap(void){
    SP = 0x07;//這行問題嗎？
    threads_num = 0;
    threads_available = 0;
    for (i = 0; i < MAXTHREADS; i++) {
        stack_pointers[i] = 0;
    }
    TMOD = 0x00;//mode 0
    IE = 0x82;
    TR0 = 1;//start timer0

    current_thread_id = ThreadCreate(&main);
    RESTORESTATE;
}

ThreadID ThreadCreate(FunctionPtr fp)__critical {
    if (threads_num >= MAXTHREADS) { return -1;}
    for (i = 0; i < MAXTHREADS; i++) {
        if ((threads_available & (1 << i)) == 0) {
            break;
        }
    }

    threads_available += 1 << i;
    tmp = SP;
    threads_num = threads_num + 1;

    SP = ((4 + i) << 4) - 1;
    { __asm
      push DPL
      push DPH
      __endasm;
    };
    tmp_ = 0;
    { __asm
      push _tmp_
      push _tmp_
      push _tmp_
      push _tmp_
      __endasm;
    };

    tmp_  = (i << 3);
    { __asm
      push _tmp_
      __endasm;
    };

    stack_pointers[i] = SP;
    SP = tmp;
    return i;
}

void ThreadYield(void) {//換下個可用的thread,過程不可中斷
    EA = 0;
    SAVESTATE;
    i = current_thread_id;
    while (1) {
        i++;
        if (i >= MAXTHREADS) {
            i = 0;
        }

        if ((threads_available >> i) & 1) {
            break;
        }
    }
    current_thread_id = i;

    RESTORESTATE;
    EA = 1;
}


void ThreadExit(void) {
    RESTORESTATE;
}

void myTimer0Handler(void) {
    EA = 0;
    SAVESTATE;
    { __asm
        mov DPH, r0
        mov DPL, r1
        __endasm;
    };

    /*
    * TODO:  [8 pts] do round-robin policy for now.
    * find the next thread that can run and
    * set the current thread ID to it,
    * so that it can be restored (by the last line of
    * this function).
    * there should be at least one thread, so this loop
    * will always terminate.
    */
    i = current_thread_id;
    do {
        i++;
        if (i >= MAXTHREADS) {
        i = 0;
        }

        if ((threads_available >> i) & 1) {
        break;
        }
    } while (1);
    current_thread_id = i;

    { __asm
        mov r0, DPH
        mov r1, DPL
        __endasm;
    };
    RESTORESTATE;
    EA = 1;
    { __asm
        reti
      __endasm;
    };
}

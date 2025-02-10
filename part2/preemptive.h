#ifndef __PREEMPTIVE_H__
#define __PREEMPTIVE_H__

#define MAXTHREADS 4

typedef char ThreadID;
typedef void (*FunctionPtr)(void);

ThreadID ThreadCreate(FunctionPtr);
void ThreadYield(void);
void ThreadExit(void);
void myTimer0Handler(void);

#define CNAME(s) _ ## s
#define SemaphoreCreate(s, n) \
  __critical { \
    s = n; \
};

#define SemaphoreSignal(s) \
  { __asm \
    INC CNAME(s) \
  __endasm; }

#define SemaphoreWait(s) \
  while (s == 0); \
  __critical { \
    s--; \
};

#endif // __PREEMPTIVE_H__


#include "libultra_internal.h"

void __osCleanupThread(void);

// Don't warn about pointer->u64 cast
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"

void osCreateThread(OSThread *thread, OSId id, void (*entry)(void *), void *arg, void *sp, OSPri pri) {
#ifndef PC_PORT
    register u32 int_disabled;
#endif
    u32 tmp;
    thread->id = id;
    thread->priority = pri;
    thread->next = NULL;
    thread->queue = NULL;
    thread->context.pc = (u32) entry;
    thread->context.a0 = (u64) arg;
    thread->context.sp = (u64) sp - 16;
#ifndef PC_PORT
    thread->context.ra = (u64) __osCleanupThread;
#endif
    tmp = OS_IM_ALL;
    thread->context.sr = 65283;
    thread->context.rcp = (tmp & 0x3f0000) >> 16;
    thread->context.fpcsr = (u32) 0x01000800;
    thread->fp = 0;
    thread->state = OS_STATE_STOPPED;
    thread->flags = 0;
#ifndef PC_PORT
    int_disabled = __osDisableInt();
#endif
    thread->tlnext = D_8033489C;

    D_8033489C = thread;
#ifndef PC_PORT
    __osRestoreInt(int_disabled);
#endif
}

#pragma GCC diagnostic pop

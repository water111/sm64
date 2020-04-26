#include "libultra_internal.h"

s32 osSendMesg(OSMesgQueue *mq, OSMesg msg, s32 flag) {
#ifndef PC_PORT
    register u32 int_disabled;
    register s32 index;
    register OSThread *s2;
    int_disabled = __osDisableInt();

    while (mq->validCount >= mq->msgCount) {
        if (flag == OS_MESG_BLOCK) {
            D_803348A0->state = 8;
            __osEnqueueAndYield(&mq->fullqueue);
        } else {
            __osRestoreInt(int_disabled);
            return -1;
        }
    }

    index = (mq->first + mq->validCount) % mq->msgCount;
    mq->msg[index] = msg;
    mq->validCount++;

    if (mq->mtqueue->next != NULL) {
        s2 = __osPopThread(&mq->mtqueue);
        osStartThread(s2);
    }

    __osRestoreInt(int_disabled);
#else
  (void)mq;
  (void)msg;
  (void)flag;
#endif
    return 0;
}

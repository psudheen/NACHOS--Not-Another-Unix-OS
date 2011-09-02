
#include "synch.h"


#define MAX_OT 100

enum WorkerStatus { FREE =0, BUSY, WAITING};


/* Bunch of data related to OrderTaker */
struct OrderTaker_t {
    int CustLineLen;
    // Lock, CV and Monitor Variables that all OTs' share.
    Lock *OrderTakerLock;
    /* OrderTaker line CV */
    Condition *OrderTakerCV;
    Condition *OrderTakerWaitCV;
    int currentToken;
    /* OrderTaker break CV */
    Condition *OrderTakerBreakCV;

    WorkerStatus *OTStatus;

};
typedef struct OrderTaker_t OrderTakers;
OrderTakers OT[MAX_OT];


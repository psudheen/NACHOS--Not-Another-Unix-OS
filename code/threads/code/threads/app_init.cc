#include "app_init.h"

void OrderTakers_init(OrderTakers *OT )
{
    OT->CustLineLen = 0;
    OT->OrderTakerCV = new Condition("OrderTakerCV");
    OT->OrderTakerLock = new Lock("OrderTakerLock");
    OT->OrderTakerWaitCV = new Condition("OrderTakerWaitCV");
    OT->OrderTakerBreakCV = new Condition("OrderTakerBreakCV");
    OT->currentToken = 0;
    *(OT->OTStatus) = BUSY;
}

OrderTakers_init(OT);
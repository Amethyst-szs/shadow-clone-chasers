#include "layouts/LayoutHolder.h"
#include "layouts/GenericStat.h"
#include "layouts/TestLayout.h"

cl::CLayoutHolder& cl::getCLayoutHolder()
{
    static CLayoutHolder i;
    return i;
}

void cl::CLayoutHolder::init(al::LayoutInitInfo const* lytInfo)
{
    mTestLayout = new TestLayout("TestLayout", *lytInfo);
    mTestLayout->tryStart();
}

void cl::CLayoutHolder::endAll()
{
    mTestLayout->tryEnd();
}
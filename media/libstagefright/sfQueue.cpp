//#include <media/stagefright/MediaExtractor.h>
#include <utils/Errors.h>
#include <utils/threads.h>
#include "include/sfQueue.h"


namespace android {
status_t sfQueue::sfQueueCreate(uint32_t uMaxEntries, uint32_t uEntrySize)
{
    Mutex::Autolock autoLock(mLock);
    sfQueueRec *pQueue;
    status_t eError = OK;

    if ( !(uMaxEntries > 0) && !(uEntrySize > 0))
        return BAD_VALUE;
    pQueue = new sfQueueRec;
    if (!pQueue)
        return NO_MEMORY;
    memset(pQueue, 0, sizeof(sfQueueRec));

    pQueue->pEntryList   = 0;
    pQueue->uPushIndex   = 0;
    pQueue->uPopIndex    = 0;
    pQueue->uMaxEntries  = uMaxEntries + 1;
    pQueue->uEntrySize   = uEntrySize;

    pQueue->pEntryList = new uint8_t[pQueue->uMaxEntries * uEntrySize];
    if (!pQueue->pEntryList)
    {
        eError = NO_MEMORY;
        goto sf_queue_exit;
    }
    hsfQueue = pQueue;
    return OK;

sf_queue_exit:
    if (pQueue)
    {
        delete pQueue;
        pQueue = NULL;
    }
    return eError;
}

status_t sfQueue::sfQueueEnQ( void *pElem, uint32_t uTimeout)
{
    Mutex::Autolock autoLock(mLock);
    sfQueueRec *pQueue = hsfQueue;
    status_t eError = OK;
    uint32_t uPush, uPop;

   uPush = pQueue->uPushIndex;
    uPop = pQueue->uPopIndex;

    if (uPush + 1 == uPop || uPush + 1 == uPop + pQueue->uMaxEntries)
    {
        eError = NO_MEMORY;
        goto sf_queue_exit;
    }

    memcpy(&pQueue->pEntryList[uPush * pQueue->uEntrySize], pElem,
               pQueue->uEntrySize);

    if (++uPush >= pQueue->uMaxEntries)
        uPush = 0;
    pQueue->uPushIndex = uPush;

sf_queue_exit:
    return eError;
}

status_t sfQueue::sfQueueDeQ(void *pElem)
{
    Mutex::Autolock autoLock(mLock);
    sfQueueRec *pQueue = hsfQueue;
    status_t eError = OK;
    uint32_t uPop;

    uPop = pQueue->uPopIndex;
    if (pQueue->uPushIndex == uPop)
    {
        eError = UNKNOWN_ERROR;
        return eError;
    }

    memcpy(pElem, &pQueue->pEntryList[uPop * pQueue->uEntrySize],
               pQueue->uEntrySize);

    if (++uPop >= pQueue->uMaxEntries)
        uPop = 0;
    pQueue->uPopIndex = uPop;
    return eError;
}

status_t sfQueue::sfQueuePeek(void *pElem)
{
    Mutex::Autolock autoLock(mLock);
    sfQueueRec *pQueue = hsfQueue;
    status_t eError = OK;
    uint32_t uPop;
    uPop = pQueue->uPopIndex;
    if (pQueue->uPushIndex == uPop)
    {
        eError = BAD_VALUE;
        return eError;
    }

    memcpy(pElem, &pQueue->pEntryList[uPop * pQueue->uEntrySize],
               pQueue->uEntrySize);
    return eError;
}

status_t sfQueue::sfQueuePeekEntry(void *pElem, uint32_t nEntry)
{
    Mutex::Autolock autoLock(mLock);
    sfQueueRec *pQueue = hsfQueue;
    status_t eError = OK;
    uint32_t entry, uPush, uPop, uNumEntries;
    uPush = pQueue->uPushIndex;
    uPop = pQueue->uPopIndex;

    uNumEntries = (uPush >= uPop) ? uPush - uPop :
                                    pQueue->uMaxEntries - uPop + uPush;

    if ((uNumEntries == 0) || (uNumEntries <= nEntry))
    {
        eError = BAD_VALUE;
    return eError;
    }

    entry = uPop + nEntry;
    if (entry >= pQueue->uMaxEntries)
        entry -= pQueue->uMaxEntries;

    memcpy(pElem, &pQueue->pEntryList[entry * pQueue->uEntrySize],
               pQueue->uEntrySize);

    return eError;
}

void sfQueue::sfQueueDestroy()
{
    Mutex::Autolock autoLock(mLock);
    sfQueueRec *pQueue = hsfQueue;
    if (!pQueue)
        return;
    delete []pQueue->pEntryList;
    pQueue->pEntryList = NULL;
    delete pQueue;
    pQueue = NULL;
}

uint32_t sfQueue::sfQueueGetNumEntries()
{
    Mutex::Autolock autoLock(mLock);
    sfQueueRec *pQueue = hsfQueue;
    uint32_t uPush = pQueue->uPushIndex;
    uint32_t uPop = pQueue->uPopIndex;
    uint32_t uNumEntries = (uPush >= uPop) ? uPush - uPop :
                                          pQueue->uMaxEntries - uPop + uPush;
    return uNumEntries;
}

status_t sfQueue::sfQueueInsertHead(void *pElem, uint32_t uTimeout)
{
    Mutex::Autolock autoLock(mLock);
    sfQueueRec *pQueue = hsfQueue;
    status_t eError = OK;
    uint32_t uPush, uPop;
    uPush = pQueue->uPushIndex;
    uPop = pQueue->uPopIndex;

    // Check if there's room
    if (uPush + 1 == uPop || uPush + 1 == uPop + pQueue->uMaxEntries)
    {
        eError = NO_MEMORY;
    return eError;
    }

    if (uPop-- == 0)
        uPop = pQueue->uMaxEntries - 1;

    memcpy(&pQueue->pEntryList[uPop * pQueue->uEntrySize], pElem,
               pQueue->uEntrySize);

    pQueue->uPopIndex = uPop;


    return eError;
}
}
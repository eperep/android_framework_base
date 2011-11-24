#include <media/stagefright/Utils.h>

namespace android {

typedef struct sfQueueRec
{
    uint32_t uMaxEntries;                  /* maximum number of allowed entries */
    uint32_t uEntrySize;                   /* size of individual entry */
    uint32_t uPushIndex;                   /* index of where to push entry */
    uint32_t uPopIndex;                    /* index of where to grab entry */
    uint8_t  *pEntryList;                   /* pointer to beginning entry */
} ;
 typedef struct sfQueueRec *sfQueueHandle;
class sfQueue{

private:
    sfQueueHandle hsfQueue;
    Mutex mLock;
public:
    status_t sfQueueCreate(uint32_t MaxEntries, uint32_t EntrySize);
    status_t sfQueueEnQ( void *pElem, uint32_t Timeout);
    status_t sfQueueDeQ(void * pElem);
    status_t sfQueuePeek(void *pElem);
    status_t sfQueuePeekEntry(void *pElem, uint32_t nEntry);
    status_t sfQueueInsertHead( void *pElem, uint32_t uTimeout);
    void sfQueueDestroy();
    uint32_t  sfQueueGetNumEntries();
};
}

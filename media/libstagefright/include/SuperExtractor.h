/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef SUPER_EXTRACTOR_H_

#define SUPER_EXTRACTOR_H_

#include <media/IOMX.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/OMXClient.h>
#include <utils/Vector.h>
#include <media/stagefright/openmax/OMX_Types.h>
#include <media/stagefright/openmax/OMX_Core.h>
#include <media/stagefright/openmax/OMX_Component.h>
#include <utils/threads.h>
#include "include/sfQueue.h"
#include <NVOMX_TrackList.h>
#include <NVOMX_ParserExtensions.h>
#include <NVOMX_IndexExtensions.h>

static uint8_t ASF_Header_GUID[16] =
    { 0x30, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C };

#define COMMON_MAX_INPUT_BUFFER_SIZE       64 * 1024
#define MAX_INPUT_BUFFERS 30

namespace android {
struct AMessage;
class DataSource;
class SampleTable;
class String8;
class Condition;
enum State {
        DEAD,
        LOADED,
        LOADED_TO_IDLE,
        IDLE_TO_EXECUTING,
        EXECUTING,
        EXECUTING_TO_IDLE,
        EXECUTING_TO_PAUSE,
        PAUSE,
        PAUSE_TO_EXECUTING,
        IDLE_TO_LOADED,
        RECONFIGURING,
        ERROR
    };
typedef struct
{
    void *hobserver;
    IOMX::node_id node;
    OMXClient mClient;
    sp<IOMX> sOMX;
    OMX_PARAM_PORTDEFINITIONTYPE decInputPortDef;
    OMX_BUFFERHEADERTYPE *AudioinputBuffer[MAX_INPUT_BUFFERS];
    OMX_BUFFERHEADERTYPE *VideoinputBuffer[MAX_INPUT_BUFFERS];

    sfQueue EmptyAudioMsgQ;
    sfQueue EmptyVideoMsgQ;
    sfQueue FilledAudioMsgQ;
    sfQueue FilledVideoMsgQ;
    bool bEmptyAudioMsgQ;
    bool bEmptyVideoMsgQ;
    bool bFilledAudioMsgQ;
    bool bFilledVideoMsgQ;
    bool bPlayingMode;
    bool bThumbnailMode;
    bool bsentEOS;
    int mWorkerTid;
    OMX_HANDLETYPE hParser;
    Mutex mMutex;
    Mutex mLock;
    Condition hsema;
    Condition hvideosignal;
    Condition haudiosignal;
    Condition ParserFlushSema;
    Condition ParserSeekFlushSema;
    Condition hseek;
    Condition mAsyncCompletion;
    bool seeking;
    bool fillwait;
    uint32_t mess;
    uint32_t Count;
    uint32_t TrackCount;
    uint32_t NoOfVideoBuffers;
    uint32_t NoOfAudioBuffers;
    uint32_t VideoIndex;
    uint32_t AudioIndex;
    OMX_TICKS nDuration;
    ENvxStreamType eVideoStreamType; /**< Stream type */
    ENvxStreamType eAudioStreamType; /**< Stream type */
    bool EOS[2];
    bool IsVideo;
    bool IsAudio;
    bool bAudDone;
    bool bVidDone;
    bool bErrorOccured;
    bool mStopThread;
    State mState;
    void (*msgCallback)(void *hExtractor, const omx_message &msg);
    uint32_t audioBufferCount;
}SuperExtractorData;

static int SFOsSnprintf( char *str, size_t size, const char *format, ... );
static void setState(SuperExtractorData *m_hExtractor, State newState);
static bool isIntermediateState(State state);

class SuperExtractor : public MediaExtractor {
public:
    // Extractor assumes ownership of "source"
    SuperExtractor(const sp<DataSource> &source);

    virtual bool initParser();

    virtual size_t countTracks();
    virtual sp<MediaSource> getTrack(size_t index);
    virtual sp<MetaData> getTrackMetaData(size_t index, uint32_t flags);

    virtual sp<MetaData> getMetaData();
    bool IsError(void);

protected:
    virtual ~SuperExtractor();

private:

    enum {
        IS_INITIALIZED              = 1,
        HAS_SHARED_NODE             = 2,
        HAS_TRACK_COUNT             = 4,
        HAS_FILE_METADATA           = 8,
        HAS_TRACK0_METADATA         = 16,
        HAS_TRACK1_METADATA         = 32,
    };

    struct Track {
        sp<MetaData> meta;
        uint32_t timescale;
        bool includes_expensive_metadata;
    };
    SuperExtractorData *Extractor;
    SuperExtractorData *AudExtractor;
    sp<DataSource> mDataSource;
    bool mHaveMetadata;
    bool mHasVideo;
    bool IsAudio;
    bool IsVideo;
    uint32_t mTrackCount;
    Track mTracks[2];
    sp<MetaData> mFileMetaData;
    Vector<uint32_t> mPath;
    NvxTrackInfo oInfo;
    Mutex mLock;
    uint32_t mFlags;

    SuperExtractor(const SuperExtractor &);
    SuperExtractor &operator=(const SuperExtractor &);
};

bool SniffSuper(
        const sp<DataSource> &source, String8 *mimeType, float *confidence,
        sp<AMessage> *);
}  // namespace android

#endif



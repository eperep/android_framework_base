/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef A_ES_PARSER_H_
#define A_ES_PARSER_H_

#include <sys/types.h>
#include <media/stagefright/foundation/AMessage.h>
#include <utils/RefBase.h>

namespace android {

#define NUM_STREAMS 2

struct MediaSource;
struct AnotherPacketSource;
struct ElementaryStreamQueue;

#define isAudio(x) ((((x) == STREAMTYPE_MPEG2_AUDIO_ATDS) || ((x) == STREAMTYPE_MPEG1_AUDIO) || ((x) == STREAMTYPE_MPEG2_AUDIO)) ? 1 : 0)
#define isVideo(x) ((((x) == STREAMTYPE_H264) || ((x) == STREAMTYPE_MPEG1_VIDEO) || \
                     ((x) == STREAMTYPE_MPEG2_VIDEO) || ((x) == STREAMTYPE_MPEG4_VIDEO)) ? 1 : 0)


struct AESParser : public RefBase {

    enum DiscontinuityType {
        DISCONTINUITY_NONE,
        DISCONTINUITY_SEEK,
        DISCONTINUITY_FORMATCHANGE
    };

    enum SourceType {
        VIDEO,
        AUDIO
    };

    enum StreamType {
        // From ISO/IEC 13818-1: 2000 (E), Table 2-29
        STREAMTYPE_MPEG1_VIDEO          = 0x01,
        STREAMTYPE_MPEG2_VIDEO          = 0x02,
        STREAMTYPE_MPEG1_AUDIO          = 0x03,
        STREAMTYPE_MPEG2_AUDIO          = 0x04,
        STREAMTYPE_MPEG2_AUDIO_ATDS     = 0x0f,
        STREAMTYPE_MPEG4_VIDEO          = 0x10,
        STREAMTYPE_H264                 = 0x1b,
    };

    AESParser();

    void feedESPacket(StreamType sType, const void *data, size_t size, int64_t timeStamp);
    void signalDiscontinuity(SourceType sType, DiscontinuityType type, const sp<AMessage> &extra);
    void signalEOS(SourceType sType, status_t finalResult);

    sp<MediaSource> getSource(SourceType sType);

    unsigned type(SourceType sType) const { return mStreamType[sType]; }

protected:
    virtual ~AESParser();

private:

    void StreamInit(SourceType sType, StreamType streamType);
    void StreamDeInit(SourceType sType);
    int64_t convertPTSToTimestamp(SourceType sType, int64_t PTS);

    StreamType mStreamType[NUM_STREAMS];
    bool mStreamInitialized[NUM_STREAMS];
    bool mFirstPTSValid[NUM_STREAMS];
    int64_t mFirstPTS[NUM_STREAMS];

    sp<AnotherPacketSource> mSource[NUM_STREAMS];
    DiscontinuityType mPendingDiscontinuity[NUM_STREAMS];
    sp<AMessage> mPendingDiscontinuityExtra[NUM_STREAMS];

    ElementaryStreamQueue *mQueue[NUM_STREAMS];

    void onPayloadData(SourceType sType, const uint8_t *data, size_t size, int64_t timeStamp);
    void deferDiscontinuity(SourceType sType, DiscontinuityType type, const sp<AMessage> &extra);

    DISALLOW_EVIL_CONSTRUCTORS(AESParser);
};


}  // namespace android

#endif  // A_ES_PARSER_H_

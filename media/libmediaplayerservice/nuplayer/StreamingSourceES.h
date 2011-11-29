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

#ifndef STREAMING_SOURCE_ES_H_

#define STREAMING_SOURCE_ES_H_

#include "NuPlayer.h"
#include "NuPlayerSource.h"

namespace android {

struct ABuffer;
struct AESParser;

struct NuPlayer::StreamingSourceES : public NuPlayer::Source {

    enum FeedMoreDataMethodES {
        NVIDIA = 0,
        H264NAL,
        AAC_ADTS,
    };

    StreamingSourceES(const sp<IStreamSource> &source, FeedMoreDataMethodES method);

    virtual void start();

    // Returns true if more data was available, false on EOS.
    virtual status_t feedMoreTSData();

    virtual sp<MetaData> getFormat(bool audio);
    virtual status_t dequeueAccessUnit(bool audio, sp<ABuffer> *accessUnit);

protected:
    virtual ~StreamingSourceES();

private:

    bool CreateMemory(int32_t memSize);
    void DestroyMemory();
    status_t feedMoreNESData();
    status_t feedMoreESData();

    sp<IStreamSource> mSource;
    FeedMoreDataMethodES mMethod;
    status_t mFinalResult;
    sp<NuPlayerStreamListener> mStreamListener;
    sp<AESParser> mESParser;

    AESParser::SourceType mSourceType;
    AESParser::StreamType mStreamType;

    void *m_pBuffer;
    int32_t mFrameSize;
    int32_t mCurrentMemorySize;
    int32_t mAmountRead;
    char mNvESHeader[16];
    bool mBlockedHeader;
    bool mBlockedFrame;
    int64_t mTimeStamp;

    DISALLOW_EVIL_CONSTRUCTORS(StreamingSourceES);
};

}  // namespace android

#endif  // STREAMING_SOURCE_ES_H_

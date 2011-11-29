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

//#define LOG_NDEBUG 0
#define LOG_TAG "StreamingSourceES"
#include <utils/Log.h>

#include "AESParser.h"
#include "StreamingSourceES.h"
#include "AnotherPacketSource.h"
#include "NuPlayerStreamListener.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>
#include <cutils/properties.h>

namespace android {

// This is our original buffer size, but it can and does get larger
#define StartingBufferSize  8192

NuPlayer::StreamingSourceES::StreamingSourceES(const sp<IStreamSource> &source, FeedMoreDataMethodES method)
    : mSource(source),
      mMethod(method),
      mFinalResult(OK)
{
    //LOGI("StreamingSourceES CONSTRUCTOR");
    mAmountRead = 0;
    mFrameSize = 0;
    mBlockedHeader = false;
    mBlockedFrame = false;
    mTimeStamp = 0LL;
    CreateMemory(StartingBufferSize);
}

NuPlayer::StreamingSourceES::~StreamingSourceES()
{
    //LOGI("StreamingSourceES DESTRUCTOR");
    DestroyMemory();
}

bool NuPlayer::StreamingSourceES::CreateMemory(int32_t memSize)
{
    //LOGI("StreamingSourceES::CreateMemory Enter (size: x%x)", memSize);

    mCurrentMemorySize = memSize;

    m_pBuffer = malloc(memSize);
    if (!m_pBuffer)
    {
        LOGE("StreamingSourceES::CreateMemory Cant create memory!");
        mCurrentMemorySize = 0;
        return false;
    }

    return true;
}
void NuPlayer::StreamingSourceES::DestroyMemory()
{
    //LOGI("StreamingSourceES::DestroyMemory Enter (size: x%x)", mCurrentMemorySize);

    if (m_pBuffer)
    {
        free(m_pBuffer);
        m_pBuffer = 0;
        mCurrentMemorySize = 0;
    }
}

void NuPlayer::StreamingSourceES::start()
{
    //LOGI("StreamingSourceES::start");
    mStreamListener = new NuPlayerStreamListener(mSource, 0);
    mESParser = new AESParser;
    mStreamListener->start();
}

status_t NuPlayer::StreamingSourceES::feedMoreTSData()
{
    status_t status = OK;

    switch (mMethod)
    {
        case NVIDIA:
             status = feedMoreNESData();
             break;

        case AAC_ADTS:
        case H264NAL:
             status = feedMoreESData();
             break;

        default:
             break;
    }

    return status;
}

// Version with Nvidia headers
status_t NuPlayer::StreamingSourceES::feedMoreNESData()
{
    //LOGI("StreamingSourceES::feedMoreTSData - ENTER (NV Headers)");
    if (mFinalResult != OK)
    {
        return mFinalResult;
    }

    bool notBlocked = !mBlockedHeader && !mBlockedFrame;

    for (int32_t i = 0; i < 10; ++i)
    {
        int32_t headerSize = sizeof(mNvESHeader);
        ssize_t n;
        sp<AMessage> extra;

        if (notBlocked || mBlockedHeader)
        {
            do
            {
                char* pReadPtr = (char *)mNvESHeader + mAmountRead;
                uint32_t sizeToRead = headerSize - mAmountRead;

                //LOGI("ABOUT TO READ! %x %x %x", sizeToRead, headerSize, mAmountRead);
                n = mStreamListener->read(pReadPtr, sizeToRead, &extra);
                //LOGI("headerSize: x%x n: x%x", headerSize, (int32_t)n);
                mBlockedHeader = false;
                notBlocked = true;

                if (n == 0)
                {
                    LOGI("input data EOS reached.");
                    mESParser->signalEOS(AESParser::AUDIO, ERROR_END_OF_STREAM);
                    mESParser->signalEOS(AESParser::VIDEO, ERROR_END_OF_STREAM);
                    mFinalResult = ERROR_END_OF_STREAM;
                    goto end;
                }
                else if (n == INFO_DISCONTINUITY)
                {
                    LOGI("input data INFO_DISCONTINUITY.");
                    AESParser::DiscontinuityType type = AESParser::DISCONTINUITY_SEEK;
                    int32_t formatChange;
                    if (extra != NULL && extra->findInt32(IStreamListener::kKeyFormatChange, &formatChange) && formatChange != 0)
                    {
                        type = AESParser::DISCONTINUITY_FORMATCHANGE;
                    }
                    mESParser->signalDiscontinuity(mSourceType, type, extra);
                }
                else if (n < 0)
                {
                    //LOGI("BLOCKED 1!");
                    mBlockedHeader = true;
                    CHECK_EQ(n, -EWOULDBLOCK);
                    goto end;
                }
                else if ((n + mAmountRead) < headerSize)
                {
                    mAmountRead += n;
                    //LOGI("mAmountRead1: x%x", mAmountRead);
                }
                else
                {
                    mAmountRead += n;
                    if (mAmountRead != headerSize)
                        LOGE("ERROR mAmountRead: %x  NOT EQUAL TO headerSize: %x", mAmountRead, headerSize);

                    //LOGI("mNvESHeader: %02x %02x %02x %02x %02x %02x %02x %02x",
                    //     mNvESHeader[0], mNvESHeader[1], mNvESHeader[2], mNvESHeader[3],
                    //     mNvESHeader[4], mNvESHeader[5], mNvESHeader[6], mNvESHeader[7]);

                    if (memcmp(mNvESHeader, "nesa", 4) == 0)
                    {
                        mStreamType = AESParser::STREAMTYPE_MPEG2_AUDIO_ATDS;
                        mSourceType = AESParser::AUDIO;
                    }
                    else if (memcmp(mNvESHeader, "nesv", 4) == 0)
                    {
                        mStreamType = AESParser::STREAMTYPE_H264;
                        mSourceType = AESParser::VIDEO;
                    }
                    else
                    {
                        LOGE("feedMoreTSData: Illegal Header!!!!");
                        break;
                    }

                    mFrameSize = *((uint32_t*)&mNvESHeader[4]);
                    if (mFrameSize <= 0)
                    {
                        LOGE("feedMoreTSData: Illegal Header Size: x%x!!!!", mFrameSize);
                        break;
                    }
                    mTimeStamp = *((uint64_t*)&mNvESHeader[8]);
                    //LOGI("TS READ(%s): %llu", (mSourceType == AESParser::VIDEO) ? "V" : "A", mTimeStamp);
                }

            } while (mAmountRead != headerSize);

            mAmountRead = 0;

            //LOGI("feedMoreTSData(%s): mFrameSize: x%x", ((mSourceType == AESParser::VIDEO) ? "VIDEO" : "AUDIO"), mFrameSize);

            // If we get a frame larger than mCurrentMemorySize, we must create a new larger block
            if (mCurrentMemorySize < mFrameSize)
            {
                DestroyMemory();
                if ( !CreateMemory(mFrameSize) )
                    break;
            }
        }

        if (notBlocked || mBlockedFrame)
        {
            do
            {
                char* pReadPtr = (char *)m_pBuffer + mAmountRead;
                uint32_t sizeToRead = mFrameSize - mAmountRead;

                n = mStreamListener->read(pReadPtr, sizeToRead, &extra);
                //LOGI("READ 2: n: x%x mAmountRead: x%x", (uint32_t)n, mAmountRead);
                mBlockedFrame = false;
                notBlocked = true;

                if (n == 0)
                {
                    LOGI("input data EOS reached.");
                    mESParser->signalEOS(AESParser::AUDIO, ERROR_END_OF_STREAM);
                    mESParser->signalEOS(AESParser::VIDEO, ERROR_END_OF_STREAM);
                    mFinalResult = ERROR_END_OF_STREAM;
                    goto end;
                }
                else if (n == INFO_DISCONTINUITY)
                {
                    LOGI("input data INFO_DISCONTINUITY.");
                    AESParser::DiscontinuityType type = AESParser::DISCONTINUITY_SEEK;
                    int32_t formatChange;
                    if (extra != NULL && extra->findInt32(IStreamListener::kKeyFormatChange, &formatChange) && formatChange != 0)
                    {
                        type = AESParser::DISCONTINUITY_FORMATCHANGE;
                    }
                    mESParser->signalDiscontinuity(mSourceType, type, extra);
                }
                else if (n < 0)
                {
                    //LOGI("BLOCKED 2!");
                    mBlockedFrame = true;
                    CHECK_EQ(n, -EWOULDBLOCK);
                    goto end;
                }
                else if ((n + mAmountRead) < mFrameSize)
                {
                    mAmountRead += n;
                    //LOGI("mAmountRead2: x%x", mAmountRead);
                }
                else
                {
                    mAmountRead += n;
                    if (mAmountRead != mFrameSize)
                        LOGE("ERROR mAmountRead: %x  NOT EQUAL TO mFrameSize: %x", mAmountRead, mFrameSize);
                    //LOGI("Writing: mAmountRead: x%x", mAmountRead);
                    mESParser->feedESPacket(mStreamType, m_pBuffer, mAmountRead, mTimeStamp);
                }

            } while (mAmountRead != mFrameSize);

            mAmountRead = 0;
        }
    }

end:
    return OK;
}

// Version with no headers
status_t NuPlayer::StreamingSourceES::feedMoreESData()
{
    //LOGI("StreamingSourceES::feedMoreTSData - ENTER (No Headers)");
    if (mFinalResult != OK)
    {
        return mFinalResult;
    }

    if (mMethod == H264NAL)
    {
        mStreamType = AESParser::STREAMTYPE_H264;
        mSourceType = AESParser::VIDEO;
    }
    else
    {
        mStreamType = AESParser::STREAMTYPE_MPEG2_AUDIO_ATDS;
        mSourceType = AESParser::AUDIO;
    }

    for (int32_t i = 0; i < 10; ++i)
    {
        char buffer[188*20];
        sp<AMessage> extra;
        ssize_t n = mStreamListener->read(buffer, sizeof(buffer), &extra);

        if (n == 0)
        {
            LOGI("input data EOS reached.");
            mESParser->signalEOS(mSourceType, ERROR_END_OF_STREAM);
            mFinalResult = ERROR_END_OF_STREAM;
            break;
        }
        else if (n == INFO_DISCONTINUITY)
        {
            AESParser::DiscontinuityType type = AESParser::DISCONTINUITY_SEEK;

            int32_t formatChange;
            if (extra != NULL && extra->findInt32(IStreamListener::kKeyFormatChange, &formatChange) && formatChange != 0)
            {
                type = AESParser::DISCONTINUITY_FORMATCHANGE;
            }

            mESParser->signalDiscontinuity(mSourceType, type, extra);
        }
        else if (n < 0)
        {
            CHECK_EQ(n, -EWOULDBLOCK);
            break;
        }
        else
        {
            mESParser->feedESPacket(mStreamType, buffer, n, 0);
        }
    }

    return OK;
}

sp<MetaData> NuPlayer::StreamingSourceES::getFormat(bool audio) {
    AESParser::SourceType type =
        audio ? AESParser::AUDIO : AESParser::VIDEO;

    sp<AnotherPacketSource> source =
        static_cast<AnotherPacketSource *>(mESParser->getSource(type).get());

    if (source == NULL) {
        return NULL;
    }

    return source->getFormat();
}

status_t NuPlayer::StreamingSourceES::dequeueAccessUnit(
        bool audio, sp<ABuffer> *accessUnit) {
    AESParser::SourceType type =
        audio ? AESParser::AUDIO : AESParser::VIDEO;

    sp<AnotherPacketSource> source =
        static_cast<AnotherPacketSource *>(mESParser->getSource(type).get());

    if (source == NULL) {
        return -EWOULDBLOCK;
    }

    status_t finalResult;
    if (!source->hasBufferAvailable(&finalResult)) {
        return finalResult == OK ? -EWOULDBLOCK : finalResult;
    }

    //LOGI("dequeue source type %s", audio ? "AUDIO" : "VIDEO");
    return source->dequeueAccessUnit(accessUnit);
}

}  // namespace android


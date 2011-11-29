/*
 * Copyright (C) 2010 The Android Open Source Project
 * Copyright (C) 2011 NVIDIA Corporation
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

#define LOG_NDEBUG 0
#define LOG_TAG "AESParser"
#include <utils/Log.h>

#include "AESParser.h"
#include "mpeg2ts/ATSParser.h"
#include "mpeg2ts/AnotherPacketSource.h"
#include "mpeg2ts/ESQueue.h"
#include "include/avc_utils.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MetaData.h>
#include <media/IStreamSource.h>

namespace android {

void AESParser::StreamInit(SourceType sType, StreamType streamType)
{
    LOGV("AESParser::StreamInit");
    mStreamType[sType] = streamType;
    mPendingDiscontinuity[sType] = DISCONTINUITY_NONE;
    mQueue[sType] = NULL;
    mStreamInitialized[sType] = true;

    switch (mStreamType[sType]) {
        case STREAMTYPE_H264:
            mQueue[sType] = new ElementaryStreamQueue(ElementaryStreamQueue::H264);
            break;
        case STREAMTYPE_MPEG2_AUDIO_ATDS:
            mQueue[sType] = new ElementaryStreamQueue(ElementaryStreamQueue::AAC);
            break;
        case STREAMTYPE_MPEG1_AUDIO:
        case STREAMTYPE_MPEG2_AUDIO:
            mQueue[sType] = new ElementaryStreamQueue(ElementaryStreamQueue::MPEG_AUDIO);
            break;
        case STREAMTYPE_MPEG1_VIDEO:
        case STREAMTYPE_MPEG2_VIDEO:
            mQueue[sType] = new ElementaryStreamQueue(ElementaryStreamQueue::MPEG_VIDEO);
            break;
        case STREAMTYPE_MPEG4_VIDEO:
            mQueue[sType] = new ElementaryStreamQueue(ElementaryStreamQueue::MPEG4_VIDEO);
            break;
        default:
            break;
    }
    LOGI("new stream type 0x%02x", streamType);
}

void AESParser::StreamDeInit(SourceType sType)
{
    LOGV("AESParser::StreamDeInit");
    delete mQueue[sType];
    mQueue[sType] = NULL;
    mStreamInitialized[sType] = false;
}

void AESParser::signalDiscontinuity(SourceType sType, DiscontinuityType type, const sp<AMessage> &extra)
{
    LOGV("AESParser::signalDiscontinuity");

    switch (type) {
        case DISCONTINUITY_SEEK:
        case DISCONTINUITY_FORMATCHANGE:
        {
            bool isASeek = (type == DISCONTINUITY_SEEK);

            mQueue[sType]->clear(!isASeek);

            if (mSource[sType] != NULL) {
                mSource[sType]->queueDiscontinuity((ATSParser::DiscontinuityType)type, extra);
            } else {
                deferDiscontinuity(sType, type, extra);
            }
            break;
        }

        default:
            TRESPASS();
            break;
    }
}

void AESParser::deferDiscontinuity(SourceType sType, DiscontinuityType type, const sp<AMessage> &extra)
{
    LOGV("AESParser::deferDiscontinuity");
    if (type > mPendingDiscontinuity[sType]) {
        // Only upgrade discontinuities.
        mPendingDiscontinuity[sType] = type;
        mPendingDiscontinuityExtra[sType] = extra;
    }
}

void AESParser::signalEOS(SourceType sType, status_t finalResult)
{
    LOGV("AESParser::signalEOS");
    if (mSource[sType] != NULL) {
        mSource[sType]->signalEOS(finalResult);
    }
}

int64_t AESParser::convertPTSToTimestamp(SourceType sType, int64_t PTS)
{
    if (PTS == 0)
    {
        // Skip if this is the first time
    }
    else if (!mFirstPTSValid[sType])
    {
        mFirstPTSValid[sType] = true;
        mFirstPTS[sType] = PTS;
        PTS = 0;
    }
    else if (PTS < mFirstPTS[sType])
    {
        PTS = 0;
    }
    else
    {
        PTS -= mFirstPTS[sType];
    }

    //return (PTS * 100) / 9; // Note: Netflix already does this
    return PTS;
}

void AESParser::onPayloadData(SourceType sType, const uint8_t *data, size_t size, int64_t timeStamp)
{
    //LOGV("AESParser::onPayloadData mStreamType=0x%02x", mStreamType[type]);

    if (mQueue[sType] == NULL)
    {
        LOGE("AESParser::onPayloadData ERROR: mQueue is NULL!");
        return;
    }

    int64_t timeUs = convertPTSToTimestamp(sType, timeStamp);
    //LOGI("TS(%s): %llu %llu", (sType == VIDEO) ? "V" : "A", timeStamp, timeUs);

    status_t err = mQueue[sType]->appendData(data, size, timeUs);
    if (err != OK)
    {
        LOGE("AESParser::onPayloadData ERROR: appendData failed!");
        return;
    }

    sp<ABuffer> accessUnit;
    while ((accessUnit = mQueue[sType]->dequeueAccessUnit()) != NULL)
    {
        if (mSource[sType] == NULL)
        {
            sp<MetaData> meta = mQueue[sType]->getFormat();

            if (meta != NULL)
            {
                LOGV("Stream of type 0x%02x now has data.", mStreamType[sType]);

                mSource[sType] = new AnotherPacketSource(meta);

                if (mPendingDiscontinuity[sType] != DISCONTINUITY_NONE)
                {
                    LOGE("AESParser::onPayloadData call mSource->queueDiscontinuity");
                    mSource[sType]->queueDiscontinuity((ATSParser::DiscontinuityType)mPendingDiscontinuity[sType],
                                                                                  mPendingDiscontinuityExtra[sType]);
                    mPendingDiscontinuity[sType] = DISCONTINUITY_NONE;
                    mPendingDiscontinuityExtra[sType].clear();
                }

                mSource[sType]->queueAccessUnit(accessUnit);
            }
        } else if (mQueue[sType]->getFormat() != NULL)
        {
            // After a discontinuity we invalidate the queue's format
            // and won't enqueue any access units to the source until
            // the queue has reestablished the new format.

            if (mSource[sType]->getFormat() == NULL)
            {
                mSource[sType]->setFormat(mQueue[sType]->getFormat());
            }
            mSource[sType]->queueAccessUnit(accessUnit);
        }
    }
}

sp<MediaSource> AESParser::getSource(SourceType sType)
{
    switch (sType) {
        case VIDEO:
        {
            if (isVideo(mStreamType[sType]))
            {
                return mSource[sType];
            }
            break;
        }

        case AUDIO:
        {
            if (isAudio(mStreamType[sType]))
            {
                return mSource[sType];
            }
            break;
        }

        default:
            break;
    }

    return NULL;
}

AESParser::AESParser()
{
    LOGI("AESParser CONSTRUCTOR");
    mStreamInitialized[VIDEO] = false;
    mStreamInitialized[AUDIO] = false;
    mFirstPTSValid[VIDEO] = false;
    mFirstPTSValid[AUDIO] = false;
}

AESParser::~AESParser()
{
    LOGI("AESParser DESTRUCTOR");

    if (mStreamInitialized[VIDEO])
        StreamDeInit(VIDEO);
    if (mStreamInitialized[AUDIO])
        StreamDeInit(AUDIO);
}

void AESParser::feedESPacket(StreamType sType, const void *data, size_t size, int64_t timeStamp)
{
    //const uint8_t * d = (const uint8_t *)data;
    //LOGI("AESParser::feedESPacket Data: %02x %02x %02x %02x %02x %02x %02x %02x",
    //     d[0],d[1],d[2],d[3],d[4],d[5],d[6],d[7]);

    SourceType source;

    if (isVideo(sType))
    {
        source = VIDEO;
        if (!mStreamInitialized[source])
            StreamInit(source, sType);
    }
    else if (isAudio(sType))
    {
        source = AUDIO;
        if (!mStreamInitialized[source])
            StreamInit(source, sType);
    }
    else
    {
        LOGI("AESParser::feedESPacket Invalid StreamType passed into AESParser (sType: %d)", sType);
        return;
    }

    onPayloadData(source, (const uint8_t *)data, size, timeStamp);
    return;
}

}  // namespace android

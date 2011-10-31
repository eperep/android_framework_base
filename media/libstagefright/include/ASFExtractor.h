/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef ASF_EXTRACTOR_H_

#define ASF_EXTRACTOR_H_

#include <utils/Errors.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/NvParserHalDefines.h>
#include <media/stagefright/openmax/OMX_Core.h>
#include <NVOMX_IndexExtensions.h>
#include <utils/List.h>
#include <utils/threads.h>

#define COMMON_MAX_INPUT_BUFFER_SIZE       64 * 1024
#define MAX_INPUT_STREAMS 2

static uint8_t ASF_HEADER_GUID[16] =
    { 0x30, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C };

namespace android {

struct AMessage;
class DataSource;
class String8;

typedef struct
{
    uint32_t mTracksStarted;
    Mutex mMutex;
}ASFExtractorData;

class ASFExtractor : public MediaExtractor {
public:
    // Extractor assumes ownership of "source".
    ASFExtractor(const sp<DataSource> &source);
    virtual size_t countTracks();
    virtual sp<MediaSource> getTrack(size_t index);
    virtual sp<MetaData> getTrackMetaData(size_t index, uint32_t flags);
    virtual sp<MetaData> getMetaData();
    virtual bool initParser();

protected:
    virtual ~ASFExtractor();

private:
    enum {
        IS_INITIALIZED              = 1,
        HAS_TRACK_COUNT             = 2,
        HAS_FILE_METADATA           = 4,
        HAS_SHARED_HAL              = 8
    };

    struct Track {
        sp<MetaData> meta;
        uint32_t timescale;
        bool includes_expensive_metadata;
        bool has_meta_data;
    };

    Track mTracks[MAX_INPUT_STREAMS];
    sp<MetaData> mTrackMeta;
    sp<DataSource> mDataSource;
    ASFExtractorData *Extractor;
    bool mHaveMetadata;
    uint32_t mTrackCount;
    uint32_t mDuration;
    uint32_t mFlags;
    void* NvAsfParserHalLibHandle;
    void* NvAsfParserHalHandle;
    sp<MetaData> mFileMetaData;

    NvParseHalImplementation nvAsfParseHalImpl;
    ASFExtractor(const ASFExtractor &);
    ASFExtractor &operator=(const ASFExtractor &);
};

bool SniffASF(
        const sp<DataSource> &source, String8 *mimeType, float *confidence,
        sp<AMessage> *);

}  // namespace android

#endif  // ASF_EXTRACTOR_H_


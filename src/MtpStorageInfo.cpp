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

#define LOG_TAG "MtpStorageInfo"

#include <iomanip>
#include <cstring>

#include "MtpDebug.h"
#include "MtpDataPacket.h"
#include "MtpStorageInfo.h"
#include "MtpStringBuffer.h"

namespace android {

MtpStorageInfo::MtpStorageInfo(MtpStorageID id)
    :   mStorageID(id),
        mStorageType(0),
        mFileSystemType(0),
        mAccessCapability(0),
        mMaxCapacity(0),
        mFreeSpaceBytes(0),
        mFreeSpaceObjects(0),
        mStorageDescription(NULL),
        mVolumeIdentifier(NULL)
{
}

MtpStorageInfo::~MtpStorageInfo() {
    if (mStorageDescription)
        free(mStorageDescription);
    if (mVolumeIdentifier)
        free(mVolumeIdentifier);
}

void MtpStorageInfo::read(MtpDataPacket& packet) {
    MtpStringBuffer string;

    // read the device info
    mStorageType = packet.getUInt16();
    mFileSystemType = packet.getUInt16();
    mAccessCapability = packet.getUInt16();
    mMaxCapacity = packet.getUInt64();
    mFreeSpaceBytes = packet.getUInt64();
    mFreeSpaceObjects = packet.getUInt32();

    packet.getString(string);
    mStorageDescription = strdup((const char *)string);
    packet.getString(string);
    mVolumeIdentifier = strdup((const char *)string);
}

void MtpStorageInfo::print() {
    VLOG(2) << "Storage Info " << std::hex << mStorageID << std::dec << ":"
            << "\n\tmStorageType: " << mStorageType
            << "\n\tmFileSystemType: " << mFileSystemType
            << "\n\tmAccessCapability: " << mAccessCapability;
    VLOG(2) << "\tmMaxCapacity: " << mMaxCapacity
            << "\n\tmFreeSpaceBytes: " << mFreeSpaceBytes
            << "\n\tmFreeSpaceObjects: " << mFreeSpaceObjects;
    VLOG(2) << "\tmStorageDescription: " << mStorageDescription
            << "\n\tmVolumeIdentifier: " << mVolumeIdentifier;
}

}  // namespace android

// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/CanFdMessage64.h>

namespace Vector {
namespace BLF {

CanFdMessage64::CanFdMessage64() :
    ObjectHeader(ObjectType::CAN_FD_MESSAGE_64),
    CanFdExtFrameData() {
}

void CanFdMessage64::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    is.read(reinterpret_cast<char *>(&validDataBytes), sizeof(validDataBytes));
    is.read(reinterpret_cast<char *>(&txCount), sizeof(txCount));
    is.read(reinterpret_cast<char *>(&id), sizeof(id));
    is.read(reinterpret_cast<char *>(&frameLength), sizeof(frameLength));
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&btrCfgArb), sizeof(btrCfgArb));
    is.read(reinterpret_cast<char *>(&btrCfgData), sizeof(btrCfgData));
    is.read(reinterpret_cast<char *>(&timeOffsetBrsNs), sizeof(timeOffsetBrsNs));
    is.read(reinterpret_cast<char *>(&timeOffsetCrcDelNs), sizeof(timeOffsetCrcDelNs));
    is.read(reinterpret_cast<char *>(&bitCount), sizeof(bitCount));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&extDataOffset), sizeof(extDataOffset));
    is.read(reinterpret_cast<char *>(&crc), sizeof(crc));
    data.resize(validDataBytes);
    is.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    if (hasExtData())
        CanFdExtFrameData::read(is);
    // @note reservedCanFdExtFrameData is read here as CanFdExtFrameData doesn't know the objectSize
    reservedCanFdExtFrameData.resize(objectSize - calculateObjectSize());
    is.read(reinterpret_cast<char *>(reservedCanFdExtFrameData.data()), static_cast<std::streamsize>(reservedCanFdExtFrameData.size()));
}

void CanFdMessage64::write(AbstractFile & os) {
    /* pre processing */
    validDataBytes = static_cast<uint8_t>(data.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    os.write(reinterpret_cast<char *>(&validDataBytes), sizeof(validDataBytes));
    os.write(reinterpret_cast<char *>(&txCount), sizeof(txCount));
    os.write(reinterpret_cast<char *>(&id), sizeof(id));
    os.write(reinterpret_cast<char *>(&frameLength), sizeof(frameLength));
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&btrCfgArb), sizeof(btrCfgArb));
    os.write(reinterpret_cast<char *>(&btrCfgData), sizeof(btrCfgData));
    os.write(reinterpret_cast<char *>(&timeOffsetBrsNs), sizeof(timeOffsetBrsNs));
    os.write(reinterpret_cast<char *>(&timeOffsetCrcDelNs), sizeof(timeOffsetCrcDelNs));
    os.write(reinterpret_cast<char *>(&bitCount), sizeof(bitCount));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&extDataOffset), sizeof(extDataOffset));
    os.write(reinterpret_cast<char *>(&crc), sizeof(crc));
    os.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    if (hasExtData())
        CanFdExtFrameData::write(os);
}

bool CanFdMessage64::hasExtData() const {
    return
        (extDataOffset != 0) &&
        (objectSize >= extDataOffset + CanFdExtFrameData::calculateObjectSize());
}

uint32_t CanFdMessage64::calculateObjectSize() const {
    uint32_t size =
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(dlc) +
        sizeof(validDataBytes) +
        sizeof(txCount) +
        sizeof(id) +
        sizeof(frameLength) +
        sizeof(flags) +
        sizeof(btrCfgArb) +
        sizeof(btrCfgData) +
        sizeof(timeOffsetBrsNs) +
        sizeof(timeOffsetCrcDelNs) +
        sizeof(bitCount) +
        sizeof(dir) +
        sizeof(extDataOffset) +
        sizeof(crc) +
        static_cast<uint32_t>(data.size());
    if (hasExtData())
        size += CanFdExtFrameData::calculateObjectSize();
    return size;
}

}
}

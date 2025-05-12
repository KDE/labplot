// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/CanFdErrorFrame64.h>

namespace Vector {
namespace BLF {

CanFdErrorFrame64::CanFdErrorFrame64() :
    ObjectHeader(ObjectType::CAN_FD_ERROR_64) {
}

void CanFdErrorFrame64::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    is.read(reinterpret_cast<char *>(&validDataBytes), sizeof(validDataBytes));
    is.read(reinterpret_cast<char *>(&ecc), sizeof(ecc));
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&errorCodeExt), sizeof(errorCodeExt));
    is.read(reinterpret_cast<char *>(&extFlags), sizeof(extFlags));
    is.read(reinterpret_cast<char *>(&extDataOffset), sizeof(extDataOffset));
    is.read(reinterpret_cast<char *>(&reservedCanFdErrorFrame1), sizeof(reservedCanFdErrorFrame1));
    is.read(reinterpret_cast<char *>(&id), sizeof(id));
    is.read(reinterpret_cast<char *>(&frameLength), sizeof(frameLength));
    is.read(reinterpret_cast<char *>(&btrCfgArb), sizeof(btrCfgArb));
    is.read(reinterpret_cast<char *>(&btrCfgData), sizeof(btrCfgData));
    is.read(reinterpret_cast<char *>(&timeOffsetBrsNs), sizeof(timeOffsetBrsNs));
    is.read(reinterpret_cast<char *>(&timeOffsetCrcDelNs), sizeof(timeOffsetCrcDelNs));
    is.read(reinterpret_cast<char *>(&crc), sizeof(crc));
    is.read(reinterpret_cast<char *>(&errorPosition), sizeof(errorPosition));
    is.read(reinterpret_cast<char *>(&reservedCanFdErrorFrame2), sizeof(reservedCanFdErrorFrame2));
    data.resize(validDataBytes);
    is.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    if (hasExtData())
        CanFdExtFrameData::read(is);
    // @note reservedCanFdExtFrameData is read here as CanFdExtFrameData doesn't know the objectSize
    reservedCanFdExtFrameData.resize(objectSize - calculateObjectSize());
    is.read(reinterpret_cast<char *>(reservedCanFdExtFrameData.data()), static_cast<std::streamsize>(reservedCanFdExtFrameData.size()));
}

void CanFdErrorFrame64::write(AbstractFile & os) {
    /* pre processing */
    validDataBytes = static_cast<uint8_t>(data.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    os.write(reinterpret_cast<char *>(&validDataBytes), sizeof(validDataBytes));
    os.write(reinterpret_cast<char *>(&ecc), sizeof(ecc));
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&errorCodeExt), sizeof(errorCodeExt));
    os.write(reinterpret_cast<char *>(&extFlags), sizeof(extFlags));
    os.write(reinterpret_cast<char *>(&extDataOffset), sizeof(extDataOffset));
    os.write(reinterpret_cast<char *>(&reservedCanFdErrorFrame1), sizeof(reservedCanFdErrorFrame1));
    os.write(reinterpret_cast<char *>(&id), sizeof(id));
    os.write(reinterpret_cast<char *>(&frameLength), sizeof(frameLength));
    os.write(reinterpret_cast<char *>(&btrCfgArb), sizeof(btrCfgArb));
    os.write(reinterpret_cast<char *>(&btrCfgData), sizeof(btrCfgData));
    os.write(reinterpret_cast<char *>(&timeOffsetBrsNs), sizeof(timeOffsetBrsNs));
    os.write(reinterpret_cast<char *>(&timeOffsetCrcDelNs), sizeof(timeOffsetCrcDelNs));
    os.write(reinterpret_cast<char *>(&crc), sizeof(crc));
    os.write(reinterpret_cast<char *>(&errorPosition), sizeof(errorPosition));
    os.write(reinterpret_cast<char *>(&reservedCanFdErrorFrame2), sizeof(reservedCanFdErrorFrame2));
    os.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    if (hasExtData())
        CanFdExtFrameData::write(os);
}

bool CanFdErrorFrame64::hasExtData() const {
    return
        (extDataOffset != 0) &&
        (objectSize >= extDataOffset + CanFdExtFrameData::calculateObjectSize());
}

uint32_t CanFdErrorFrame64::calculateObjectSize() const {
    uint32_t size =
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(dlc) +
        sizeof(validDataBytes) +
        sizeof(ecc) +
        sizeof(flags) +
        sizeof(errorCodeExt) +
        sizeof(extFlags) +
        sizeof(extDataOffset) +
        sizeof(reservedCanFdErrorFrame1) +
        sizeof(id) +
        sizeof(frameLength) +
        sizeof(btrCfgArb) +
        sizeof(btrCfgData) +
        sizeof(timeOffsetBrsNs) +
        sizeof(timeOffsetCrcDelNs) +
        sizeof(crc) +
        sizeof(errorPosition) +
        sizeof(reservedCanFdErrorFrame2) +
        static_cast<uint32_t>(data.size());
    if (hasExtData())
        size += CanFdExtFrameData::calculateObjectSize();
    return size;
}

}
}

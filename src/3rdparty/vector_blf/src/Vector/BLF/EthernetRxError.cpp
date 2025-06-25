// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/EthernetRxError.h>

namespace Vector {
namespace BLF {

EthernetRxError::EthernetRxError() :
    ObjectHeader(ObjectType::ETHERNET_RX_ERROR) {
}

void EthernetRxError::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&structLength), sizeof(structLength));
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&hardwareChannel), sizeof(hardwareChannel));
    is.read(reinterpret_cast<char *>(&fcs), sizeof(fcs));
    is.read(reinterpret_cast<char *>(&frameDataLength), sizeof(frameDataLength));
    is.read(reinterpret_cast<char *>(&reservedEthernetRxError), sizeof(reservedEthernetRxError));
    is.read(reinterpret_cast<char *>(&error), sizeof(error));
    frameData.resize(frameDataLength);
    is.read(reinterpret_cast<char *>(frameData.data()), frameDataLength);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void EthernetRxError::write(AbstractFile & os) {
    /* pre processing */
    structLength = calculateStructLength();
    frameDataLength = static_cast<uint16_t>(frameData.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&structLength), sizeof(structLength));
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&hardwareChannel), sizeof(hardwareChannel));
    os.write(reinterpret_cast<char *>(&fcs), sizeof(fcs));
    os.write(reinterpret_cast<char *>(&frameDataLength), sizeof(frameDataLength));
    os.write(reinterpret_cast<char *>(&reservedEthernetRxError), sizeof(reservedEthernetRxError));
    os.write(reinterpret_cast<char *>(&error), sizeof(error));
    os.write(reinterpret_cast<char *>(frameData.data()), frameDataLength);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t EthernetRxError::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(structLength) +
        sizeof(channel) +
        sizeof(dir) +
        sizeof(hardwareChannel) +
        sizeof(fcs) +
        sizeof(frameDataLength) +
        sizeof(reservedEthernetRxError) +
        sizeof(error) +
        frameDataLength;
}

uint16_t EthernetRxError::calculateStructLength() const {
    return
        sizeof(structLength) +
        sizeof(channel) +
        sizeof(dir) +
        sizeof(hardwareChannel) +
        sizeof(fcs) +
        sizeof(frameDataLength) +
        sizeof(reservedEthernetRxError) +
        sizeof(error);
}

}
}

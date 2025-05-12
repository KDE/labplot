// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/AfdxFrame.h>

namespace Vector {
namespace BLF {

AfdxFrame::AfdxFrame() :
    ObjectHeader(ObjectType::AFDX_FRAME) {
}

void AfdxFrame::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(sourceAddress.data()), static_cast<std::streamsize>(sourceAddress.size()));
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(destinationAddress.data()), static_cast<std::streamsize>(destinationAddress.size()));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&type), sizeof(type));
    is.read(reinterpret_cast<char *>(&tpid), sizeof(tpid));
    is.read(reinterpret_cast<char *>(&tci), sizeof(tci));
    is.read(reinterpret_cast<char *>(&ethChannel), sizeof(ethChannel));
    is.read(reinterpret_cast<char *>(&reservedAfdxFrame1), sizeof(reservedAfdxFrame1));
    is.read(reinterpret_cast<char *>(&afdxFlags), sizeof(afdxFlags));
    is.read(reinterpret_cast<char *>(&reservedAfdxFrame2), sizeof(reservedAfdxFrame2));
    is.read(reinterpret_cast<char *>(&bagUsec), sizeof(bagUsec));
    is.read(reinterpret_cast<char *>(&payLoadLength), sizeof(payLoadLength));
    is.read(reinterpret_cast<char *>(&reservedAfdxFrame3), sizeof(reservedAfdxFrame3));
    is.read(reinterpret_cast<char *>(&reservedAfdxFrame4), sizeof(reservedAfdxFrame4));
    payLoad.resize(payLoadLength);
    is.read(reinterpret_cast<char *>(payLoad.data()), payLoadLength);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void AfdxFrame::write(AbstractFile & os) {
    /* pre processing */
    payLoadLength = static_cast<uint16_t>(payLoad.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(sourceAddress.data()), static_cast<std::streamsize>(sourceAddress.size()));
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(destinationAddress.data()), static_cast<std::streamsize>(destinationAddress.size()));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&type), sizeof(type));
    os.write(reinterpret_cast<char *>(&tpid), sizeof(tpid));
    os.write(reinterpret_cast<char *>(&tci), sizeof(tci));
    os.write(reinterpret_cast<char *>(&ethChannel), sizeof(ethChannel));
    os.write(reinterpret_cast<char *>(&reservedAfdxFrame1), sizeof(reservedAfdxFrame1));
    os.write(reinterpret_cast<char *>(&afdxFlags), sizeof(afdxFlags));
    os.write(reinterpret_cast<char *>(&reservedAfdxFrame2), sizeof(reservedAfdxFrame2));
    os.write(reinterpret_cast<char *>(&bagUsec), sizeof(bagUsec));
    os.write(reinterpret_cast<char *>(&payLoadLength), sizeof(payLoadLength));
    os.write(reinterpret_cast<char *>(&reservedAfdxFrame3), sizeof(reservedAfdxFrame3));
    os.write(reinterpret_cast<char *>(&reservedAfdxFrame4), sizeof(reservedAfdxFrame4));
    os.write(reinterpret_cast<char *>(payLoad.data()), payLoadLength);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t AfdxFrame::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        static_cast<uint32_t>(sourceAddress.size()) +
        sizeof(channel) +
        static_cast<uint32_t>(destinationAddress.size()) +
        sizeof(dir) +
        sizeof(type) +
        sizeof(tpid) +
        sizeof(tci) +
        sizeof(ethChannel) +
        sizeof(reservedAfdxFrame1) +
        sizeof(afdxFlags) +
        sizeof(reservedAfdxFrame2) +
        sizeof(bagUsec) +
        sizeof(payLoadLength) +
        sizeof(reservedAfdxFrame3) +
        sizeof(reservedAfdxFrame4) +
        payLoadLength;
}

}
}

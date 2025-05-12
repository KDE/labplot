// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinSendError2.h>

namespace Vector {
namespace BLF {

LinSendError2::LinSendError2() :
    ObjectHeader(ObjectType::LIN_SND_ERROR2, 1) {
}

void LinSendError2::read(AbstractFile & is) {
    ObjectHeader::read(is);
    LinMessageDescriptor::read(is);
    is.read(reinterpret_cast<char *>(&eoh), sizeof(eoh));
    is.read(reinterpret_cast<char *>(&isEtf), sizeof(isEtf));
    is.read(reinterpret_cast<char *>(&fsmId), sizeof(fsmId));
    is.read(reinterpret_cast<char *>(&fsmState), sizeof(fsmState));
    is.read(reinterpret_cast<char *>(&reservedLinSendError1), sizeof(reservedLinSendError1));
    is.read(reinterpret_cast<char *>(&reservedLinSendError2), sizeof(reservedLinSendError2));
    is.read(reinterpret_cast<char *>(&exactHeaderBaudrate), sizeof(exactHeaderBaudrate));
    is.read(reinterpret_cast<char *>(&earlyStopbitOffset), sizeof(earlyStopbitOffset));

    reservedLinSendError3_present = false;
    if (this->objectSize >= calculateObjectSize() + sizeof(reservedLinSendError3)) {
        is.read(reinterpret_cast<char*>(&reservedLinSendError3), sizeof(reservedLinSendError3));
        reservedLinSendError3_present = true;
	// @note might be extended in future versions
    }
}

void LinSendError2::write(AbstractFile & os) {
    ObjectHeader::write(os);
    LinMessageDescriptor::write(os);
    os.write(reinterpret_cast<char *>(&eoh), sizeof(eoh));
    os.write(reinterpret_cast<char *>(&isEtf), sizeof(isEtf));
    os.write(reinterpret_cast<char *>(&fsmId), sizeof(fsmId));
    os.write(reinterpret_cast<char *>(&fsmState), sizeof(fsmState));
    os.write(reinterpret_cast<char *>(&reservedLinSendError1), sizeof(reservedLinSendError1));
    os.write(reinterpret_cast<char *>(&reservedLinSendError2), sizeof(reservedLinSendError2));
    os.write(reinterpret_cast<char *>(&exactHeaderBaudrate), sizeof(exactHeaderBaudrate));
    os.write(reinterpret_cast<char *>(&earlyStopbitOffset), sizeof(earlyStopbitOffset));
    os.write(reinterpret_cast<char *>(&reservedLinSendError3), sizeof(reservedLinSendError3));
}

uint32_t LinSendError2::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        LinMessageDescriptor::calculateObjectSize() +
        sizeof(eoh) +
        sizeof(isEtf) +
        sizeof(fsmId) +
        sizeof(fsmState) +
        sizeof(reservedLinSendError1) +
        sizeof(reservedLinSendError2) +
        sizeof(exactHeaderBaudrate) +
        sizeof(earlyStopbitOffset) +
        (reservedLinSendError3_present ? sizeof(reservedLinSendError3) : 0);
}

}
}

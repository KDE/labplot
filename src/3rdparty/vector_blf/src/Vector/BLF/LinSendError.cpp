// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinSendError.h>

namespace Vector {
namespace BLF {

LinSendError::LinSendError() :
    ObjectHeader(ObjectType::LIN_SND_ERROR) {
}

void LinSendError::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&id), sizeof(id));
    is.read(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    is.read(reinterpret_cast<char *>(&fsmId), sizeof(fsmId));
    is.read(reinterpret_cast<char *>(&fsmState), sizeof(fsmState));
    is.read(reinterpret_cast<char *>(&headerTime), sizeof(headerTime));
    is.read(reinterpret_cast<char *>(&fullTime), sizeof(fullTime));
}

void LinSendError::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&id), sizeof(id));
    os.write(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    os.write(reinterpret_cast<char *>(&fsmId), sizeof(fsmId));
    os.write(reinterpret_cast<char *>(&fsmState), sizeof(fsmState));
    os.write(reinterpret_cast<char *>(&headerTime), sizeof(headerTime));
    os.write(reinterpret_cast<char *>(&fullTime), sizeof(fullTime));
}

uint32_t LinSendError::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(id) +
        sizeof(dlc) +
        sizeof(fsmId) +
        sizeof(fsmState) +
        sizeof(headerTime) +
        sizeof(fullTime);
}

}
}

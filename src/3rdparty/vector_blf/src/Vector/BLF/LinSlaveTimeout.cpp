// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinSlaveTimeout.h>

namespace Vector {
namespace BLF {

LinSlaveTimeout::LinSlaveTimeout() :
    ObjectHeader(ObjectType::LIN_SLV_TIMEOUT) {
}

void LinSlaveTimeout::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&slaveId), sizeof(slaveId));
    is.read(reinterpret_cast<char *>(&stateId), sizeof(stateId));
    is.read(reinterpret_cast<char *>(&followStateId), sizeof(followStateId));
}

void LinSlaveTimeout::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&slaveId), sizeof(slaveId));
    os.write(reinterpret_cast<char *>(&stateId), sizeof(stateId));
    os.write(reinterpret_cast<char *>(&followStateId), sizeof(followStateId));
}

uint32_t LinSlaveTimeout::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(slaveId) +
        sizeof(stateId) +
        sizeof(followStateId);
}

}
}

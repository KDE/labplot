// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostNetState.h>

namespace Vector {
namespace BLF {

MostNetState::MostNetState() :
    ObjectHeader2(ObjectType::MOST_NETSTATE) {
}

void MostNetState::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&stateNew), sizeof(stateNew));
    is.read(reinterpret_cast<char *>(&stateOld), sizeof(stateOld));
    is.read(reinterpret_cast<char *>(&reservedMostNetState), sizeof(reservedMostNetState));
}

void MostNetState::write(AbstractFile & os) {
    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&stateNew), sizeof(stateNew));
    os.write(reinterpret_cast<char *>(&stateOld), sizeof(stateOld));
    os.write(reinterpret_cast<char *>(&reservedMostNetState), sizeof(reservedMostNetState));
}

uint32_t MostNetState::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(stateNew) +
        sizeof(stateOld) +
        sizeof(reservedMostNetState);
}

}
}

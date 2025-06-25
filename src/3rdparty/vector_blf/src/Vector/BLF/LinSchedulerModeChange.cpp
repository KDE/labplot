// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinSchedulerModeChange.h>

namespace Vector {
namespace BLF {

LinSchedulerModeChange::LinSchedulerModeChange() :
    ObjectHeader(ObjectType::LIN_SCHED_MODCH) {
}

void LinSchedulerModeChange::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&oldMode), sizeof(oldMode));
    is.read(reinterpret_cast<char *>(&newMode), sizeof(newMode));
    is.read(reinterpret_cast<char *>(&reservedLinSchedulerModeChange), sizeof(reservedLinSchedulerModeChange));
}

void LinSchedulerModeChange::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&oldMode), sizeof(oldMode));
    os.write(reinterpret_cast<char *>(&newMode), sizeof(newMode));
    os.write(reinterpret_cast<char *>(&reservedLinSchedulerModeChange), sizeof(reservedLinSchedulerModeChange));
}

uint32_t LinSchedulerModeChange::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(oldMode) +
        sizeof(newMode) +
        sizeof(reservedLinSchedulerModeChange);

}

}
}

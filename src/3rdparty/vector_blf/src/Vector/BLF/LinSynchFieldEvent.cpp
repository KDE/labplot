// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinSynchFieldEvent.h>

namespace Vector {
namespace BLF {

void LinSynchFieldEvent::read(AbstractFile & is) {
    LinBusEvent::read(is);
    is.read(reinterpret_cast<char *>(&synchBreakLength), sizeof(synchBreakLength));
    is.read(reinterpret_cast<char *>(&synchDelLength), sizeof(synchDelLength));
}

void LinSynchFieldEvent::write(AbstractFile & os) {
    LinBusEvent::write(os);
    os.write(reinterpret_cast<char *>(&synchBreakLength), sizeof(synchBreakLength));
    os.write(reinterpret_cast<char *>(&synchDelLength), sizeof(synchDelLength));
}

uint32_t LinSynchFieldEvent::calculateObjectSize() const {
    return
        LinBusEvent::calculateObjectSize() +
        sizeof(synchBreakLength) +
        sizeof(synchDelLength);
}

}
}

// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/WaterMarkEvent.h>

namespace Vector {
namespace BLF {

WaterMarkEvent::WaterMarkEvent() :
    ObjectHeader(ObjectType::WATER_MARK_EVENT) {
}

void WaterMarkEvent::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&queueState), sizeof(queueState));
    is.read(reinterpret_cast<char *>(&reservedWaterMarkEvent), sizeof(reservedWaterMarkEvent));
}

void WaterMarkEvent::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&queueState), sizeof(queueState));
    os.write(reinterpret_cast<char *>(&reservedWaterMarkEvent), sizeof(reservedWaterMarkEvent));
}

uint32_t WaterMarkEvent::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(queueState) +
        sizeof(reservedWaterMarkEvent);
}

}
}

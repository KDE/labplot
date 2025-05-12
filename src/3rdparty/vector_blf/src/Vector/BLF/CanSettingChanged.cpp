// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/CanSettingChanged.h>

namespace Vector {
namespace BLF {

CanSettingChanged::CanSettingChanged() :
    ObjectHeader(ObjectType::CAN_SETTING_CHANGED) {
}

void CanSettingChanged::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&changedType), sizeof(changedType));
    bitTimings.read(is);
}

void CanSettingChanged::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&changedType), sizeof(changedType));
    bitTimings.write(os);
}

uint32_t CanSettingChanged::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(changedType) +
        bitTimings.calculateObjectSize();
}

}
}

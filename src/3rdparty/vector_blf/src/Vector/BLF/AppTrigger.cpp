// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/AppTrigger.h>

namespace Vector {
namespace BLF {

AppTrigger::AppTrigger() :
    ObjectHeader(ObjectType::APP_TRIGGER) {
}

void AppTrigger::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&preTriggerTime), sizeof(preTriggerTime));
    is.read(reinterpret_cast<char *>(&postTriggerTime), sizeof(postTriggerTime));
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&appSpecific2), sizeof(appSpecific2));
}

void AppTrigger::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&preTriggerTime), sizeof(preTriggerTime));
    os.write(reinterpret_cast<char *>(&postTriggerTime), sizeof(postTriggerTime));
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&appSpecific2), sizeof(appSpecific2));
}

uint32_t AppTrigger::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(preTriggerTime) +
        sizeof(postTriggerTime) +
        sizeof(channel) +
        sizeof(flags) +
        sizeof(appSpecific2);
}

}
}

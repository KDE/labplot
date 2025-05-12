// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/TriggerCondition.h>

namespace Vector {
namespace BLF {

TriggerCondition::TriggerCondition() :
    ObjectHeader(ObjectType::TRIGGER_CONDITION) {
}

void TriggerCondition::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&state), sizeof(state));
    is.read(reinterpret_cast<char *>(&triggerBlockNameLength), sizeof(triggerBlockNameLength));
    is.read(reinterpret_cast<char *>(&triggerConditionLength), sizeof(triggerConditionLength));
    triggerBlockName.resize(triggerBlockNameLength);
    is.read(const_cast<char *>(triggerBlockName.data()), triggerBlockNameLength);
    triggerCondition.resize(triggerConditionLength);
    is.read(const_cast<char *>(triggerCondition.data()), triggerConditionLength);
}

void TriggerCondition::write(AbstractFile & os) {
    /* pre processing */
    triggerBlockNameLength = static_cast<uint32_t>(triggerBlockName.size());
    triggerConditionLength = static_cast<uint32_t>(triggerCondition.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&state), sizeof(state));
    os.write(reinterpret_cast<char *>(&triggerBlockNameLength), sizeof(triggerBlockNameLength));
    os.write(reinterpret_cast<char *>(&triggerConditionLength), sizeof(triggerConditionLength));
    os.write(const_cast<char *>(triggerBlockName.data()), triggerBlockNameLength);
    os.write(const_cast<char *>(triggerCondition.data()), triggerConditionLength);
}

uint32_t TriggerCondition::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(state) +
        sizeof(triggerBlockNameLength) +
        sizeof(triggerConditionLength) +
        triggerBlockNameLength +
        triggerConditionLength;
}

}
}

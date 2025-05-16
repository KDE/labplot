// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/Most150AllocTab.h>

namespace Vector {
namespace BLF {

Most150AllocTab::Most150AllocTab() :
    ObjectHeader2(ObjectType::MOST_150_ALLOCTAB) {
}

void Most150AllocTab::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&eventModeFlags), sizeof(eventModeFlags));
    is.read(reinterpret_cast<char *>(&freeBytes), sizeof(freeBytes));
    is.read(reinterpret_cast<char *>(&length), sizeof(length));
    is.read(reinterpret_cast<char *>(&reservedMost150AllocTab), sizeof(reservedMost150AllocTab));
    tableData.resize(length);
    is.read(reinterpret_cast<char *>(tableData.data()), length);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void Most150AllocTab::write(AbstractFile & os) {
    /* pre processing */
    length = static_cast<uint16_t>(tableData.size());

    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&eventModeFlags), sizeof(eventModeFlags));
    os.write(reinterpret_cast<char *>(&freeBytes), sizeof(freeBytes));
    os.write(reinterpret_cast<char *>(&length), sizeof(length));
    os.write(reinterpret_cast<char *>(&reservedMost150AllocTab), sizeof(reservedMost150AllocTab));
    os.write(reinterpret_cast<char *>(tableData.data()), length);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t Most150AllocTab::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(eventModeFlags) +
        sizeof(freeBytes) +
        sizeof(length) +
        sizeof(reservedMost150AllocTab) +
        length;
}

}
}

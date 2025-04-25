// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostAllocTab.h>

namespace Vector {
namespace BLF {

MostAllocTab::MostAllocTab() :
    ObjectHeader2(ObjectType::MOST_ALLOCTAB) {
}

void MostAllocTab::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&length), sizeof(length));
    is.read(reinterpret_cast<char *>(&reservedMostAllocTab), sizeof(reservedMostAllocTab));
    tableData.resize(length);
    is.read(reinterpret_cast<char *>(tableData.data()), length);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void MostAllocTab::write(AbstractFile & os) {
    /* pre processing */
    length = static_cast<uint16_t>(tableData.size());

    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&length), sizeof(length));
    os.write(reinterpret_cast<char *>(&reservedMostAllocTab), sizeof(reservedMostAllocTab));
    os.write(reinterpret_cast<char *>(tableData.data()), length);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t MostAllocTab::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(length) +
        sizeof(reservedMostAllocTab) +
        length;
}

}
}

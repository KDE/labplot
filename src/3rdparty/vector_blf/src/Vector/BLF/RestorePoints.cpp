// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/RestorePoints.h>

#include <algorithm>

namespace Vector {
namespace BLF {

void RestorePoints::read(AbstractFile & is) {
    is.read(reinterpret_cast<char *>(&objectSize), sizeof(objectSize));
    is.read(reinterpret_cast<char *>(&objectInterval), sizeof(objectInterval));
    restorePoints.resize((objectSize - calculateObjectSize()) / RestorePoint::calculateObjectSize()); // all remaining data
    is.read(reinterpret_cast<char *>(restorePoints.data()), static_cast<std::streamsize>(restorePoints.size()) * RestorePoint::calculateObjectSize());
}

void RestorePoints::write(AbstractFile & os) {
    /* pre processing */
    objectSize = calculateObjectSize();

    os.write(reinterpret_cast<char *>(&objectSize), sizeof(objectSize));
    os.write(reinterpret_cast<char *>(&objectInterval), sizeof(objectInterval));
    os.write(reinterpret_cast<char *>(restorePoints.data()), static_cast<std::streamsize>(restorePoints.size()) * RestorePoint::calculateObjectSize());
}

uint32_t RestorePoints::calculateObjectSize() const {
    return
        sizeof(objectSize) +
        sizeof(objectInterval) +
        restorePoints.size() * RestorePoint::calculateObjectSize();
}

}
}

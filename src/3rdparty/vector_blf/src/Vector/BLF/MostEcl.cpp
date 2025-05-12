// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostEcl.h>

namespace Vector {
namespace BLF {

MostEcl::MostEcl() :
    ObjectHeader2(ObjectType::MOST_ECL) {
}

void MostEcl::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&mode), sizeof(mode));
    is.read(reinterpret_cast<char *>(&eclState), sizeof(eclState));
    is.read(reinterpret_cast<char *>(&reservedMostEcl), sizeof(reservedMostEcl));
    // @note might be extended in future versions
}

void MostEcl::write(AbstractFile & os) {
    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&mode), sizeof(mode));
    os.write(reinterpret_cast<char *>(&eclState), sizeof(eclState));
    os.write(reinterpret_cast<char *>(&reservedMostEcl), sizeof(reservedMostEcl));
}

uint32_t MostEcl::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(mode) +
        sizeof(eclState) +
        sizeof(reservedMostEcl);
}

}
}

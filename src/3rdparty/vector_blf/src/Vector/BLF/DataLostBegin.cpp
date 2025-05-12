// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/DataLostBegin.h>

namespace Vector {
namespace BLF {

DataLostBegin::DataLostBegin() :
    ObjectHeader(ObjectType::DATA_LOST_BEGIN) {
}

void DataLostBegin::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&queueIdentifier), sizeof(queueIdentifier));
}

void DataLostBegin::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&queueIdentifier), sizeof(queueIdentifier));
}

uint32_t DataLostBegin::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(queueIdentifier);
}

}
}

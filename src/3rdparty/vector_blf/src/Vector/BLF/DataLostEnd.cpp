// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/DataLostEnd.h>

namespace Vector {
namespace BLF {

DataLostEnd::DataLostEnd() :
    ObjectHeader(ObjectType::DATA_LOST_END) {
}

void DataLostEnd::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&queueIdentifier), sizeof(queueIdentifier));
    is.read(reinterpret_cast<char *>(&firstObjectLostTimeStamp), sizeof(firstObjectLostTimeStamp));
    is.read(reinterpret_cast<char *>(&numberOfLostEvents), sizeof(numberOfLostEvents));
}

void DataLostEnd::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&queueIdentifier), sizeof(queueIdentifier));
    os.write(reinterpret_cast<char *>(&firstObjectLostTimeStamp), sizeof(firstObjectLostTimeStamp));
    os.write(reinterpret_cast<char *>(&numberOfLostEvents), sizeof(numberOfLostEvents));
}

uint32_t DataLostEnd::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(queueIdentifier) +
        sizeof(firstObjectLostTimeStamp) +
        sizeof(numberOfLostEvents);
}

}
}

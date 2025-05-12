// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/DistributedObjectMember.h>

namespace Vector {
namespace BLF {

DistributedObjectMember::DistributedObjectMember() :
    ObjectHeader(ObjectType::DISTRIBUTED_OBJECT_MEMBER) {
}

void DistributedObjectMember::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&memberType), sizeof(memberType));
    is.read(reinterpret_cast<char *>(&detailType), sizeof(detailType));
    is.read(reinterpret_cast<char *>(&pathLength), sizeof(pathLength));
    is.read(reinterpret_cast<char *>(&dataLength), sizeof(dataLength));
    path.resize(pathLength);
    is.read(const_cast<char *>(path.data()), pathLength);
    data.resize(dataLength);
    is.read(reinterpret_cast<char *>(data.data()), dataLength);
}

void DistributedObjectMember::write(AbstractFile & os) {
    /* pre processing */
    pathLength = static_cast<uint32_t>(path.size());
    dataLength = static_cast<uint32_t>(data.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&memberType), sizeof(memberType));
    os.write(reinterpret_cast<char *>(&detailType), sizeof(detailType));
    os.write(reinterpret_cast<char *>(&pathLength), sizeof(pathLength));
    os.write(reinterpret_cast<char *>(&dataLength), sizeof(dataLength));
    os.write(const_cast<char *>(path.data()), pathLength);
    os.write(reinterpret_cast<char *>(data.data()), dataLength);
}

uint32_t DistributedObjectMember::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(memberType) +
        sizeof(detailType) +
        sizeof(pathLength) +
        sizeof(dataLength) +
        pathLength +
        dataLength;
}

}
}

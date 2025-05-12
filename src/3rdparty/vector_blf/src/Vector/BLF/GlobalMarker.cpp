// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstring>

#include <Vector/BLF/GlobalMarker.h>

namespace Vector {
namespace BLF {

GlobalMarker::GlobalMarker() :
    ObjectHeader(ObjectType::GLOBAL_MARKER) {
}

void GlobalMarker::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&commentedEventType), sizeof(commentedEventType));
    is.read(reinterpret_cast<char *>(&foregroundColor), sizeof(foregroundColor));
    is.read(reinterpret_cast<char *>(&backgroundColor), sizeof(backgroundColor));
    is.read(reinterpret_cast<char *>(&isRelocatable), sizeof(isRelocatable));
    is.read(reinterpret_cast<char *>(&reservedGlobalMarker1), sizeof(reservedGlobalMarker1));
    is.read(reinterpret_cast<char *>(&reservedGlobalMarker2), sizeof(reservedGlobalMarker2));
    is.read(reinterpret_cast<char *>(&groupNameLength), sizeof(groupNameLength));
    is.read(reinterpret_cast<char *>(&markerNameLength), sizeof(markerNameLength));
    is.read(reinterpret_cast<char *>(&descriptionLength), sizeof(descriptionLength));
    is.read(reinterpret_cast<char *>(&reservedGlobalMarker3), sizeof(reservedGlobalMarker3));
    is.read(reinterpret_cast<char *>(&reservedGlobalMarker4), sizeof(reservedGlobalMarker4));
    groupName.resize(groupNameLength);
    is.read(const_cast<char *>(groupName.data()), groupNameLength);
    markerName.resize(markerNameLength);
    is.read(const_cast<char *>(markerName.data()), markerNameLength);
    description.resize(descriptionLength);
    is.read(const_cast<char *>(description.data()), descriptionLength);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void GlobalMarker::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&commentedEventType), sizeof(commentedEventType));
    os.write(reinterpret_cast<char *>(&foregroundColor), sizeof(foregroundColor));
    os.write(reinterpret_cast<char *>(&backgroundColor), sizeof(backgroundColor));
    os.write(reinterpret_cast<char *>(&isRelocatable), sizeof(isRelocatable));
    os.write(reinterpret_cast<char *>(&reservedGlobalMarker1), sizeof(reservedGlobalMarker1));
    os.write(reinterpret_cast<char *>(&reservedGlobalMarker2), sizeof(reservedGlobalMarker2));
    os.write(reinterpret_cast<char *>(&groupNameLength), sizeof(groupNameLength));
    os.write(reinterpret_cast<char *>(&markerNameLength), sizeof(markerNameLength));
    os.write(reinterpret_cast<char *>(&descriptionLength), sizeof(descriptionLength));
    os.write(reinterpret_cast<char *>(&reservedGlobalMarker3), sizeof(reservedGlobalMarker3));
    os.write(reinterpret_cast<char *>(&reservedGlobalMarker4), sizeof(reservedGlobalMarker4));
    os.write(const_cast<char *>(groupName.data()), groupNameLength);
    os.write(const_cast<char *>(markerName.data()), markerNameLength);
    os.write(const_cast<char *>(description.data()), descriptionLength);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t GlobalMarker::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(commentedEventType) +
        sizeof(foregroundColor) +
        sizeof(backgroundColor) +
        sizeof(isRelocatable) +
        sizeof(reservedGlobalMarker1) +
        sizeof(reservedGlobalMarker2) +
        sizeof(groupNameLength) +
        sizeof(markerNameLength) +
        sizeof(descriptionLength) +
        sizeof(reservedGlobalMarker3) +
        sizeof(reservedGlobalMarker4) +
        groupNameLength +
        markerNameLength +
        descriptionLength;
}

}
}

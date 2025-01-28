// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/GpsEvent.h>

namespace Vector {
namespace BLF {

GpsEvent::GpsEvent() :
    ObjectHeader(ObjectType::GPS_EVENT) {
}

void GpsEvent::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedGpsEvent), sizeof(reservedGpsEvent));
    is.read(reinterpret_cast<char *>(&latitude), sizeof(latitude));
    is.read(reinterpret_cast<char *>(&longitude), sizeof(longitude));
    is.read(reinterpret_cast<char *>(&altitude), sizeof(altitude));
    is.read(reinterpret_cast<char *>(&speed), sizeof(speed));
    is.read(reinterpret_cast<char *>(&course), sizeof(course));
}

void GpsEvent::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedGpsEvent), sizeof(reservedGpsEvent));
    os.write(reinterpret_cast<char *>(&latitude), sizeof(latitude));
    os.write(reinterpret_cast<char *>(&longitude), sizeof(longitude));
    os.write(reinterpret_cast<char *>(&altitude), sizeof(altitude));
    os.write(reinterpret_cast<char *>(&speed), sizeof(speed));
    os.write(reinterpret_cast<char *>(&course), sizeof(course));
}

uint32_t GpsEvent::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(flags) +
        sizeof(channel) +
        sizeof(reservedGpsEvent) +
        sizeof(latitude) +
        sizeof(longitude) +
        sizeof(altitude) +
        sizeof(speed) +
        sizeof(course);
}

}
}

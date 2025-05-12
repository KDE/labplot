// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinStatisticEvent.h>

namespace Vector {
namespace BLF {

LinStatisticEvent::LinStatisticEvent() :
    ObjectHeader(ObjectType::LIN_STATISTIC) {
}

void LinStatisticEvent::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedLinStatisticEvent1), sizeof(reservedLinStatisticEvent1));
    is.read(reinterpret_cast<char *>(&reservedLinStatisticEvent2), sizeof(reservedLinStatisticEvent2));
    is.read(reinterpret_cast<char *>(&busLoad), sizeof(busLoad));
    is.read(reinterpret_cast<char *>(&burstsTotal), sizeof(burstsTotal));
    is.read(reinterpret_cast<char *>(&burstsOverrun), sizeof(burstsOverrun));
    is.read(reinterpret_cast<char *>(&framesSent), sizeof(framesSent));
    is.read(reinterpret_cast<char *>(&framesReceived), sizeof(framesReceived));
    is.read(reinterpret_cast<char *>(&framesUnanswered), sizeof(framesUnanswered));
    is.read(reinterpret_cast<char *>(&reservedLinStatisticEvent3), sizeof(reservedLinStatisticEvent3));
}

void LinStatisticEvent::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedLinStatisticEvent1), sizeof(reservedLinStatisticEvent1));
    os.write(reinterpret_cast<char *>(&reservedLinStatisticEvent2), sizeof(reservedLinStatisticEvent2));
    os.write(reinterpret_cast<char *>(&busLoad), sizeof(busLoad));
    os.write(reinterpret_cast<char *>(&burstsTotal), sizeof(burstsTotal));
    os.write(reinterpret_cast<char *>(&burstsOverrun), sizeof(burstsOverrun));
    os.write(reinterpret_cast<char *>(&framesSent), sizeof(framesSent));
    os.write(reinterpret_cast<char *>(&framesReceived), sizeof(framesReceived));
    os.write(reinterpret_cast<char *>(&framesUnanswered), sizeof(framesUnanswered));
    os.write(reinterpret_cast<char *>(&reservedLinStatisticEvent3), sizeof(reservedLinStatisticEvent3));
}

uint32_t LinStatisticEvent::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(reservedLinStatisticEvent1) +
        sizeof(reservedLinStatisticEvent2) +
        sizeof(busLoad) +
        sizeof(burstsTotal) +
        sizeof(burstsOverrun) +
        sizeof(framesSent) +
        sizeof(framesReceived) +
        sizeof(framesUnanswered) +
        sizeof(reservedLinStatisticEvent3);
}

}
}

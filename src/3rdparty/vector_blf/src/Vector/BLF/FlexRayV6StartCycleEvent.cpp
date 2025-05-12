// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/FlexRayV6StartCycleEvent.h>

namespace Vector {
namespace BLF {

FlexRayV6StartCycleEvent::FlexRayV6StartCycleEvent() :
    ObjectHeader(ObjectType::FLEXRAY_CYCLE) {
}

void FlexRayV6StartCycleEvent::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&lowTime), sizeof(lowTime));
    is.read(reinterpret_cast<char *>(&fpgaTick), sizeof(fpgaTick));
    is.read(reinterpret_cast<char *>(&fpgaTickOverflow), sizeof(fpgaTickOverflow));
    is.read(reinterpret_cast<char *>(&clientIndexFlexRayV6StartCycleEvent), sizeof(clientIndexFlexRayV6StartCycleEvent));
    is.read(reinterpret_cast<char *>(&clusterTime), sizeof(clusterTime));
    is.read(reinterpret_cast<char *>(dataBytes.data()), static_cast<std::streamsize>(dataBytes.size()));
    is.read(reinterpret_cast<char *>(&reservedFlexRayV6StartCycleEvent), sizeof(reservedFlexRayV6StartCycleEvent));
}

void FlexRayV6StartCycleEvent::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&lowTime), sizeof(lowTime));
    os.write(reinterpret_cast<char *>(&fpgaTick), sizeof(fpgaTick));
    os.write(reinterpret_cast<char *>(&fpgaTickOverflow), sizeof(fpgaTickOverflow));
    os.write(reinterpret_cast<char *>(&clientIndexFlexRayV6StartCycleEvent), sizeof(clientIndexFlexRayV6StartCycleEvent));
    os.write(reinterpret_cast<char *>(&clusterTime), sizeof(clusterTime));
    os.write(reinterpret_cast<char *>(dataBytes.data()), static_cast<std::streamsize>(dataBytes.size()));
    os.write(reinterpret_cast<char *>(&reservedFlexRayV6StartCycleEvent), sizeof(reservedFlexRayV6StartCycleEvent));
}

uint32_t FlexRayV6StartCycleEvent::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(dir) +
        sizeof(lowTime) +
        sizeof(fpgaTick) +
        sizeof(fpgaTickOverflow) +
        sizeof(clientIndexFlexRayV6StartCycleEvent) +
        sizeof(clusterTime) +
        static_cast<uint32_t>(dataBytes.size()) +
        sizeof(reservedFlexRayV6StartCycleEvent);
}

}
}

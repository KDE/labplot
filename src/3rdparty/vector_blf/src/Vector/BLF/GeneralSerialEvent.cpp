// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/GeneralSerialEvent.h>

namespace Vector {
namespace BLF {

void GeneralSerialEvent::read(AbstractFile & is) {
    is.read(reinterpret_cast<char *>(&dataLength), sizeof(dataLength));
    is.read(reinterpret_cast<char *>(&timeStampsLength), sizeof(timeStampsLength));
    is.read(reinterpret_cast<char *>(&reservedGeneralSerialEvent), sizeof(reservedGeneralSerialEvent));
    data.resize(dataLength);
    is.read(reinterpret_cast<char *>(data.data()), dataLength);
    timeStamps.resize(timeStampsLength / sizeof(int64_t));
    is.read(reinterpret_cast<char *>(timeStamps.data()), timeStampsLength);
    // @note might be extended in future versions
}

void GeneralSerialEvent::write(AbstractFile & os) {
    /* pre processing */
    dataLength = static_cast<uint32_t>(data.size());
    timeStampsLength = static_cast<uint32_t>(timeStamps.size() * sizeof(int64_t));

    os.write(reinterpret_cast<char *>(&dataLength), sizeof(dataLength));
    os.write(reinterpret_cast<char *>(&timeStampsLength), sizeof(timeStampsLength));
    os.write(reinterpret_cast<char *>(&reservedGeneralSerialEvent), sizeof(reservedGeneralSerialEvent));
    os.write(reinterpret_cast<char *>(data.data()), dataLength);
    os.write(reinterpret_cast<char *>(timeStamps.data()), timeStampsLength);
}

uint32_t GeneralSerialEvent::calculateObjectSize() const {
    return
        sizeof(dataLength) +
        sizeof(timeStampsLength) +
        sizeof(reservedGeneralSerialEvent) +
        dataLength +
        timeStampsLength;
}

}
}

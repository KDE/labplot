// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinDatabyteTimestampEvent.h>

namespace Vector {
namespace BLF {

void LinDatabyteTimestampEvent::read(AbstractFile & is) {
    LinMessageDescriptor::read(is);
    is.read(reinterpret_cast<char *>(databyteTimestamps.data()), static_cast<std::streamsize>(databyteTimestamps.size() * sizeof(uint64_t)));
}

void LinDatabyteTimestampEvent::write(AbstractFile & os) {
    LinMessageDescriptor::write(os);
    os.write(reinterpret_cast<char *>(databyteTimestamps.data()), static_cast<std::streamsize>(databyteTimestamps.size() * sizeof(uint64_t)));
}

uint32_t LinDatabyteTimestampEvent::calculateObjectSize() const {
    return
        LinMessageDescriptor::calculateObjectSize() +
        static_cast<uint32_t>(databyteTimestamps.size() * sizeof(uint64_t));
}

}
}

// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinDlcInfo.h>

namespace Vector {
namespace BLF {

LinDlcInfo::LinDlcInfo() :
    ObjectHeader(ObjectType::LIN_DLC_INFO) {
}

void LinDlcInfo::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&id), sizeof(id));
    is.read(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    is.read(reinterpret_cast<char *>(&reservedLinDlcInfo), sizeof(reservedLinDlcInfo));
}

void LinDlcInfo::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&id), sizeof(id));
    os.write(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    os.write(reinterpret_cast<char *>(&reservedLinDlcInfo), sizeof(reservedLinDlcInfo));
}

uint32_t LinDlcInfo::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(id) +
        sizeof(dlc) +
        sizeof(reservedLinDlcInfo);
}

}
}

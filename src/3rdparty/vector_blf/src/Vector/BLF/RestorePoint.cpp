// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/RestorePoint.h>

namespace Vector {
namespace BLF {

void RestorePoint::read(AbstractFile & is) {
    is.read(reinterpret_cast<char *>(&timeStamp), sizeof(timeStamp));
    is.read(reinterpret_cast<char *>(&compressedFilePosition), sizeof(compressedFilePosition));
    is.read(reinterpret_cast<char *>(&uncompressedFileOffset), sizeof(uncompressedFileOffset));
    is.read(reinterpret_cast<char *>(&unknownRestorePoint), sizeof(unknownRestorePoint));
}

void RestorePoint::write(AbstractFile & os) {
    os.write(reinterpret_cast<char *>(&timeStamp), sizeof(timeStamp));
    os.write(reinterpret_cast<char *>(&compressedFilePosition), sizeof(compressedFilePosition));
    os.write(reinterpret_cast<char *>(&uncompressedFileOffset), sizeof(uncompressedFileOffset));
    os.write(reinterpret_cast<char *>(&unknownRestorePoint), sizeof(unknownRestorePoint));
}

uint32_t RestorePoint::calculateObjectSize() {
    return
        sizeof(timeStamp) +
        sizeof(compressedFilePosition) +
        sizeof(uncompressedFileOffset) +
        sizeof(unknownRestorePoint);
}

}
}

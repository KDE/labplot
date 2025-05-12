// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LogContainer.h>

#include <zlib.h>

#include <Vector/BLF/Exceptions.h>

namespace Vector {
namespace BLF {

LogContainer::LogContainer() :
    ObjectHeaderBase(1, ObjectType::LOG_CONTAINER) {
}

void LogContainer::read(AbstractFile & is) {
    ObjectHeaderBase::read(is);
    is.read(reinterpret_cast<char *>(&compressionMethod), sizeof(compressionMethod));
    is.read(reinterpret_cast<char *>(&reservedLogContainer1), sizeof(reservedLogContainer1));
    is.read(reinterpret_cast<char *>(&reservedLogContainer2), sizeof(reservedLogContainer2));
    is.read(reinterpret_cast<char *>(&uncompressedFileSize), sizeof(uncompressedFileSize));
    is.read(reinterpret_cast<char *>(&reservedLogContainer3), sizeof(reservedLogContainer3));
    compressedFileSize = objectSize - internalHeaderSize();
    compressedFile.resize(compressedFileSize);
    is.read(reinterpret_cast<char *>(compressedFile.data()), compressedFileSize);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void LogContainer::write(AbstractFile & os) {
    /* pre processing */
    compressedFileSize = static_cast<uint32_t>(compressedFile.size());

    ObjectHeaderBase::write(os);
    os.write(reinterpret_cast<char *>(&compressionMethod), sizeof(compressionMethod));
    os.write(reinterpret_cast<char *>(&reservedLogContainer1), sizeof(reservedLogContainer1));
    os.write(reinterpret_cast<char *>(&reservedLogContainer2), sizeof(reservedLogContainer2));
    os.write(reinterpret_cast<char *>(&uncompressedFileSize), sizeof(uncompressedFileSize));
    os.write(reinterpret_cast<char *>(&reservedLogContainer3), sizeof(reservedLogContainer3));
    os.write(reinterpret_cast<char *>(compressedFile.data()), compressedFileSize);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t LogContainer::calculateObjectSize() const {
    return
        internalHeaderSize() +
        static_cast<uint32_t>(compressedFile.size());
}

uint16_t LogContainer::internalHeaderSize() const {
    return
        ObjectHeaderBase::calculateHeaderSize() +
        sizeof(compressionMethod) +
        sizeof(reservedLogContainer1) +
        sizeof(reservedLogContainer2) +
        sizeof(uncompressedFileSize) +
        sizeof(reservedLogContainer3);
}

void LogContainer::uncompress() {
    switch (compressionMethod) {
    case 0: /* no compression */
        uncompressedFile = compressedFile;
        break;

    case 2: { /* zlib compress */
        /* create buffer */
        uLong size = static_cast<uLong>(uncompressedFileSize);
        uncompressedFile.resize(size);

        /* inflate */
        int retVal = ::uncompress(
                         reinterpret_cast<Byte *>(uncompressedFile.data()),
                         &size,
                         reinterpret_cast<Byte *>(compressedFile.data()),
                         static_cast<uLong>(compressedFileSize));
        if (size != uncompressedFileSize)
            throw Exception("LogContainer::uncompress(): unexpected uncompressedSize");
        if (retVal != Z_OK)
            throw Exception("LogContainer::uncompress(): uncompress error");
    }
    break;

    default:
        throw Exception("LogContainer::uncompress(): unknown compression method");
    }
}

void LogContainer::compress(const uint16_t compressionMethod, const int compressionLevel) {
    this->compressionMethod = compressionMethod;

    switch (compressionMethod) {
    case 0: /* no compression */
        compressedFile = uncompressedFile;
        compressedFileSize = uncompressedFileSize;
        break;

    case 2: { /* zlib compress */
        /* deflate/compress data */
        uLong compressedBufferSize = compressBound(uncompressedFileSize);
        compressedFile.resize(compressedBufferSize); // extend
        int retVal = ::compress2(
                         reinterpret_cast<Byte *>(compressedFile.data()),
                         &compressedBufferSize,
                         reinterpret_cast<Byte *>(uncompressedFile.data()),
                         uncompressedFileSize,
                         compressionLevel);
        if (retVal != Z_OK)
            throw Exception("File::uncompressedFile2CompressedFile(): compress2 error");
        compressedFileSize = static_cast<uint32_t>(compressedBufferSize);
        compressedFile.resize(compressedFileSize); // shrink
    }
    break;

    default:
        throw Exception("LogContainer::compress(): unknown compression method");
    }
}

}
}

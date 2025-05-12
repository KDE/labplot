// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/ObjectHeaderBase.h>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/Exceptions.h>

namespace Vector {
	namespace BLF {

		ObjectHeaderBase::ObjectHeaderBase(const uint16_t headerVersion, const ObjectType objectType) :
			headerVersion(headerVersion),
			objectType(objectType) {
		}

		void ObjectHeaderBase::read(AbstractFile& is) {
			/* let us be more robust, when reading */
			uint32_t tmp = 0;
			while (tmp != ObjectSignature) {
				is.read(reinterpret_cast<char *>(&tmp), sizeof(tmp));
				if (tmp == ObjectSignature) {
					signature = tmp;
				} else {
					if (is.eof()) {
						throw Exception("ObjectHeaderBase::read(): End of File.");
					}

					if ((0xffffff00 & tmp) == 0x424f4c00) {
						is.seekg(-3, std::ios_base::cur);
					} else if ((0xffff0000 & tmp) == 0x4f4c0000) {
						is.seekg(-2, std::ios_base::cur);
					} else if ((0xff000000 & tmp) == 0x4c000000) {
						is.seekg(-1, std::ios_base::cur);
					} else {
						/* do not seek as we did not find a single char */
					}
				}
			}
			is.read(reinterpret_cast<char*>(&headerSize), sizeof(headerSize));
			is.read(reinterpret_cast<char*>(&headerVersion), sizeof(headerVersion));
			is.read(reinterpret_cast<char*>(&objectSize), sizeof(objectSize));
			is.read(reinterpret_cast<char*>(&objectType), sizeof(objectType));
		}

		void ObjectHeaderBase::write(AbstractFile& os) {
			/* pre processing */
			headerSize = calculateHeaderSize();
			objectSize = calculateObjectSize();

			os.write(reinterpret_cast<char*>(&signature), sizeof(signature));
			os.write(reinterpret_cast<char*>(&headerSize), sizeof(headerSize));
			os.write(reinterpret_cast<char*>(&headerVersion), sizeof(headerVersion));
			os.write(reinterpret_cast<char*>(&objectSize), sizeof(objectSize));
			os.write(reinterpret_cast<char*>(&objectType), sizeof(objectType));
		}

		uint16_t ObjectHeaderBase::calculateHeaderSize() const {
			return
				sizeof(signature) +
				sizeof(headerSize) +
				sizeof(headerVersion) +
				sizeof(objectSize) +
				sizeof(objectType);
		}

		uint32_t ObjectHeaderBase::calculateObjectSize() const {
			return calculateHeaderSize();
		}

	}
}

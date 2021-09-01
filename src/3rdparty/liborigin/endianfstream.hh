/*
    File                 : endianfstream.hh
    --------------------------------------------------------------------
    Description          : Endianless file stream class

    SPDX-FileCopyrightText: 2008 Alex Kargovsky <kargovsky@yumr.phys.msu.su>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ENDIAN_FSTREAM_H
#define ENDIAN_FSTREAM_H

#include <fstream>
#include "OriginObj.h"

namespace std
{
	class iendianfstream : public ifstream
	{
	public:
		iendianfstream(const char *_Filename, ios_base::openmode _Mode = ios_base::in)
			:	ifstream(_Filename, _Mode)
		{
			short word = 0x4321;
			bigEndian = (*(char*)& word) != 0x21;
		};

		iendianfstream& operator>>(bool& value)
		{
			char c;
			get(c);
			value = (c != 0);
			return *this;
		}

		iendianfstream& operator>>(char& value)
		{
			get(value);
			return *this;
		}

		iendianfstream& operator>>(unsigned char& value)
		{
			get(reinterpret_cast<char&>(value));
			return *this;
		}

		iendianfstream& operator>>(short& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			return *this;
		}

		iendianfstream& operator>>(unsigned short& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			return *this;
		}

		iendianfstream& operator>>(int& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			return *this;
		}

		iendianfstream& operator>>(unsigned int& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			return *this;
		}

		iendianfstream& operator>>(long& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			return *this;
		}

		iendianfstream& operator>>(unsigned long& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			return *this;
		}

		iendianfstream& operator>>(float& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			return *this;
		}

		iendianfstream& operator>>(double& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			return *this;
		}

		iendianfstream& operator>>(long double& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			return *this;
		}

		iendianfstream& operator>>(string& value)
		{
			read(reinterpret_cast<char*>(&value[0]), value.size());
			string::size_type pos = value.find_first_of('\0');
			if(pos != string::npos)
				value.resize(pos);

			return *this;
		}

		iendianfstream& operator>>(Origin::Color& value)
		{
			unsigned char color[4];
			read(reinterpret_cast<char*>(&color), sizeof(color));
			switch(color[3])
			{
			case 0:
				if(color[0] < 0x64)
				{
					value.type = Origin::Color::Regular;
					value.regular = color[0];
				}
				else
				{
					switch(color[2])
					{
					case 0:
						value.type = Origin::Color::Indexing;
						break;
					case 0x40:
						value.type = Origin::Color::Mapping;
						break;
					case 0x80:
						value.type = Origin::Color::RGB;
						break;
					}

					value.column = color[0] - 0x64;
				}
				
				break;
			case 1:
				value.type = Origin::Color::Custom;
				for(int i = 0; i < 3; ++i)
					value.custom[i] = color[i];
				break;
			case 0x20:
				value.type = Origin::Color::Increment;
				value.starting = color[1];
				break;
			case 0xFF:
				if(color[0] == 0xFC)
					value.type = Origin::Color::None;
				else if(color[0] == 0xF7)
					value.type = Origin::Color::Automatic;

				break;

			default:
				value.type = Origin::Color::Regular;
				value.regular = color[0];
				break;

			}

			return *this;
		}

	private:
		bool bigEndian;
		void swap_bytes(unsigned char* data, int size)
		{
			int i = 0;
			int j = size - 1;
			while(i < j)
			{
				std::swap(data[i], data[j]);
				++i, --j;
			}
		}
	};
}

#endif // ENDIAN_FSTREAM_H

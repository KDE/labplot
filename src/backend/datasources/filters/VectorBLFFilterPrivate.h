/*
	File                 : VectorBLFFilterPrivate.h
	Project              : LabPlot
	Description          : Private implementation class for VectorBLFFilter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef VECTORBLFFILTERPRIVATE_H
#define VECTORBLFFILTERPRIVATE_H

#include "CANFilterPrivate.h"
#include "VectorBLFFilter.h"

class AbstractDataSource;
class VectorBLFFilter;

class VectorBLFFilterPrivate : public CANFilterPrivate {
public:
	explicit VectorBLFFilterPrivate(VectorBLFFilter*);
	virtual ~VectorBLFFilterPrivate() {
	}
	virtual bool isValid(const QString& filename) const override;

private:
	enum class ParseStatus {
		Success,
		ErrorUnknown,
		ErrorInvalidFile,
		DBCInvalidFile,
		DBCMessageToLong,
		DBCBigEndian,
		DBCParserUnsupported,
		DBCUnknownID,
		DBCInvalidConversion
	};
	static ParseStatus DBCParserParseStatusToVectorBLFStatus(DbcParser::ParseStatus);

	struct Warning {
		ParseStatus status;
		uint32_t CANId;
	};
	void addWarningError(const Warning&) const;

private:
	virtual int readDataFromFileCommonTime(const QString& fileName, int lines = -1) override;
	virtual int readDataFromFileSeparateTime(const QString& fileName, int lines = -1) override;

#ifdef HAVE_VECTOR_BLF
	VectorBLFFilter* const q;
#endif
};

#endif // VECTORBLFFILTERPRIVATE_H

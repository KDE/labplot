/*
	File                 : VectorASCFilterPrivate.h
	Project              : LabPlot
	Description          : Vector ASC I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef VECTORASCFILTERPRIVATE_H
#define VECTORASCFILTERPRIVATE_H

#include "CANFilterPrivate.h"
#include "VectorASCFilter.h"

class AbstractDataSource;

class VectorASCFilterPrivate : public CANFilterPrivate {
public:
	explicit VectorASCFilterPrivate(VectorASCFilter*);
	virtual ~VectorASCFilterPrivate() {
	}
	virtual bool isValid(const QString& filename) const override;

	virtual QStringList lastErrors() const override;

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
	static ParseStatus DBCParserParseStatusToVectorASCStatus(DbcParser::ParseStatus);
	struct Errors {
		ParseStatus e;
		uint32_t CANId;
	};
	QList<Errors> errors;

private:
	virtual int readDataFromFileCommonTime(const QString& fileName, int lines = -1) override;
	virtual int readDataFromFileSeparateTime(const QString& fileName, int lines = -1) override;

	const VectorASCFilter* q;
};

#endif // VECTORASCFILTERPRIVATE_H

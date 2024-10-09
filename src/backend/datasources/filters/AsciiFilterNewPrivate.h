/*
	File                 : AsciiFilterPrivate.h
	Project              : LabPlot
	Description          : Private implementation class for AsciiFilter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASCIIFILTERNEWPRIVATE_H
#define ASCIIFILTERNEWPRIVATE_H

#include <QString>
#include <QLocale>

#include "AsciiFilterNew.h"

class AsciiFilterNewPrivate {
public:
	AsciiFilterNewPrivate(AsciiFilterNew *owner);

	AsciiFilterNew::Status initialize(QIODevice& device);
	qint64 readFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines);
	QVector<QStringList> preview(const QString& fileName, int lines);

	AsciiFilterNew::Properties properties;
	bool dirty{true}; // false if an initialization with the current properties was done, otherwise true

private:
	QString determineSeparator(QString& line);
	AsciiFilterNew::Status getLine(QIODevice& device, QString& line);
	static QString deviceStatusToString(AsciiFilterNew::Status);
	void setLastError(AsciiFilterNew::Status);

private:
	AsciiFilterNew* const q;

};

#endif // ASCIIFILTERNEWPRIVATE_H

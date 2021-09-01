/*
File                 : ImageFilter.h
Project              : LabPlot
Description          : Image I/O-filter
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2015 Stefan Gerlach (stefan.gerlach@uni.kn)
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef IMAGEFILTER_H
#define IMAGEFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"

class ImageFilterPrivate;
class QStringList;

class ImageFilter : public AbstractFileFilter {
	Q_OBJECT
	Q_ENUMS(ImportFormat)

public:
	enum class ImportFormat {MATRIX, XYZ, XYRGB};

	ImageFilter();
	~ImageFilter() override;

	static QStringList importFormats();
	static QString fileInfoString(const QString&);

	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace) override;
	void write(const QString& fileName, AbstractDataSource*) override;

	void loadFilterSettings(const QString&) override;
	void saveFilterSettings(const QString&) const override;

	void setImportFormat(const ImageFilter::ImportFormat);
	ImageFilter::ImportFormat importFormat() const;

	void setStartRow(const int);
	int startRow() const;
	void setEndRow(const int);
	int endRow() const;
	void setStartColumn(const int);
	int startColumn() const;
	void setEndColumn(const int);
	int endColumn() const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

private:
	std::unique_ptr<ImageFilterPrivate> const d;
	friend class ImageFilterPrivate;
};

#endif

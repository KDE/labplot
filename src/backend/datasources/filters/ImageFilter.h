/*
	File                 : ImageFilter.h
	Project              : LabPlot
	Description          : Image I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef IMAGEFILTER_H
#define IMAGEFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"

class ImageFilterPrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT ImageFilter : public AbstractFileFilter {
#else
class ImageFilter : public AbstractFileFilter {
#endif
	Q_OBJECT

public:
	enum class ImportFormat { MATRIX, XYZ, XYRGB };
	Q_ENUM(ImportFormat)

	ImageFilter();
	~ImageFilter() override;

	static QStringList importFormats();
	static QString fileInfoString(const QString&);

	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace) override;
	void write(const QString& fileName, AbstractDataSource*) override;

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

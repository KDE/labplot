/*
File                 : ImageFilterPrivate.h
Project              : LabPlot
Description          : Private implementation class for ImageFilter.
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2015 Stefan Gerlach (stefan.gerlach@uni.kn)

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef IMAGEFILTERPRIVATE_H
#define IMAGEFILTERPRIVATE_H

class AbstractDataSource;

class ImageFilterPrivate {

public:
	explicit ImageFilterPrivate(ImageFilter*);

	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	void write(const QString& fileName, AbstractDataSource*);

	const ImageFilter* q;

	ImageFilter::ImportFormat importFormat{ImageFilter::ImportFormat::MATRIX};	// how to import the image
	int startRow{1};		// start row
	int endRow{-1}; 		// end row
	int startColumn{1};		// start column
	int endColumn{-1};		// end column
};

#endif

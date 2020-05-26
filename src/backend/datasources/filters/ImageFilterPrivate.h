/***************************************************************************
File                 : ImageFilterPrivate.h
Project              : LabPlot
Description          : Private implementation class for ImageFilter.
--------------------------------------------------------------------
Copyright            : (C) 2015 Stefan Gerlach (stefan.gerlach@uni.kn)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
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

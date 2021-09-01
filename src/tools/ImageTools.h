/*
    File                 : ImageTools.cpp
    Project              : LabPlot
    Description          : Collection of different image processing algorithms
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef IMAGETOOLS_H
#define IMAGETOOLS_H

#include <QImage>

class ImageTools {

public:
	static QImage blurred(const QImage& image, QRect rect, int radius, bool alphaOnly = false);
};

#endif

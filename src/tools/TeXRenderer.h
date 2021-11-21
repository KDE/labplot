/*
    File                 : TeXRenderer.h
    Project              : LabPlot
    Description          : TeX renderer class
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2008-2016 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEXRENDERER_H
#define TEXRENDERER_H

#include <QColor>

class QString;
class QImage;
class QTemporaryFile;

class TeXRenderer {
public:
	struct Formatting {
		QColor fontColor;
		QColor backgroundColor;
		int fontSize;
		QString fontFamily;
		int dpi;
	};

	static QByteArray renderImageLaTeX(const QString&, bool* success, const TeXRenderer::Formatting&);
	static QByteArray imageFromPDF(const QTemporaryFile&, const int dpi, const QString& engine, bool* success);
	static QByteArray imageFromDVI(const QTemporaryFile&, const int dpi, bool* success);
	static bool enabled();
	static bool executableExists(const QString&);
};

#endif

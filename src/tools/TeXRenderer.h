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

	struct Result {
		Result()
			: successful(false) {
		}
		bool successful;
		QString errorMessage;
	};

	static QByteArray renderImageLaTeX(const QString&, Result*, const TeXRenderer::Formatting&);
	static bool executeLatexProcess(const QString& engine, const QString& baseName, const QTemporaryFile&, const QString& resultFileExtension, Result*);
	static QByteArray imageFromPDF(const QTemporaryFile&, const QString& engine, Result*);
	static QByteArray imageFromDVI(const QTemporaryFile&, const int dpi, Result*);
	static bool enabled();
	static bool executableExists(const QString&);
};

#endif

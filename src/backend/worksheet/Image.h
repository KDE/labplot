/*
	File                 : Image.h
	Project              : LabPlot
	Description          : Worksheet element to draw images
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGE_H
#define IMAGE_H

#include "backend/worksheet/WorksheetElement.h"

#include <QPen>

class Line;
class ImagePrivate;
class QBrush;
class QFont;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT Image : public WorksheetElement {
#else
class Image : public WorksheetElement {
#endif
	Q_OBJECT

public:
	explicit Image(const QString& name);
	~Image() override;

	QIcon icon() const override;
	virtual void setParentGraphicsItem(QGraphicsItem*) override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	CLASS_D_ACCESSOR_DECL(QString, fileName, FileName)
	BASIC_D_ACCESSOR_DECL(bool, embedded, Embedded)
	BASIC_D_ACCESSOR_DECL(qreal, opacity, Opacity)
	BASIC_D_ACCESSOR_DECL(int, width, Width)
	BASIC_D_ACCESSOR_DECL(int, height, Height)
	BASIC_D_ACCESSOR_DECL(bool, keepRatio, KeepRatio)

	Line* borderLine() const;

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	typedef ImagePrivate Private;

protected:
	Image(const QString&, ImagePrivate*);

private:
	Q_DECLARE_PRIVATE(Image)
	void init();

Q_SIGNALS:
	void fileNameChanged(const QString&);
	void embeddedChanged(bool);
	void opacityChanged(float);
	void widthChanged(int);
	void heightChanged(int);
	void keepRatioChanged(bool);
	void borderPenChanged(QPen&);
	void borderOpacityChanged(float);
};

#endif

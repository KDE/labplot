/*
	File                 : TextLabelPrivate.h
	Project              : LabPlot
	Description          : Private members of TextLabel
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2012-2021 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2019-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEXTLABELPRIVATE_H
#define TEXTLABELPRIVATE_H

#include "src/backend/worksheet/TextLabel.h"
#include "src/backend/worksheet/WorksheetElementPrivate.h"
#include "tools/TeXRenderer.h"
#include <QFutureWatcher>
#include <QScreen>
#include <QStaticText>

#include <gsl/gsl_const_cgs.h>

class QGraphicsSceneHoverEvent;
class CartesianPlot;
class CartesianCoordinateSystem;
class ScaledTextItem;
class TextLabel;
class QStaticText;

class TextLabelPrivate : public WorksheetElementPrivate {
public:
	explicit TextLabelPrivate(TextLabel*);

	double zoomFactor{-1.0};
	int teXImageResolution{static_cast<int>(QApplication::primaryScreen()->physicalDotsPerInchX())};
	double teXImageScaleFactor{
		Worksheet::convertToSceneUnits(GSL_CONST_CGS_INCH / QApplication::primaryScreen()->physicalDotsPerInchX(), Worksheet::Unit::Centimeter)};

	TextLabel::TextWrapper textWrapper;
	QFont teXFont{QStringLiteral("Computer Modern"), 12}; // reasonable default font and size
	QColor fontColor{Qt::black}; // used only by the theme for unformatted text. The text font is in the HTML and so this variable is never set
	QColor backgroundColor{Qt::transparent}; // same as fontColor
	QImage teXImage;
	QByteArray teXPdfData;
	QFutureWatcher<QByteArray> teXImageFutureWatcher;
	TeXRenderer::Result teXRenderResult;

	// see TextLabel::init() for type specific default settings

	TextLabel::BorderShape borderShape{TextLabel::BorderShape::NoBorder};
	QPen borderPen{Qt::black, Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point), Qt::SolidLine};
	qreal borderOpacity{1.0};

	void retransform() override;
	void updateBoundingRect();
	void setZoomFactor(double);
	virtual void recalcShapeAndBoundingRect() override;
	void updatePosition();
	void updateText();
	void updateTeXImage();
	void updateBorder();
	QRectF size();
	QPointF findNearestGluePoint(QPointF scenePoint);
	TextLabel::GluePoint gluePointAt(int index);

	ScaledTextItem* m_textItem{nullptr};

	QPainterPath borderShapePath;
	QPainterPath labelShape;

	// reimplemented from QGraphicsItem
	QPainterPath shape() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* = nullptr) override;
	TextLabel* const q{nullptr};

	// used in the InfoElement (Marker) to attach the line to the label
	QVector<TextLabel::GluePoint> m_gluePoints;
};

#endif

/***************************************************************************
    File                 : TextLabelPrivate.h
    Project              : LabPlot
    Description          : Private members of TextLabel
    --------------------------------------------------------------------
    Copyright            : (C) 2012-2014 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2019 by Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef TEXTLABELPRIVATE_H
#define TEXTLABELPRIVATE_H

#include "src/backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"

#include <QStaticText>
#include <QFutureWatcher>
#include <QGraphicsItem>
#include <QDesktopWidget>

class QGraphicsSceneHoverEvent;
class CartesianPlot;
class CartesianCoordinateSystem;
class TextLabel;

class TextLabelPrivate: public QGraphicsItem {
public:
	explicit TextLabelPrivate(TextLabel*);
	explicit TextLabelPrivate(TextLabel*, CartesianPlot* plot);

	qreal rotationAngle{0.0};
	//scaling:
	//we need to scale from the font size specified in points to scene units.
	//furhermore, we create the tex-image in a higher resolution then usual desktop resolution
	// -> take this into account
	double scaleFactor{Worksheet::convertToSceneUnits(1, Worksheet::Unit::Point)};
	int teXImageResolution{QApplication::desktop()->physicalDpiX()};
	//TODO: use constant for 2.54
	double teXImageScaleFactor{Worksheet::convertToSceneUnits(2.54/QApplication::desktop()->physicalDpiX(), Worksheet::Unit::Centimeter)};

	TextLabel::TextWrapper textWrapper;
	QFont teXFont{"Computer Modern", 42};
	QColor fontColor{Qt::black}; // used only by the theme for unformatted text. The text font is in the HTML and so this variable is never set
	QColor backgroundColor{Qt::white}; // used only by the theme for unformatted text. The text font is in the HTML and so this variable is never set
	QImage teXImage;
	QFutureWatcher<QImage> teXImageFutureWatcher;
	bool teXRenderSuccessful{false};

	// see TextLabel::init() for type specific default settings
	// position in parent's coordinate system, the label gets aligned around this point
	WorksheetElement::PositionWrapper position{
		QPoint(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter)),
		TextLabel::HorizontalPosition::Center, TextLabel::VerticalPosition::Center};
	bool positionInvalid{false};
	bool coordinateBindingEnabled{false};
	QPointF positionLogical;

	const CartesianPlot* plot{nullptr};

	WorksheetElement::HorizontalAlignment horizontalAlignment{WorksheetElement::HorizontalAlignment::Center};
	WorksheetElement::VerticalAlignment verticalAlignment{WorksheetElement::VerticalAlignment::Center};

	TextLabel::BorderShape borderShape{TextLabel::BorderShape::NoBorder};
	QPen borderPen{Qt::black, Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point), Qt::SolidLine};
	qreal borderOpacity{1.0};

	QString name() const;
	void retransform();
	bool swapVisible(bool on);
	virtual void recalcShapeAndBoundingRect();
	void updatePosition();
	QPointF positionFromItemPosition(QPointF);
	void updateText();
	void updateTeXImage();
	void updateBorder();
	QRectF size();
	QPointF findNearestGluePoint(QPointF scenePoint);
	TextLabel::GluePoint gluePointAt(int index);

	QStaticText staticText;

	bool suppressItemChangeEvent{false};
	bool suppressRetransform{false};
	bool m_hovered{false};
	bool m_coordBinding{false};
	bool m_coordBindingEnable{false};

	QRectF boundingRectangle; //bounding rectangle of the text
	QRectF transformedBoundingRectangle; //bounding rectangle of transformed (rotated etc.) text
	QPainterPath borderShapePath;
	QPainterPath labelShape;

	//reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

	TextLabel* const q{nullptr};

	// used in the InfoElement (Marker) to attach the line to the label
	QVector<TextLabel::GluePoint> m_gluePoints;

private:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void keyPressEvent(QKeyEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
	QPointF mapPlotAreaToParent(QPointF point);
	QPointF mapParentToPlotArea(QPointF point);
};

#endif

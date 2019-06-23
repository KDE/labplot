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

#include <QStaticText>
#include <QFutureWatcher>
#include <QGraphicsItem>
#include <QDesktopWidget>

class QGraphicsSceneHoverEvent;

class TextLabelPrivate: public QGraphicsItem {
public:
	explicit TextLabelPrivate(TextLabel*);

	qreal rotationAngle{0.0};
	//scaling:
	//we need to scale from the font size specified in points to scene units.
	//furhermore, we create the tex-image in a higher resolution then usual desktop resolution
	// -> take this into account
	float scaleFactor{Worksheet::convertToSceneUnits(1, Worksheet::Point)};
	int teXImageResolution{QApplication::desktop()->physicalDpiX()};
	float teXImageScaleFactor{Worksheet::convertToSceneUnits(2.54/QApplication::desktop()->physicalDpiX(), Worksheet::Centimeter)};

	TextLabel::TextWrapper textWrapper;
	QFont teXFont{"Computer Modern", 42};
	QColor fontColor{Qt::black}; // used only by the theme for unformatted text. The text font is in the HTML and so this variable is never set
	QColor backgroundColor{Qt::white}; // used only by the theme for unformatted text. The text font is in the HTML and so this variable is never set
	QImage teXImage;
	QFutureWatcher<QImage> teXImageFutureWatcher;
	bool teXRenderSuccessful{false};

	// see TextLabel::init() for type specific default settings
	// position in parent's coordinate system, the label gets aligned around this point
	TextLabel::PositionWrapper position{
		QPoint(Worksheet::convertToSceneUnits(1, Worksheet::Centimeter), Worksheet::convertToSceneUnits(1, Worksheet::Centimeter)),
		TextLabel::hPositionCenter, TextLabel::vPositionTop};
	bool positionInvalid{false};

	TextLabel::HorizontalAlignment horizontalAlignment{TextLabel::hAlignCenter};
	TextLabel::VerticalAlignment verticalAlignment{TextLabel::vAlignBottom};

	TextLabel::BorderShape borderShape{TextLabel::NoBorder};
	QPen borderPen{Qt::black, Worksheet::convertToSceneUnits(1.0, Worksheet::Point), Qt::SolidLine};
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
	QStaticText staticText;

	bool suppressItemChangeEvent{false};
	bool suppressRetransform{false};
	bool m_printing{false};
	bool m_hovered{false};

	QRectF boundingRectangle; //bounding rectangle of the text
	QRectF transformedBoundingRectangle; //bounding rectangle of transformed (rotated etc.) text
	QPainterPath borderShapePath;
	QPainterPath labelShape;

	//reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	QPainterPath shape() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

	TextLabel* const q;

private:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
};

#endif

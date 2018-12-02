/***************************************************************************
    File                 : TextLabel.h
    Project              : LabPlot
    Description          : Text label supporting reach text and latex formatting
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2012-2014 Alexander Semke (alexander.semke@web.de)

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

#ifndef TEXTLABEL_H
#define TEXTLABEL_H

#include "backend/lib/macros.h"
#include "tools/TeXRenderer.h"
#include "backend/worksheet/WorksheetElement.h"

#include <QFont>
#include <QBrush>
#include <QPen>

class TextLabelPrivate;

class TextLabel : public WorksheetElement {
Q_OBJECT

public:
	enum Type {General, PlotTitle, AxisTitle, PlotLegendTitle};

	enum HorizontalPosition {hPositionLeft, hPositionCenter, hPositionRight, hPositionCustom};
	enum VerticalPosition {vPositionTop, vPositionCenter, vPositionBottom, vPositionCustom};

	enum HorizontalAlignment {hAlignLeft, hAlignCenter, hAlignRight};
	enum VerticalAlignment {vAlignTop, vAlignCenter, vAlignBottom};

	enum BorderShape {NoBorder, Rect, Ellipse, RoundSideRect, RoundCornerRect, InwardsRoundCornerRect, DentedBorderRect,
			Cuboid, UpPointingRectangle, DownPointingRectangle, LeftPointingRectangle, RightPointingRectangle};

	struct TextWrapper {
		TextWrapper() : teXUsed(false) {}
		TextWrapper(const QString& t, bool b) : text(t), teXUsed(b) {}
		TextWrapper(const QString& t) : text(t), teXUsed(false) {}

		QString text;
		bool teXUsed;
	};

	struct PositionWrapper {
		QPointF 		   point;
		HorizontalPosition horizontalPosition;
		VerticalPosition   verticalPosition;
	};

	explicit TextLabel(const QString& name, Type type = General);
	~TextLabel() override;

	Type type() const;
	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;
	void setParentGraphicsItem(QGraphicsItem*);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig& config) override;
	void saveThemeConfig(const KConfig& config) override;

	CLASS_D_ACCESSOR_DECL(TextWrapper, text, Text)
	BASIC_D_ACCESSOR_DECL(QColor, teXFontColor, TeXFontColor)
	BASIC_D_ACCESSOR_DECL(QColor, teXBackgroundColor, TeXBackgroundColor)
	CLASS_D_ACCESSOR_DECL(QFont, teXFont, TeXFont)
	CLASS_D_ACCESSOR_DECL(PositionWrapper, position, Position)
	void setPosition(QPointF);
	void setPositionInvalid(bool);
	BASIC_D_ACCESSOR_DECL(HorizontalAlignment, horizontalAlignment, HorizontalAlignment)
	BASIC_D_ACCESSOR_DECL(VerticalAlignment, verticalAlignment, VerticalAlignment)
	BASIC_D_ACCESSOR_DECL(qreal, rotationAngle, RotationAngle)

	BASIC_D_ACCESSOR_DECL(BorderShape, borderShape, BorderShape);
	CLASS_D_ACCESSOR_DECL(QPen, borderPen, BorderPen)
	BASIC_D_ACCESSOR_DECL(qreal, borderOpacity, BorderOpacity)

	void setVisible(bool on) override;
	bool isVisible() const override;
	void setPrinting(bool) override;

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	typedef TextLabelPrivate Private;

private slots:
	void updateTeXImage();

	//SLOTs for changes triggered via QActions in the context menu
	void visibilityChanged();

protected:
	TextLabelPrivate* const d_ptr;
	TextLabel(const QString& name, TextLabelPrivate* dd, Type type = General);

private:
	Q_DECLARE_PRIVATE(TextLabel)
	void init();

	Type m_type;
	QAction* visibilityAction;

signals:
	void textWrapperChanged(const TextLabel::TextWrapper&);
	void teXFontSizeChanged(const int);
	void teXFontChanged(const QFont);
	void teXFontColorChanged(const QColor);
	void teXBackgroundColorChanged(const QColor);
	void positionChanged(const TextLabel::PositionWrapper&);
	void horizontalAlignmentChanged(TextLabel::HorizontalAlignment);
	void verticalAlignmentChanged(TextLabel::VerticalAlignment);
	void rotationAngleChanged(qreal);
	void visibleChanged(bool);
	void borderShapeChanged(TextLabel::BorderShape);
	void borderPenChanged(QPen&);
	void borderOpacityChanged(float);

	void teXImageUpdated(bool);
	void changed();
};

#endif

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
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"

#include <QPen>
#include <QTextEdit>

class QBrush;
class QFont;
class TextLabelPrivate;
class CartesianPlot;

class TextLabel : public WorksheetElement {
	Q_OBJECT

public:
	enum class Type {General, PlotTitle, AxisTitle, PlotLegendTitle, InfoElementLabel};
	enum class BorderShape {NoBorder, Rect, Ellipse, RoundSideRect, RoundCornerRect, InwardsRoundCornerRect, DentedBorderRect,
	                        Cuboid, UpPointingRectangle, DownPointingRectangle, LeftPointingRectangle, RightPointingRectangle
	                       };

	// The text is always in HMTL format
	struct TextWrapper {
		TextWrapper() {}
		TextWrapper(const QString& text, bool teXUsed, bool html): teXUsed(teXUsed) {
			if (teXUsed) {
				this->text = text; // LaTeX does not support HTML, so assume t is a plain text string
				return;
			}
			this->text = createHtml(text, html);
		}
		TextWrapper(const QString& text, bool html = false) {
			this->text = createHtml(text, html);
		}
		TextWrapper(const QString& text, bool html, QString& placeholder): teXUsed(false), allowPlaceholder(true), textPlaceholder(placeholder) {
			this->text = createHtml(text, html);
		}
		TextWrapper(const QString& text, bool teXUsed, bool html, bool allowPlaceholder): teXUsed(teXUsed), allowPlaceholder(allowPlaceholder) {
			if (teXUsed) {
				this->text = text; // latex does not support html, so assume t is a plain string
				return;
			}
			this->text = createHtml(text, html);
		}
		QString createHtml(QString text, bool isHtml) {
			if (isHtml)
				return text;

			QTextEdit te(text);
			return te.toHtml();
		}

		QString text;
		bool teXUsed{false};
		/*! Determines if the Textlabe can have a placeholder or not.
		 * Depending on this variable in the LabelWidget between
		 * the text and the placeholder text can be switched
		 */
		bool allowPlaceholder{false};
		QString textPlaceholder{""}; // text with placeholders
	};

	explicit TextLabel(const QString& name, Type type = Type::General);
	TextLabel(const QString& name, CartesianPlot*, Type type = Type::General);
	~TextLabel() override;

	Type type() const;
	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;
	void setParentGraphicsItem(QGraphicsItem*);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	CLASS_D_ACCESSOR_DECL(TextWrapper, text, Text)
	BASIC_D_ACCESSOR_DECL(QColor, fontColor, FontColor)
	BASIC_D_ACCESSOR_DECL(QColor, backgroundColor, BackgroundColor)
	CLASS_D_ACCESSOR_DECL(TextWrapper, textPlaceholder, PlaceholderText)
	BASIC_D_ACCESSOR_DECL(QColor, teXFontColor, TeXFontColor)
	BASIC_D_ACCESSOR_DECL(QColor, teXBackgroundColor, TeXBackgroundColor)
	CLASS_D_ACCESSOR_DECL(QFont, teXFont, TeXFont)
	CLASS_D_ACCESSOR_DECL(WorksheetElement::PositionWrapper, position, Position)
// 	BASIC_D_ACCESSOR_DELC(bool, coordinateBindingEnabled, CoordinateBindingEnabled)
	void setCoordinateBindingEnabled(bool);
	bool coordinateBindingEnabled() const;
	BASIC_D_ACCESSOR_DECL(QPointF, positionLogical, PositionLogical)
	void setPosition(QPointF);
	void setPositionInvalid(bool);
	BASIC_D_ACCESSOR_DECL(WorksheetElement::HorizontalAlignment, horizontalAlignment, HorizontalAlignment)
	BASIC_D_ACCESSOR_DECL(WorksheetElement::VerticalAlignment, verticalAlignment, VerticalAlignment)
	BASIC_D_ACCESSOR_DECL(qreal, rotationAngle, RotationAngle)

	BASIC_D_ACCESSOR_DECL(BorderShape, borderShape, BorderShape);
	CLASS_D_ACCESSOR_DECL(QPen, borderPen, BorderPen)
	BASIC_D_ACCESSOR_DECL(qreal, borderOpacity, BorderOpacity)

	void setVisible(bool on) override;
	bool isVisible() const override;
	void setPrinting(bool) override;
	QRectF size();
	QPointF findNearestGluePoint(QPointF scenePoint);
	int gluePointCount();
	struct GluePoint {
#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))	// we need a default constructor for QVector
		GluePoint() = default;
#endif
		GluePoint(QPointF point, QString name) : point(point), name(name) {}
		QPointF point;
		QString name;
	};

	GluePoint gluePointAt(int index);

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	typedef TextLabelPrivate Private;

private slots:
	void updateTeXImage();

	//SLOTs for changes triggered via QActions in the context menu
	void visibilityChanged();

protected:
	TextLabelPrivate* const d_ptr;
	TextLabel(const QString& name, TextLabelPrivate* dd, Type type = Type::General);

private:
	Q_DECLARE_PRIVATE(TextLabel)
	void init();

	Type m_type;
	QAction* visibilityAction{nullptr};

signals:
	void textWrapperChanged(const TextLabel::TextWrapper&);
	void teXFontSizeChanged(const int);
	void teXFontChanged(const QFont);
	void fontColorChanged(const QColor);
	void backgroundColorChanged(const QColor);
	void positionChanged(const WorksheetElement::PositionWrapper&);
	void coordinateBindingEnabledChanged(bool);
	void positionLogicalChanged(QPointF);
	void horizontalAlignmentChanged(WorksheetElement::HorizontalAlignment);
	void verticalAlignmentChanged(WorksheetElement::VerticalAlignment);
	void rotationAngleChanged(qreal);
	void visibleChanged(bool);
	void borderShapeChanged(TextLabel::BorderShape);
	void borderPenChanged(QPen&);
	void borderOpacityChanged(float);

	void teXImageUpdated(bool);
	void changed();
};

#endif

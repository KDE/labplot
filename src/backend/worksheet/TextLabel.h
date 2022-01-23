/*
    File                 : TextLabel.h
    Project              : LabPlot
    Description          : Text label supporting reach text and latex formatting
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2012-2014 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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
	enum class Mode {Text, LaTeX, Markdown};
	enum class BorderShape {NoBorder, Rect, Ellipse, RoundSideRect, RoundCornerRect, InwardsRoundCornerRect, DentedBorderRect,
	                        Cuboid, UpPointingRectangle, DownPointingRectangle, LeftPointingRectangle, RightPointingRectangle
	                       };

	// The text is always in HMTL format
	struct TextWrapper {
		TextWrapper() {}
		TextWrapper(const QString& text, TextLabel::Mode mode, bool html): mode(mode) {
			if (mode != TextLabel::Mode::Text) {
				this->text = text; //LaTeX and markdown use plain string
				return;
			}
			this->text = createHtml(text, html);
		}
		TextWrapper(const QString& text): mode(TextLabel::Mode::Text) {
			// assume text is not HTML yet
			this->text = createHtml(text, false);
		}
		TextWrapper(const QString& text, bool html, QString& placeholder): allowPlaceholder(true), textPlaceholder(placeholder) {
			this->text = createHtml(text, html);
		}
		TextWrapper(const QString& text, TextLabel::Mode mode, bool html, bool allowPlaceholder): mode(mode), allowPlaceholder(allowPlaceholder) {
			if (mode != TextLabel::Mode::Text) {
				this->text = text; //LaTeX and markdown use plain string
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
		TextLabel::Mode mode{TextLabel::Mode::Text};
		/*! Determines if the Textlabe can have a placeholder or not.
		 * Depending on this variable in the LabelWidget between
		 * the text and the placeholder text can be switched
		 */
		bool allowPlaceholder{false};
		QString textPlaceholder{QLatin1String("")}; // text with placeholders
	};

	explicit TextLabel(const QString& name, Type = Type::General);
	TextLabel(const QString& name, CartesianPlot*, Type = Type::General);
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

	BASIC_D_ACCESSOR_DECL(BorderShape, borderShape, BorderShape)
	CLASS_D_ACCESSOR_DECL(QPen, borderPen, BorderPen)
	BASIC_D_ACCESSOR_DECL(qreal, borderOpacity, BorderOpacity)

	void setZoomFactor(double);
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

private Q_SLOTS:
	void updateTeXImage();

protected:
	TextLabel(const QString& name, TextLabelPrivate* dd, Type = Type::General);

private:
	Q_DECLARE_PRIVATE(TextLabel)
	void init();

	Type m_type;
	QAction* visibilityAction{nullptr};

Q_SIGNALS:
	void textWrapperChanged(const TextLabel::TextWrapper&);
	void teXFontSizeChanged(const int);
	void teXFontChanged(const QFont);
	void fontColorChanged(const QColor);
	void backgroundColorChanged(const QColor);

	void borderShapeChanged(TextLabel::BorderShape);
	void borderPenChanged(QPen&);
	void borderOpacityChanged(float);

	void teXImageUpdated(bool);
};

#endif

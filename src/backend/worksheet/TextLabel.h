/*
	File                 : TextLabel.h
	Project              : LabPlot
	Description          : Text label supporting reach text and latex formatting
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2012-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEXTLABEL_H
#define TEXTLABEL_H

#include "backend/worksheet/WorksheetElement.h"
#include "tools/TeXRenderer.h"

#include <QTextEdit>

class Line;
class QBrush;
class QFont;
class TextLabelPrivate;
class CartesianPlot;
class QPen;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT TextLabel : public WorksheetElement {
#else
class TextLabel : public WorksheetElement {
#endif
	Q_OBJECT

public:
	enum class Type { General, PlotTitle, AxisTitle, PlotLegendTitle, InfoElementLabel };
	enum class Mode { Text, LaTeX, Markdown };
	enum class BorderShape {
		NoBorder,
		Rect,
		Ellipse,
		RoundSideRect,
		RoundCornerRect,
		InwardsRoundCornerRect,
		DentedBorderRect,
		Cuboid,
		UpPointingRectangle,
		DownPointingRectangle,
		LeftPointingRectangle,
		RightPointingRectangle
	};

	struct TextWrapper {
		TextWrapper() = default;
		TextWrapper(const QString& twtext, TextLabel::Mode twmode, bool html)
			: mode(twmode) {
			if (mode == TextLabel::Mode::Text)
				text = createHtml(twtext, html);
			else // LaTeX and markdown use plain string
				text = twtext;
		}
		TextWrapper(const QString& twtext)
			: mode(TextLabel::Mode::Text) {
			// assume text is not HTML yet
			text = createHtml(twtext, false);
		}
		TextWrapper(const QString& twtext, bool html, QString& placeholder)
			: allowPlaceholder(true)
			, textPlaceholder(placeholder) {
			text = createHtml(twtext, html);
		}
		TextWrapper(const QString& twtext, TextLabel::Mode twmode, bool html, bool twAllowPlaceholder)
			: allowPlaceholder(twAllowPlaceholder) {
			TextWrapper(twtext, twmode, html);
		}
		QString createHtml(QString plaintext, bool isHtml) {
			if (isHtml || plaintext.isEmpty())
				return plaintext;

			QTextEdit te(plaintext);
			// the html does not contain any colors!
			return te.toHtml();
		}

		bool isHtml() const {
			return text.startsWith(QStringLiteral("<!DOCTYPE HTML"));
		}

		bool operator!=(const TextWrapper& other) const {
			return (text != other.text || mode != other.mode || allowPlaceholder != other.allowPlaceholder
					|| ((allowPlaceholder || other.allowPlaceholder) && textPlaceholder != other.textPlaceholder));
		}

		bool operator==(const TextWrapper& other) const {
			return (text == other.text && mode == other.mode && allowPlaceholder == other.allowPlaceholder
					&& ((allowPlaceholder || other.allowPlaceholder) && textPlaceholder == other.textPlaceholder));
		}

		QString text; // actual text. Contains font and color/bg color in Text mode
		TextLabel::Mode mode{TextLabel::Mode::Text};
		/*! Determines if the Textlabel can have a placeholder or not.
		 * Depending on this variable in the LabelWidget between
		 * the text and the placeholder text can be switched
		 */
		bool allowPlaceholder{false};
		QString textPlaceholder{QLatin1String("")}; // text with placeholders
	};

	explicit TextLabel(const QString& name, Type = Type::General);
	TextLabel(const QString& name, CartesianPlot*, Type = Type::General);
	~TextLabel() override;

	QIcon icon() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	CLASS_D_ACCESSOR_DECL(TextWrapper, text, Text)
	void setFont(const QFont&);
	BASIC_D_ACCESSOR_DECL(QColor, fontColor, FontColor)
	BASIC_D_ACCESSOR_DECL(QColor, backgroundColor, BackgroundColor)
	void setPlaceholderText(const TextWrapper& value);
	CLASS_D_ACCESSOR_DECL(QFont, teXFont, TeXFont)

	BASIC_D_ACCESSOR_DECL(BorderShape, borderShape, BorderShape)
	Line* borderLine() const;

	void setZoomFactor(double);
	QRectF size();
	QPointF findNearestGluePoint(QPointF scenePoint);
	int gluePointCount();
	struct GluePoint {
		GluePoint(QPointF gpoint, const QString& gpname)
			: point(gpoint)
			, name(gpname) {
		}
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
	TextLabel(const QString& name, TextLabelPrivate*, Type = Type::General);

private:
	Q_DECLARE_PRIVATE(TextLabel)
	void init();

	Type m_type;

Q_SIGNALS:
	void textWrapperChanged(const TextLabel::TextWrapper&);
	void teXFontSizeChanged(const int);
	void teXFontChanged(const QFont);
	void fontColorChanged(const QColor);
	void backgroundColorChanged(const QColor);
	void borderShapeChanged(TextLabel::BorderShape);
	void teXImageUpdated(const TeXRenderer::Result&);
};

#endif

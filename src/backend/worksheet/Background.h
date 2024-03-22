/*
	File                 : Background.h
	Project              : LabPlot
	Description          : Background
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "backend/core/AbstractAspect.h"
#include "backend/lib/macros.h"

class BackgroundPrivate;
class KConfigGroup;

class Background : public AbstractAspect {
	Q_OBJECT

public:
	enum class Position { No, Above, Below, ZeroBaseline, Left, Right };
	enum class Type { Color, Image, Pattern };
	enum class ColorStyle {
		SingleColor,
		HorizontalLinearGradient,
		VerticalLinearGradient,
		TopLeftDiagonalLinearGradient,
		BottomLeftDiagonalLinearGradient,
		RadialGradient
	};
	enum class ImageStyle { ScaledCropped, Scaled, ScaledAspectRatio, Centered, Tiled, CenterTiled };

	explicit Background(const QString& name);
	~Background() override;

	void setPrefix(const QString&);
	const QString& prefix() const;
	void init(const KConfigGroup&);

	void draw(QPainter*, const QPolygonF&) const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfigGroup&);
	void loadThemeConfig(const KConfigGroup&, const QColor&);
	void saveThemeConfig(KConfigGroup&) const;

	BASIC_D_ACCESSOR_DECL(bool, enabledAvailable, EnabledAvailable)
	BASIC_D_ACCESSOR_DECL(bool, positionAvailable, PositionAvailable)

	BASIC_D_ACCESSOR_DECL(bool, enabled, Enabled)
	BASIC_D_ACCESSOR_DECL(Position, position, Position)
	BASIC_D_ACCESSOR_DECL(Type, type, Type)
	BASIC_D_ACCESSOR_DECL(ColorStyle, colorStyle, ColorStyle)
	BASIC_D_ACCESSOR_DECL(ImageStyle, imageStyle, ImageStyle)
	BASIC_D_ACCESSOR_DECL(Qt::BrushStyle, brushStyle, BrushStyle)
	CLASS_D_ACCESSOR_DECL(QColor, firstColor, FirstColor)
	CLASS_D_ACCESSOR_DECL(QColor, secondColor, SecondColor)
	CLASS_D_ACCESSOR_DECL(QString, fileName, FileName)
	BASIC_D_ACCESSOR_DECL(double, opacity, Opacity)

	typedef BackgroundPrivate Private;

protected:
	BackgroundPrivate* const d_ptr;

private:
	Q_DECLARE_PRIVATE(Background)

Q_SIGNALS:
	void enabledChanged(bool);
	void positionChanged(Position);
	void typeChanged(Type);
	void colorStyleChanged(ColorStyle);
	void imageStyleChanged(ImageStyle);
	void brushStyleChanged(Qt::BrushStyle);
	void firstColorChanged(const QColor&);
	void secondColorChanged(const QColor&);
	void fileNameChanged(const QString&);
	void opacityChanged(float);

	void updatePositionRequested();
	void updateRequested();
};

#endif

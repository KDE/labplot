/*
	File                 : ErrorBarStyle.h
	Project              : LabPlot
	Description          : The styling related part of the error bar definition
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ERRORBARSTYLE_H
#define ERRORBARSTYLE_H

#include "backend/core/AbstractAspect.h"
#include "backend/lib/macros.h"

class ErrorBarStylePrivate;
class Line;
class KConfigGroup;
class QPainter;
class QPainterPath;

class ErrorBarStyle : public AbstractAspect {
	Q_OBJECT

public:
	enum class Type { Simple, WithEnds };

	explicit ErrorBarStyle(const QString& name);
	~ErrorBarStyle() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfigGroup&);
	void loadThemeConfig(const KConfigGroup&, const QColor&);
	void saveThemeConfig(KConfigGroup&) const;

	void init(const KConfigGroup&);
	void draw(QPainter*, const QPainterPath&);

	BASIC_D_ACCESSOR_DECL(Type, type, Type)
	BASIC_D_ACCESSOR_DECL(double, capSize, CapSize)
	Line* line() const;

	typedef ErrorBarStylePrivate Private;

protected:
	ErrorBarStylePrivate* const d_ptr;

private:
	Q_DECLARE_PRIVATE(ErrorBarStyle)

Q_SIGNALS:
	void updateRequested();
	void updatePixmapRequested();

	void typeChanged(ErrorBarStyle::Type);
	void capSizeChanged(double);
};

#endif

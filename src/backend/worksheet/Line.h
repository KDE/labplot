/*
	File                 : Line.h
	Project              : LabPlot
	Description          : Line
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LINE_H
#define LINE_H

#include "backend/core/AbstractAspect.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"

class LinePrivate;
class KConfigGroup;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT Line : public AbstractAspect {
#else
class Line : public AbstractAspect {
#endif
	Q_OBJECT

public:
	explicit Line(const QString& name);
	~Line() override;

	void setPrefix(const QString&);
	const QString& prefix() const;
	void setCreateXmlElement(bool);
	void init(const KConfigGroup&);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfigGroup&);
	void loadThemeConfig(const KConfigGroup&, const QColor&);
	void saveThemeConfig(KConfigGroup&) const;

	// histogram specific parameters
	BASIC_D_ACCESSOR_DECL(bool, histogramLineTypeAvailable, HistogramLineTypeAvailable)
	BASIC_D_ACCESSOR_DECL(Histogram::LineType, histogramLineType, HistogramLineType)

	// drop line specific parameters for XYCurve
	BASIC_D_ACCESSOR_DECL(XYCurve::DropLineType, dropLineType, DropLineType)

	// common parameters
	QPen pen() const;
	BASIC_D_ACCESSOR_DECL(Qt::PenStyle, style, Style)
	CLASS_D_ACCESSOR_DECL(QColor, color, Color)
	BASIC_D_ACCESSOR_DECL(double, width, Width)
	BASIC_D_ACCESSOR_DECL(double, opacity, Opacity)

	typedef LinePrivate Private;

protected:
	LinePrivate* const d_ptr;

private:
	Q_DECLARE_PRIVATE(Line)

Q_SIGNALS:
	void histogramLineTypeChanged(Histogram::LineType);
	void dropLineTypeChanged(XYCurve::DropLineType);

	void styleChanged(Qt::PenStyle);
	void widthChanged(double);
	void colorChanged(const QColor&);
	void opacityChanged(float);

	void updateRequested();
	void updatePixmapRequested();
};

#endif

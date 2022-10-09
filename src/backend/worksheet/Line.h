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

class LinePrivate;
class KConfigGroup;

class Line : public AbstractAspect {
	Q_OBJECT

public:
	explicit Line(const QString& name);
	~Line() override;

	void setPrefix(const QString&);
	void init(const KConfigGroup&);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfigGroup&, const QColor&);
	void saveThemeConfig(KConfigGroup&) const;

	BASIC_D_ACCESSOR_DECL(bool, histogramLineTypeAvailable, HistogramLineTypeAvailable)
	BASIC_D_ACCESSOR_DECL(Histogram::LineType, histogramLineType, HistogramLineType)
	CLASS_D_ACCESSOR_DECL(QPen, pen, Pen)
	BASIC_D_ACCESSOR_DECL(qreal, opacity, Opacity)

	typedef LinePrivate Private;

protected:
	LinePrivate* const d_ptr;

private:
	Q_DECLARE_PRIVATE(Line)

Q_SIGNALS:
	void histogramLineTypeChanged(Histogram::LineType);
	void penChanged(QPen&);
	void opacityChanged(float);

	void updateRequested();
	void updatePixmapRequested();
};

#endif

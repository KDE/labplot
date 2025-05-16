/*
	File                 : CustomPoint.h
	Project              : LabPlot
	Description          : Custom user-defined point on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CUSTOMPOINT_H
#define CUSTOMPOINT_H

#include <QPen>

#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"

class CartesianPlot;
class CustomPointPrivate;
class Symbol;
class QBrush;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT CustomPoint : public WorksheetElement {
#else
class CustomPoint : public WorksheetElement {
#endif
	Q_OBJECT

public:
	explicit CustomPoint(CartesianPlot*, const QString&, bool loading = false);
	~CustomPoint() override;

	QIcon icon() const override;
	virtual QMenu* createContextMenu() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	Symbol* symbol() const;

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	static QString xmlName();

	typedef CustomPointPrivate Private;

protected:
	CustomPoint(const QString& name, CustomPointPrivate* dd);

private:
	Q_DECLARE_PRIVATE(CustomPoint)
	void init(bool loading);

Q_SIGNALS:
	friend class CustomPointSetPositionCmd;
};

#endif

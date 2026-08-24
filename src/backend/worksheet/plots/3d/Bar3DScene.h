/***************************************************************************
	File                 : Bar3DScene.h
	Project              : LabPlot
	Description          : 3D Bar Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef BAR3DSCENE_H
#define BAR3DSCENE_H

#include "Base3DPlot.h"
#include <QtGraphsWidgets/Q3DBarsWidgetItem>

class AbstractColumn;
class Bar3DScenePrivate;

class Bar3DScene : public Base3DPlot {
	Q_OBJECT
public:
	explicit Bar3DScene(const QString& name);
	~Bar3DScene() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void init(bool transform = true);
	void finalizeAdd() override;
	void retransform() override;
	void recalc();
	BASIC_D_ACCESSOR_DECL(QVector<AbstractColumn*>, dataColumns, DataColumns)
	CLASS_D_ACCESSOR_DECL(QVector<QString>, columnPaths, ColumnPaths)
	BASIC_D_ACCESSOR_DECL(QColor, color, Color)
	void setRect(const QRectF&) override;
	void setPrevRect(const QRectF&) override;
	typedef Bar3DScenePrivate Private;

private:
	Q_DECLARE_PRIVATE(Bar3DScene)
protected:
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) override;
private Q_SLOTS:
	void columnAboutToBeRemoved(const AbstractAspect*);
Q_SIGNALS:
	void dataColumnsChanged(QVector<AbstractColumn*>);
	void colorChanged(QColor);
	void rectChanged(QRectF&);
};
#endif // BAR3DSCENE_H

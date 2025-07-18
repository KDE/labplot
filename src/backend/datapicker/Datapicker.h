/*
	File                 : Datapicker.h
	Project              : LabPlot
	Description          : Datapicker
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATAPICKER_H
#define DATAPICKER_H

#include "Vector3D.h"
#include "backend/core/AbstractPart.h"
#include "backend/worksheet/DefaultColorTheme.h"

class Spreadsheet;
class DatapickerCurve;
class DatapickerImage;
class DatapickerImageView;
class DatapickerView;

class QXmlStreamWriter;
class XmlStreamReader;
class Transform;
class QPointF;
class QVector3D;
class KConfig;

class Datapicker : public AbstractPart {
	Q_OBJECT

public:
	explicit Datapicker(const QString& name, const bool loading = false);
	~Datapicker() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QWidget* view() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	DatapickerCurve* activeCurve();
	Spreadsheet* currentSpreadsheet() const;
	DatapickerImage* image() const;
	DatapickerImageView* imageView() const;

	void setChildSelectedInView(int index, bool selected);
	void setSelectedInView(const bool);
	void addNewPoint(QPointF, AbstractAspect*);

	bool xDateTime() const;

	Vector3D mapSceneToLogical(QPointF) const;
	Vector3D mapSceneLengthToLogical(QPointF) const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	void setColorPalette(const KConfig&);
	QColor themeColorPalette(int index) const;

public Q_SLOTS:
	void childSelected(const AbstractAspect*) override;

private:
	mutable DatapickerView* m_view{nullptr};
	DatapickerCurve* m_activeCurve{nullptr};
	Transform* m_transform;
	DatapickerImage* m_image{nullptr};
	QList<QColor> m_themeColorPalette = defaultColorPalette;

	void init();
	void handleChildAspectAboutToBeRemoved(const AbstractAspect*);
	void handleChildAspectAdded(const AbstractAspect*);

private Q_SLOTS:
	void childDeselected(const AbstractAspect*) override;
	void handleAspectAdded(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);

Q_SIGNALS:
	void datapickerItemSelected(int);
	void requestUpdateActions();
};

#endif

/*
    File                 : Datapicker.h
    Project              : LabPlot
    Description          : Datapicker
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre (wagadre.ankit@gmail.com)
    SPDX-FileCopyrightText: 2015-2017 Alexander Semke (alexander.semke@web.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATAPICKER_H
#define DATAPICKER_H

#include "backend/core/AbstractPart.h"

class Spreadsheet;
class DatapickerCurve;
class DatapickerImage;
class DatapickerView;

class QXmlStreamWriter;
class XmlStreamReader;
class Transform;
class QPointF;
class QVector3D;

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

	void setChildSelectedInView(int index, bool selected);
	void setSelectedInView(const bool);
	void addNewPoint(QPointF, AbstractAspect*);

	QVector3D mapSceneToLogical(QPointF) const;
	QVector3D mapSceneLengthToLogical(QPointF) const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

public slots:
	void childSelected(const AbstractAspect*) override;

private:
	mutable DatapickerView* m_view{nullptr};
	DatapickerCurve* m_activeCurve{nullptr};
	Transform* m_transform;
	DatapickerImage* m_image{nullptr};
	void init();
	void handleChildAspectAboutToBeRemoved(const AbstractAspect*);
	void handleChildAspectAdded(const AbstractAspect*);

private slots:
	void childDeselected(const AbstractAspect*) override;
	void handleAspectAdded(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);

signals:
	void datapickerItemSelected(int);
	void requestUpdateActions();
};

#endif

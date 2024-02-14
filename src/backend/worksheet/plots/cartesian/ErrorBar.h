/*
	File                 : ErrorBar.h
	Project              : LabPlot
	Description          : ErrorBar
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ERRORBAR_H
#define ERRORBAR_H

#include "backend/core/AbstractAspect.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"

class AbstractColumn;
class CartesianCoordinateSystem;
class ErrorBarPrivate;
class ErrorBarStyle;
class KConfigGroup;

class ErrorBar : public AbstractAspect {
	Q_OBJECT

public:
	friend class ErrorBarSetPlusColumnCmd;
	friend class ErrorBarSetMinusColumnCmd;
	enum class Type { NoError, Symmetric, Asymmetric, Poisson };
	enum class BarsType { Simple, WithEnds };

	explicit ErrorBar(const QString& name);
	~ErrorBar() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	void setPrefix(const QString&);
	const QString& prefix() const;
	void init(const KConfigGroup&);
	void update();
	QPainterPath painterPath(const ErrorBarStyle*, const QVector<QPointF>&, const CartesianCoordinateSystem*, WorksheetElement::Orientation) const;

	BASIC_D_ACCESSOR_DECL(Type, type, Type)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, plusColumn, PlusColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, minusColumn, MinusColumn)
	CLASS_D_ACCESSOR_DECL(QString, plusColumnPath, PlusColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, minusColumnPath, MinusColumnPath)

	typedef ErrorBarPrivate Private;

protected:
	ErrorBarPrivate* const d_ptr;

private:
	Q_DECLARE_PRIVATE(ErrorBar)
	void connectPlusColumn(const AbstractColumn*);
	void connectMinusColumn(const AbstractColumn*);

private Q_SLOTS:
	void plusColumnAboutToBeRemoved(const AbstractAspect*);
	void minusColumnAboutToBeRemoved(const AbstractAspect*);

Q_SIGNALS:
	void updateRequested();

	void typeChanged(ErrorBar::Type);
	void plusDataChanged();
	void plusColumnChanged(const AbstractColumn*);
	void minusDataChanged();
	void minusColumnChanged(const AbstractColumn*);
};

#endif

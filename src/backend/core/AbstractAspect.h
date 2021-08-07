/***************************************************************************
    File                 : AbstractAspect.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 by Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2007-2010 by Knut Franke (knut.franke@gmx.de)
    Copyright            : (C) 2011-2015 by Alexander Semke (alexander.semke@web.de)
    Description          : Base class for all objects in a Project.

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#ifndef ABSTRACT_ASPECT_H
#define ABSTRACT_ASPECT_H

#include <QObject>
#include <QVector>

class AbstractAspectPrivate;
class Folder;
class Project;
class XmlStreamReader;

class QDateTime;
class QIcon;
class QMenu;
class QUndoCommand;
class QUndoStack;
class QXmlStreamWriter;

/// Information about class inheritance
/// enum values are chosen such that @verbatim inherits(base)@endverbatim
/// returns true iff the class inherits from @verbatim base@endverbatim.
///
/// AspectType is used in GuiObserver to select the correct dock widget.
enum class AspectType : quint64 {
	AbstractAspect = 0,

	// classes without inheriters
	AbstractFilter = 0x0100001,
	DatapickerCurve = 0x0100002,
	DatapickerPoint = 0x0100004,

	WorksheetElement = 0x0200000,
		Axis = 0x0210001,
		CartesianPlotLegend = 0x0210002,
		CustomPoint = 0x0210004,
		Histogram = 0x0210008,
		PlotArea = 0x0210010,
		TextLabel = 0x0210020,
		Image = 0x0210030,
		ReferenceLine = 0x0210040,
		InfoElement = 0x0210080,
		WorksheetElementContainer = 0x0220000,
			AbstractPlot = 0x0221000,
				CartesianPlot = 0x0221001,
			WorksheetElementGroup = 0x0222000,
		XYCurve = 0x0240000,
			XYEquationCurve = 0x0240001,
		XYAnalysisCurve = 0x0280000,
			XYConvolutionCurve = 0x0280001,
			XYCorrelationCurve = 0x0280002,
			XYDataReductionCurve = 0x0280004,
			XYDifferentiationCurve = 0x0280008,
			XYFitCurve = 0x0280010,
			XYFourierFilterCurve = 0x0280020,
			XYFourierTransformCurve = 0x0280040,
			XYInterpolationCurve = 0x0280080,
			XYIntegrationCurve = 0x0280100,
			XYSmoothCurve = 0x0280200,
			XYHilbertTransformCurve = 0x0280400,
		BoxPlot = 0x0250000,

	AbstractPart = 0x0400000,
		AbstractDataSource = 0x0410000,
			Matrix = 0x0411000,
			Spreadsheet = 0x0412000,
				LiveDataSource = 0x0412001,
				MQTTTopic = 0x0412002,
		CantorWorksheet = 0x0420001,
		Datapicker = 0x0420002,
		DatapickerImage = 0x0420004,
		Note = 0x0420008,
		Workbook = 0x0420010,
		Worksheet = 0x0420020,

	AbstractColumn = 0x1000000,
		Column = 0x1000001,
		SimpleFilterColumn = 0x1000002,
		ColumnStringIO = 0x1000004,

	Folder = 0x2000000,
		Project = 0x2000001,
		MQTTClient = 0x2000002,
		MQTTSubscription = 0x2000004,
};

class AbstractAspect : public QObject {
	Q_OBJECT

public:
	enum class ChildIndexFlag {
		IncludeHidden = 0x01,
		Recursive = 0x02,
		Compress = 0x04
	};

	Q_DECLARE_FLAGS(ChildIndexFlags, ChildIndexFlag)

	friend class AspectChildAddCmd;
	friend class AspectChildRemoveCmd;
	friend class AbstractAspectPrivate;

	AbstractAspect(const QString& name, AspectType type);
	~AbstractAspect() override;

	// type name for internal use (no translation)
	static QString typeName(AspectType type) {
		switch(type) {
		case AspectType::AbstractAspect:
			return QString("AbstractAspect");
		case AspectType::AbstractFilter:
			return QString("AbstractFilter");
		case AspectType::DatapickerCurve:
			return QString("DatapickerCurve");
		case AspectType::DatapickerPoint:
			return QString("DatapickerPoint");
		case AspectType::WorksheetElement:
			return QString("WorksheetElement");
		case AspectType::Axis:
			return QString("Axis");
		case AspectType::CartesianPlotLegend:
			return QString("CartesianPlotLegend");
		case AspectType::CustomPoint:
			return QString("CustomPoint");
		case AspectType::Histogram:
			return QString("Histogram");
		case AspectType::PlotArea:
			return QString("PlotArea");
		case AspectType::TextLabel:
			return QString("TextLabel");
		case AspectType::Image:
			return QString("Image");
		case AspectType::ReferenceLine:
			return QString("ReferenceLine");
		case AspectType::InfoElement:
			return QString("InfoElement");
		case AspectType::WorksheetElementContainer:
			return QString("WorksheetElementContainer");
		case AspectType::AbstractPlot:
			return QString("AbstractPlot");
		case AspectType::CartesianPlot:
			return QString("CartesianPlot");
		case AspectType::WorksheetElementGroup:
			return QString("WorksheetElementGroup");
		case AspectType::XYCurve:
			return QString("XYCurve");
		case AspectType::XYEquationCurve:
			return QString("XYEquationCurve");
		case AspectType::XYAnalysisCurve:
			return QString("XYAnalysisCurve");
		case AspectType::XYConvolutionCurve:
			return QString("XYConvolutionCurve");
		case AspectType::XYCorrelationCurve:
			return QString("XYCorrelationCurve");
		case AspectType::XYDataReductionCurve:
			return QString("XYDataReductionCurve");
		case AspectType::XYDifferentiationCurve:
			return QString("XYDifferentiationCurve");
		case AspectType::XYFitCurve:
			return QString("XYFitCurve");
		case AspectType::XYFourierFilterCurve:
			return QString("XYFourierFilterCurve");
		case AspectType::XYFourierTransformCurve:
			return QString("XYFourierTransformCurve");
		case AspectType::XYInterpolationCurve:
			return QString("XYInterpolationCurve");
		case AspectType::XYIntegrationCurve:
			return QString("XYIntegrationCurve");
		case AspectType::XYSmoothCurve:
			return QString("XYSmoothCurve");
		case AspectType::XYHilbertTransformCurve:
			return QString("XYHilbertTransformCurve");
		case AspectType::BoxPlot:
			return QString("BoxPlot");
		case AspectType::AbstractPart:
			return QString("AbstractPart");
		case AspectType::AbstractDataSource:
			return QString("AbstractDataSource");
		case AspectType::Matrix:
			return QString("Matrix");
		case AspectType::Spreadsheet:
			return QString("Spreadsheet");
		case AspectType::LiveDataSource:
			return QString("LiveDataSource");
		case AspectType::MQTTTopic:
			return QString("MQTTTopic");
		case AspectType::CantorWorksheet:
			return QString("CantorWorksheet");
		case AspectType::Datapicker:
			return QString("Datapicker");
		case AspectType::DatapickerImage:
			return QString("DatapickerImage");
		case AspectType::Note:
			return QString("Note");
		case AspectType::Workbook:
			return QString("Workbook");
		case AspectType::Worksheet:
			return QString("Worksheet");
		case AspectType::AbstractColumn:
			return QString("AbstractColumn");
		case AspectType::Column:
			return QString("Column");
		case AspectType::SimpleFilterColumn:
			return QString("SimpleFilterColumn");
		case AspectType::ColumnStringIO:
			return QString("ColumnStringIO");
		case AspectType::Folder:
			return QString("Folder");
		case AspectType::Project:
			return QString("Project");
		case AspectType::MQTTClient:
			return QString("MQTTClient");
		case AspectType::MQTTSubscription:
			return QString("MQTTSubscription");
		}

		return QString();
	}

	QString name() const;
	QString comment() const;
	void setCreationTime(const QDateTime&);
	QDateTime creationTime() const;
	virtual Project* project();
	virtual QString path() const;
	void setHidden(bool);
	bool hidden() const;
	void setSelected(bool);
	void setIsLoading(bool);
	bool isLoading() const;
	virtual QIcon icon() const;
	virtual QMenu* createContextMenu();

	AspectType type() const;
	bool inherits(AspectType type) const;

	//functions related to the handling of the tree-like project structure
	AbstractAspect* parentAspect() const;
	AbstractAspect* parent(AspectType type) const;
	void setParentAspect(AbstractAspect*);
	Folder* folder();
	bool isDescendantOf(AbstractAspect* other);
	void addChild(AbstractAspect*);
	void addChildFast(AbstractAspect*);
	virtual void finalizeAdd() {};
	QVector<AbstractAspect*> children(AspectType type, ChildIndexFlags flags = {}) const;
	void insertChildBefore(AbstractAspect* child, AbstractAspect* before);
	void insertChildBeforeFast(AbstractAspect* child, AbstractAspect* before);
	void reparent(AbstractAspect* newParent, int newIndex = -1);
	void removeChild(AbstractAspect*);
	void removeAllChildren();
	virtual QVector<AbstractAspect*> dependsOn() const;

	virtual QVector<AspectType> pasteTypes() const;
	virtual bool isDraggable() const;
	virtual QVector<AspectType> dropableOn() const;
	virtual void processDropEvent(const QVector<quintptr>&) {};

	template <class T> T* ancestor() const {
		AbstractAspect* parent = parentAspect();
		while (parent) {
			T* ancestorAspect = dynamic_cast<T*>(parent);
			if (ancestorAspect)
				return ancestorAspect;
			parent = parent->parentAspect();
		}
		return nullptr;
	}

	template <class T> QVector<T*> children(ChildIndexFlags flags = {}) const {
		QVector<T*> result;
		for (auto* child: children()) {
			if (flags & ChildIndexFlag::IncludeHidden || !child->hidden()) {
				T* i = dynamic_cast<T*>(child);
				if (i)
					result << i;

				if (child && flags & ChildIndexFlag::Recursive)
					result << child->template children<T>(flags);
			}
		}
		return result;
	}

	template <class T> T* child(int index, ChildIndexFlags flags = {}) const {
		int i = 0;
		for (auto* child: children()) {
			T* c = dynamic_cast<T*>(child);
			if (c && (flags & ChildIndexFlag::IncludeHidden || !child->hidden()) && index == i++)
				return c;
		}
		return nullptr;
	}

	template <class T> T* child(const QString& name) const {
		for (auto* child: children()) {
			T* c = dynamic_cast<T*>(child);
			if (c && child->name() == name)
				return c;
		}
		return nullptr;
	}

	template <class T> int childCount(ChildIndexFlags flags = {}) const {
		int result = 0;
		for (auto* child: children()) {
			T* i = dynamic_cast<T*>(child);
			if (i && (flags & ChildIndexFlag::IncludeHidden || !child->hidden()))
				result++;
		}
		return result;
	}

	template <class T> int indexOfChild(const AbstractAspect* child, ChildIndexFlags flags = {}) const {
		int index = 0;
		for (auto* c:	 children()) {
			if (child == c) return index;
			T* i = dynamic_cast<T*>(c);
			if (i && (flags & ChildIndexFlag::IncludeHidden || !c->hidden()))
				index++;
		}
		return -1;
	}

	//undo/redo related functions
	void setUndoAware(bool);
	virtual QUndoStack* undoStack() const;
	void exec(QUndoCommand*);
	void exec(QUndoCommand* command, const char* preChangeSignal, const char* postChangeSignal,
		QGenericArgument val0 = QGenericArgument(), QGenericArgument val1 = QGenericArgument(),
		QGenericArgument val2 = QGenericArgument(), QGenericArgument val3 = QGenericArgument());
	void beginMacro(const QString& text);
	void endMacro();

	//save/load
	virtual void save(QXmlStreamWriter*) const = 0;
	virtual bool load(XmlStreamReader*, bool preview) = 0;

	static AspectType clipboardAspectType(QString&);

protected:
	void info(const QString& text) { emit statusInfo(text); }

	//serialization/deserialization
	bool readBasicAttributes(XmlStreamReader*);
	void writeBasicAttributes(QXmlStreamWriter*) const;
	void writeCommentElement(QXmlStreamWriter*) const;
	bool readCommentElement(XmlStreamReader*);

	const AspectType m_type;

private:
	AbstractAspectPrivate* d;

	QString uniqueNameFor(const QString&) const;
	const QVector<AbstractAspect*>& children() const;
	void connectChild(AbstractAspect*);

public slots:
	bool setName(const QString&, bool autoUnique = true);
	void setComment(const QString&);
	void remove();
	void copy() const;
	void duplicate();
	void paste(bool duplicate = false);

protected slots:
	virtual void childSelected(const AbstractAspect*);
	virtual void childDeselected(const AbstractAspect*);

signals:
	void aspectDescriptionAboutToChange(const AbstractAspect*);
	void aspectDescriptionChanged(const AbstractAspect*);
	void aspectAboutToBeAdded(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);
	void aspectAdded(const AbstractAspect*);
	void aspectAboutToBeRemoved(const AbstractAspect*);
	void aspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);
	void aspectHiddenAboutToChange(const AbstractAspect*);
	void aspectHiddenChanged(const AbstractAspect*);
	void statusInfo(const QString&);
	void renameRequested();

	//selection/deselection in model (project explorer)
	void selected(const AbstractAspect*);
	void deselected(const AbstractAspect*);

	//selection/deselection in view
	void childAspectSelectedInView(const AbstractAspect*);
	void childAspectDeselectedInView(const AbstractAspect*);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractAspect::ChildIndexFlags)

#endif // ifndef ABSTRACT_ASPECT_H

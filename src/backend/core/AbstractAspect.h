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
class QDropEvent;
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
		WorksheetElementContainer = 0x0220000,
			AbstractPlot = 0x0221000,
				CartesianPlot = 0x0221001,
			WorksheetElementGroup = 0x0222000,
		XYCurve = 0x0240000,
			XYEquationCurve = 0x0240001,
		XYAnalysisCurve = 0x0280000,
			XYConvolution = 0x0280001,
			XYCorrelationCurve = 0x0280002,
			XYDataReductionCurve = 0x0280004,
			XYDifferentiationCurve = 0x0280008,
			XYFitCurve = 0x0280010,
			XYFourierFilterCurve = 0x0280020,
			XYFourierTransformCurve = 0x0280040,
			XYInterpolationCurve = 0x0280080,
			XYIntegrationCurve = 0x0280100,
			XYSmoothCurve = 0x0280200,

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
	enum ChildIndexFlag {
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
	QVector<AbstractAspect*> children(AspectType type, ChildIndexFlags flags=nullptr);
	void insertChildBefore(AbstractAspect* child, AbstractAspect* before);
	void insertChildBeforeFast(AbstractAspect* child, AbstractAspect* before);
	void reparent(AbstractAspect* newParent, int newIndex = -1);
	void removeChild(AbstractAspect*);
	void removeAllChildren();
	virtual QVector<AbstractAspect*> dependsOn() const;

	virtual bool isDraggable() const;
	virtual QVector<AspectType> dropableOn() const;
	virtual void processDropEvent(QDropEvent*) {};

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

	template <class T> QVector<T*> children(ChildIndexFlags flags = nullptr) const {
		QVector<T*> result;
		for (auto* child: children()) {
			if (flags & IncludeHidden || !child->hidden()) {
				T* i = dynamic_cast<T*>(child);
				if (i)
					result << i;

				if (flags & Recursive)
					result << child->template children<T>(flags);
			}
		}
		return result;
	}

	template <class T> T* child(int index, ChildIndexFlags flags=nullptr) const {
		int i = 0;
		for (auto* child: children()) {
			T* c = dynamic_cast<T*>(child);
			if (c && (flags & IncludeHidden || !child->hidden()) && index == i++)
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

	template <class T> int childCount(ChildIndexFlags flags = nullptr) const {
		int result = 0;
		for (auto* child: children()) {
			T* i = dynamic_cast<T*>(child);
			if (i && (flags & IncludeHidden || !child->hidden()))
				result++;
		}
		return result;
	}

	template <class T> int indexOfChild(const AbstractAspect* child, ChildIndexFlags flags = nullptr) const {
		int index = 0;
		for (auto* c:	 children()) {
			if (child == c) return index;
			T* i = dynamic_cast<T*>(c);
			if (i && (flags & IncludeHidden || !c->hidden()))
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
	const QVector<AbstractAspect*> children() const;
	void connectChild(AbstractAspect*);

public slots:
	bool setName(const QString&, bool autoUnique = true);
	void setComment(const QString&);
	void remove();

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

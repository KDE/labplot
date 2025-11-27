/*
	File                 : AbstractAspect.h
	Project              : LabPlot
	Description          : Base class for all objects in a Project.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007-2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2007-2010 Knut Franke <knut.franke@gmx.de>
	SPDX-FileCopyrightText: 2011-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

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
/// returns true if the class inherits from @verbatim base@endverbatim.
///
/// AspectType is used in GuiObserver to select the correct dock widget.
///
/// IMPORTANT: Never expose the type value outside of the running application,
/// because it can change with every labplot version!
enum class AspectType : quint64 {
	AbstractAspect,

	// classes without inheriters
	AbstractFilter,
	DatapickerCurve,
	DatapickerPoint,

	WorksheetElement,
	Axis,
	CartesianPlotLegend,
	CustomPoint,
	PlotArea,
	TextLabel,
	Image,
	ReferenceLine,
	ReferenceRange,
	InfoElement,

	// bar plots
	BarPlot,
	LollipopPlot,

	// statistical plots
	Histogram,
	BoxPlot,
	QQPlot,
	KDEPlot,
	
	Heatmap,

	HypothesisTest,

	// continuous improvement plots
	ProcessBehaviorChart,
	RunChart,

	WorksheetElementContainer,
	AbstractPlot,
	CartesianPlot,
	WorksheetElementGroup,
	XYCurve,
	XYEquationCurve,

	// analysis curves
	XYAnalysisCurve,
	XYConvolutionCurve,
	XYCorrelationCurve,
	XYDataReductionCurve,
	XYDifferentiationCurve,
	XYFitCurve,
	XYFourierFilterCurve,
	XYFourierTransformCurve,
	XYInterpolationCurve,
	XYIntegrationCurve,
	XYSmoothCurve,
	XYHilbertTransformCurve,
	XYFunctionCurve,

	AbstractPart,
	AbstractDataSource,
	Matrix,
	Spreadsheet,
	LiveDataSource,
	MQTTTopic,
	StatisticsSpreadsheet,
	Notebook,
	Datapicker,
	DatapickerImage,
	Note,
	Workbook,
	Worksheet,
	Script,

	AbstractColumn,
	Column,
	SimpleFilterColumn,
	ColumnStringIO,

	Folder,
	Project,
	MQTTClient,
	MQTTSubscription,
};

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT AbstractAspect : public QObject {
#else
class AbstractAspect : public QObject {
#endif
	Q_OBJECT

public:
	enum class ChildIndexFlag { IncludeHidden = 0x01, Recursive = 0x02, Compress = 0x04 };

	Q_DECLARE_FLAGS(ChildIndexFlags, ChildIndexFlag)

	friend class AspectChildAddCmd;
	friend class AspectChildRemoveCmd;
	friend class AbstractAspectPrivate;

	AbstractAspect(const QString& name, AspectType type);
	~AbstractAspect() override;

	enum class NameHandling {
		AutoUnique, // Set the name and make it unique (enforce the uniqueness)
		UniqueNotRequired, // Set the name without making it unique
		UniqueRequired, // Set the name only if it's already unique
	};

	// type name for internal use (no translation)
	// IMPORTANT: This name must match the class name!
	// It will be used to use the QObject::inherits() function
	static constexpr std::string_view typeName(AspectType type) {
		switch (type) {
		case AspectType::AbstractAspect:
			return std::string_view("AbstractAspect");
		case AspectType::AbstractFilter:
			return std::string_view("AbstractFilter");
		case AspectType::DatapickerCurve:
			return std::string_view("DatapickerCurve");
		case AspectType::DatapickerPoint:
			return std::string_view("DatapickerPoint");
		case AspectType::WorksheetElement:
			return std::string_view("WorksheetElement");
		case AspectType::Axis:
			return std::string_view("Axis");
		case AspectType::CartesianPlotLegend:
			return std::string_view("CartesianPlotLegend");
		case AspectType::CustomPoint:
			return std::string_view("CustomPoint");
		case AspectType::Histogram:
			return std::string_view("Histogram");
		case AspectType::PlotArea:
			return std::string_view("PlotArea");
		case AspectType::TextLabel:
			return std::string_view("TextLabel");
		case AspectType::Image:
			return std::string_view("Image");
		case AspectType::ReferenceLine:
			return std::string_view("ReferenceLine");
		case AspectType::ReferenceRange:
			return std::string_view("ReferenceRange");
		case AspectType::InfoElement:
			return std::string_view("InfoElement");
		case AspectType::WorksheetElementContainer:
			return std::string_view("WorksheetElementContainer");
		case AspectType::AbstractPlot:
			return std::string_view("AbstractPlot");
		case AspectType::CartesianPlot:
			return std::string_view("CartesianPlot");
		case AspectType::WorksheetElementGroup:
			return std::string_view("WorksheetElementGroup");
		case AspectType::XYCurve:
			return std::string_view("XYCurve");
		case AspectType::XYEquationCurve:
			return std::string_view("XYEquationCurve");
		case AspectType::XYFunctionCurve:
			return std::string_view("XYFunctionCurve");
		case AspectType::XYAnalysisCurve:
			return std::string_view("XYAnalysisCurve");
		case AspectType::XYConvolutionCurve:
			return std::string_view("XYConvolutionCurve");
		case AspectType::XYCorrelationCurve:
			return std::string_view("XYCorrelationCurve");
		case AspectType::XYDataReductionCurve:
			return std::string_view("XYDataReductionCurve");
		case AspectType::XYDifferentiationCurve:
			return std::string_view("XYDifferentiationCurve");
		case AspectType::XYFitCurve:
			return std::string_view("XYFitCurve");
		case AspectType::XYFourierFilterCurve:
			return std::string_view("XYFourierFilterCurve");
		case AspectType::XYFourierTransformCurve:
			return std::string_view("XYFourierTransformCurve");
		case AspectType::XYInterpolationCurve:
			return std::string_view("XYInterpolationCurve");
		case AspectType::XYIntegrationCurve:
			return std::string_view("XYIntegrationCurve");
		case AspectType::XYSmoothCurve:
			return std::string_view("XYSmoothCurve");
		case AspectType::XYHilbertTransformCurve:
			return std::string_view("XYHilbertTransformCurve");
		case AspectType::BarPlot:
			return std::string_view("BarPlot");
		case AspectType::BoxPlot:
			return std::string_view("BoxPlot");
		case AspectType::QQPlot:
			return std::string_view("QQPlot");
		case AspectType::KDEPlot:
			return std::string_view("KDEPlot");
		case AspectType::Heatmap:
			return std::string_view("Heatmap");
		case AspectType::LollipopPlot:
			return std::string_view("LollipopPlot");
		case AspectType::ProcessBehaviorChart:
			return std::string_view("ProcessBehaviorChart");
		case AspectType::RunChart:
			return std::string_view("RunChart");
		case AspectType::AbstractPart:
			return std::string_view("AbstractPart");
		case AspectType::AbstractDataSource:
			return std::string_view("AbstractDataSource");
		case AspectType::Matrix:
			return std::string_view("Matrix");
		case AspectType::Spreadsheet:
			return std::string_view("Spreadsheet");
		case AspectType::StatisticsSpreadsheet:
			return std::string_view("StatisticsSpreadsheet");
		case AspectType::LiveDataSource:
			return std::string_view("LiveDataSource");
		case AspectType::MQTTTopic:
			return std::string_view("MQTTTopic");
		case AspectType::Notebook:
			return std::string_view("Notebook");
		case AspectType::Datapicker:
			return std::string_view("Datapicker");
		case AspectType::DatapickerImage:
			return std::string_view("DatapickerImage");
		case AspectType::Note:
			return std::string_view("Note");
		case AspectType::Workbook:
			return std::string_view("Workbook");
		case AspectType::Worksheet:
			return std::string_view("Worksheet");
		case AspectType::Script:
			return std::string_view("Script");
		case AspectType::AbstractColumn:
			return std::string_view("AbstractColumn");
		case AspectType::Column:
			return std::string_view("Column");
		case AspectType::SimpleFilterColumn:
			return std::string_view("SimpleFilterColumn");
		case AspectType::ColumnStringIO:
			return std::string_view("ColumnStringIO");
		case AspectType::Folder:
			return std::string_view("Folder");
		case AspectType::Project:
			return std::string_view("Project");
		case AspectType::MQTTClient:
			return std::string_view("MQTTClient");
		case AspectType::MQTTSubscription:
			return std::string_view("MQTTSubscription");
		case AspectType::HypothesisTest:
			return std::string_view("HypothesisTest");
			break;
		}

		return {};
	}

	QString name() const;
	QUuid uuid() const;
	void setSuppressWriteUuid(bool);
	QString comment() const;
	void setCreationTime(const QDateTime&);
	QDateTime creationTime() const;
	virtual Project* project();
	virtual const Project* project() const;
	virtual QString path() const;
	void setHidden(bool);
	bool isHidden() const;
	void setFixed(bool);
	bool isFixed() const;
	void setSelected(bool);
	void setMoved(bool);
	bool isMoved() const;
	void setIsLoading(bool);
	bool isLoading() const;
	virtual QIcon icon() const;
	virtual QMenu* createContextMenu();
	void setProjectChanged(bool);

	AspectType type() const;
	using QObject::inherits;
	/*!
	 * \brief inherits
	 * Checks if the aspect inherits from Target
	 * \return true if inherits from Target, otherwise false
	 */
	template<typename Target>
	bool inherits() const {
		return (dynamic_cast<const Target*>(this) != nullptr);
	}
	/*!
	 * \brief castTo
	 * \return a pointer to the casted Target if this aspect
	 * derives from Target otherwise a nullptr
	 */
	template<typename Target>
	Target* castTo() {
		return dynamic_cast<Target*>(this);
	}

	/*!
	 * \brief castTo (const variant)
	 * \return a pointer to the casted Target if this aspect
	 * derives from Target otherwise a nullptr
	 */
	template<typename Target>
	Target* castTo() const {
		return dynamic_cast<Target*>(this);
	}

	/*!
	 * \brief parent
	 * In the parent-child hierarchy, return the first parent of type \param Target or null pointer if there is none.
	 * \return
	 */
	template<typename Target>
	Target* parent() {
		AbstractAspect* parent = parentAspect();
		if (!parent)
			return nullptr;

		if (auto* p = parent->castTo<Target>())
			return p;

		return parent->parent<Target>();
	}

	// functions related to the handling of the tree-like project structure
	AbstractAspect* parentAspect() const;
	void setParentAspect(AbstractAspect*);
	Folder* folder();
	bool isDescendantOf(AbstractAspect* other);
	virtual bool addChild(AbstractAspect*);
	void addChildFast(AbstractAspect*);
	virtual void finalizeAdd() { };
	QVector<AbstractAspect*> children(AspectType, ChildIndexFlags = {}) const;
	void insertChild(AbstractAspect* child, int index);
	void insertChildBefore(AbstractAspect* child, AbstractAspect* before);
	void insertChildBeforeFast(AbstractAspect* child, AbstractAspect* before);
	void reparent(AbstractAspect* newParent, int newIndex = -1);
	/*!
	 * \brief removeChild
	 * Removing child aspect using an undo command
	 * \param parent If parent is not nullptr the command will not be executed, but the parent must be executed
	 * to indirectly execute the created undocommand
	 */
	void removeChild(AbstractAspect*);
	void removeAllChildren();
	void moveChild(AbstractAspect*, int steps);
	virtual QVector<AbstractAspect*> dependsOn() const;

	virtual QVector<AspectType> pasteTypes() const;
	virtual bool isDraggable() const;
	virtual QVector<AspectType> dropableOn() const;
	virtual void processDropEvent(const QVector<quintptr>&) { };

	template<class T>
	T* ancestor() const {
		AbstractAspect* parent = parentAspect();
		while (parent) {
			T* ancestorAspect = dynamic_cast<T*>(parent);
			if (ancestorAspect)
				return ancestorAspect;
			parent = parent->parentAspect();
		}
		return nullptr;
	}

	template<class Target>
	QVector<Target*> children(ChildIndexFlags flags = {}) const {
		QVector<Target*> result;
		for (auto* child : children()) {
			if (flags & ChildIndexFlag::IncludeHidden || !child->isHidden()) {
				auto* i = dynamic_cast<Target*>(child);
				if (i)
					result << i;

				if (child && flags & ChildIndexFlag::Recursive)
					result << child->template children<Target>(flags);
			}
		}
		return result;
	}

	template<class T>
	T* child(int index, ChildIndexFlags flags = {}) const {
		int i = 0;
		for (auto* child : children()) {
			T* c = dynamic_cast<T*>(child);
			if (c && (flags & ChildIndexFlag::IncludeHidden || !child->isHidden()) && index == i++)
				return c;
		}
		return nullptr;
	}

	template<class T>
	T* child(const QString& name) const {
		for (auto* child : children()) {
			T* c = dynamic_cast<T*>(child);
			if (c && child->name() == name)
				return c;
		}
		return nullptr;
	}

	AbstractAspect* child(const QString& name, AspectType type) const {
		for (auto* child : children()) {
			if (child->type() == type && child->name() == name)
				return child;
		}
		return nullptr;
	}

	template<class T>
	int childCount(ChildIndexFlags flags = {}) const {
		int result = 0;
		for (auto* child : children()) {
			T* i = dynamic_cast<T*>(child);
			if (i && (flags & ChildIndexFlag::IncludeHidden || !child->isHidden()))
				result++;
		}
		return result;
	}

	template<class T>
	int indexOfChild(const AbstractAspect* child, ChildIndexFlags flags = {}) const {
		int index = 0;
		for (auto* c : children()) {
			if (child == c)
				return index;
			T* i = dynamic_cast<T*>(c);
			if (i && (flags & ChildIndexFlag::IncludeHidden || !c->isHidden()))
				index++;
		}
		return -1;
	}

	// undo/redo related functions
	void setUndoAware(bool value);
	bool isUndoAware() const;
	virtual QUndoStack* undoStack() const;
	void exec(QUndoCommand*);
	void exec(QUndoCommand* command,
			  const char* preChangeSignal,
			  const char* postChangeSignal,
			  QGenericArgument val0 = QGenericArgument(),
			  QGenericArgument val1 = QGenericArgument(),
			  QGenericArgument val2 = QGenericArgument(),
			  QGenericArgument val3 = QGenericArgument());
	void beginMacro(const QString& text);
	void endMacro();

	// save/load
	virtual void save(QXmlStreamWriter*) const = 0;
	virtual bool load(XmlStreamReader*, bool preview) = 0;
	void setPasted(bool);
	bool isPasted() const;

	static AspectType clipboardAspectType(QString&);
	static QString uniqueNameFor(const QString& name, const QStringList& names);

	typedef AbstractAspectPrivate Private;

protected:
	void info(const QString& text) {
		Q_EMIT statusInfo(text);
	}

	// serialization/deserialization
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

public Q_SLOTS:
	bool setName(const QString&, NameHandling handling = NameHandling::AutoUnique, QUndoCommand* parent = nullptr);
	void setComment(const QString&);
	void remove();
	void copy();
	void duplicate();
	void paste(bool duplicate = false, int index = -1);

private Q_SLOTS:
	void moveUp();
	void moveDown();

protected Q_SLOTS:
	virtual void childSelected(const AbstractAspect*);
	virtual void childDeselected(const AbstractAspect*);

Q_SIGNALS:
	void aspectDescriptionAboutToChange(const AbstractAspect*);
	void aspectDescriptionChanged(const AbstractAspect*);

	void aspectCommentAboutToChange(const AbstractAspect*);
	void aspectCommentChanged(const AbstractAspect*);

	/*!
	 * \brief aspectAboutToBeAdded
	 * Signal indicating a new child was added at position \p index. Do not connect to both variants of aspectAboutToBeAdded!
	 * \param parent
	 * \param index Position of the new aspect
	 * \param child
	 */
	void childAspectAboutToBeAdded(const AbstractAspect* parent, int index, const AbstractAspect* child);
	/*!
	 * \brief aspectAboutToBeAdded
	 * \param parent
	 * \param before aspect one position before the child
	 * \param child
	 */
	void childAspectAboutToBeAdded(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);
	void childAspectAdded(const AbstractAspect*);
	/*!
	 * \brief aspectAboutToBeRemoved
	 * Called from the parent if a child is being removed
	 */
	void childAspectAboutToBeRemoved(const AbstractAspect* child);
	void childAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);
	/*!
	 * \brief aspectAboutToBeRemoved
	 * Called by the aspect itself when it's being removed
	 */
	void aspectAboutToBeRemoved(const AbstractAspect*);
	void childAspectAboutToBeMoved(const AbstractAspect*, int destinationRow);
	void childAspectMoved();
	void aspectHiddenAboutToChange(const AbstractAspect*);
	void aspectHiddenChanged(const AbstractAspect*);
	void statusInfo(const QString&);
	void statusError(const QString&);
	void renameRequested();
	void contextMenuRequested(AspectType, QMenu*);

	// selection/deselection in model (project explorer)
	void selected(const AbstractAspect*);
	void deselected(const AbstractAspect*);

	// selection/deselection in view
	void childAspectSelectedInView(const AbstractAspect*);
	void childAspectDeselectedInView(const AbstractAspect*);

	// Used by the retransformTests
Q_SIGNALS:
	void retransformCalledSignal(const AbstractAspect* sender, bool suppressed);

public:
	void resetRetransformCalled() {
		mRetransformCalled = 0;
	}
	int retransformCalled() const {
		return mRetransformCalled;
	}
	int mRetransformCalled{0};

#define trackRetransformCalled(suppressed)                                                                                                                     \
	Q_EMIT q->retransformCalledSignal(q, suppressed);                                                                                                          \
	if (!suppressed)                                                                                                                                           \
		q->mRetransformCalled += 1;

	friend class AbstractAspectTest;
	friend class InfoElementTest;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractAspect::ChildIndexFlags)

#endif // ifndef ABSTRACT_ASPECT_H

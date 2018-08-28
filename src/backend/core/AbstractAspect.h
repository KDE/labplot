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
class Project;
class QUndoStack;
class QDateTime;
class QUndoCommand;
class QIcon;
class QMenu;
class Folder;
class XmlStreamReader;
class QXmlStreamWriter;

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

	explicit AbstractAspect(const QString& name);
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

	//functions related to the handling of the tree-like project structure
	AbstractAspect* parentAspect() const;
	void setParentAspect(AbstractAspect*);
	Folder* folder();
	bool isDescendantOf(AbstractAspect* other);
	void addChild(AbstractAspect*);
	void addChildFast(AbstractAspect*);
	virtual void finalizeAdd() {};
	QVector<AbstractAspect*> children(const char* className, ChildIndexFlags flags=nullptr);
	void insertChildBefore(AbstractAspect* child, AbstractAspect* before);
	void insertChildBeforeFast(AbstractAspect* child, AbstractAspect* before);
	void reparent(AbstractAspect* newParent, int newIndex = -1);
	void removeChild(AbstractAspect*);
	void removeAllChildren();
	virtual QVector<AbstractAspect*> dependsOn() const;

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

private:
	AbstractAspectPrivate* d;

	QString uniqueNameFor(const QString&) const;
	const QVector<AbstractAspect*> children() const;
	void connectChild(AbstractAspect*);

public slots:
	void setName(const QString&);
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

/***************************************************************************
    File                 : AbstractAspect.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 by Knut Franke, Tilman Benkert
                           (C) 2010 by Knut Franke
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : Base class for all persistent objects in a Project.

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
#include <QList>
#include <QDebug>

class Project;
class QUndoStack;
class QString;
class QDateTime;
class QUndoCommand;
class QIcon;
class QMenu;
class Folder;
class XmlStreamReader;
#ifdef Q_OS_MAC32
// A hack in Qt 4.4 and later forces us to include QXmlStream* headers on MacOS instead of simple
// forward declarations. See
// http://lists.trolltech.com/qt-interest/2008-07/thread00798-0.html
#include <QXmlStreamWriter>
#else
class QXmlStreamWriter;
#endif
class QAction;

class AbstractAspect : public QObject
{
	Q_OBJECT

	public:
		enum ChildIndexFlag {
			IncludeHidden = 0x01,
			Recursive = 0x02,
			Compress = 0x04
		};
		Q_DECLARE_FLAGS(ChildIndexFlags, ChildIndexFlag)

		class Private;
		static void staticInit();

		AbstractAspect(const QString &name);
		virtual ~AbstractAspect();

		AbstractAspect * parentAspect() const;
		template < class T > T * ancestor() const{
			AbstractAspect *parent = parentAspect();
			while (parent) {
				T *ancestorAspect = qobject_cast<T *>(parent);
				if (ancestorAspect)
					return ancestorAspect;
				parent = parent->parentAspect();
			}
			return NULL;
		}
		Folder * folder();
		bool isDescendantOf(AbstractAspect *other);

		void setSelected(bool);
		
		void addChild(AbstractAspect* child);
		void insertChildBefore(AbstractAspect *child, AbstractAspect *before);
		void removeChild(AbstractAspect* child);
		QList<AbstractAspect*> children(const char* className, const ChildIndexFlags &flags=0);

		//TODO: recursive flag doesn't work! How should it work with templates?!?
		template < class T > QList<T*> children(const ChildIndexFlags &flags=0) const {
			QList<T*> result;
			foreach (AbstractAspect * child, rawChildren()) {
				if (flags & IncludeHidden || !child->hidden()) {
					T * i = qobject_cast< T* >(child);
					if (i) {
						result << i;
						if (flags & Recursive)
							result << i->children<T>(flags);
					}
				}
			}
			return result;
		}
		template < class T > T * child(int index, const ChildIndexFlags &flags=0) const {
			int i = 0;
			foreach(AbstractAspect * child, rawChildren()) {
				T * c = qobject_cast< T* >(child);
				if (c && (flags & IncludeHidden || !child->hidden()) && index == i++)
					return c;
			}
			return 0;
		}
		template < class T > T * child(const QString &name) const {
			foreach(AbstractAspect * child, rawChildren()) {
			T * c = qobject_cast< T* >(child);
			if (c && child->name() == name)
				return c;
			}
			return 0;
		}
		template < class T > int childCount(const ChildIndexFlags &flags=0) const {
			int result = 0;
			foreach(AbstractAspect * child, rawChildren()) {
				T * i = qobject_cast< T* >(child);
				if (i && (flags & IncludeHidden || !child->hidden()))
					result++;
			}
			return result;
		}
		template < class T > int indexOfChild(const AbstractAspect * child, const ChildIndexFlags &flags=0) const {
			int index = 0;
			foreach(AbstractAspect * c, rawChildren()) {
				if (child == c) return index;
				T * i = qobject_cast< T* >(c);
				if (i && (flags & IncludeHidden || !c->hidden()))
					index++;
			}
			return -1;
		}
		void reparent(AbstractAspect * new_parent, int new_index=-1);

		void removeAllChildren();

		virtual const Project *project() const { return parentAspect() ? parentAspect()->project() : 0; }
		virtual Project *project() { return parentAspect() ? parentAspect()->project() : 0; }
		virtual QString path() const { return parentAspect() ? parentAspect()->path() + "/" + name() : "";  }

		virtual QIcon icon() const;
		virtual QMenu *createContextMenu();

		QString name() const;
		QString comment() const;
		QString captionSpec() const;
		QDateTime creationTime() const;
		QString caption() const;
		bool hidden() const;

		virtual QUndoStack *undoStack() const { return parentAspect() ? parentAspect()->undoStack() : 0; }
		void exec(QUndoCommand *command);
		void exec(QUndoCommand *command, const char *preChangeSignal, const char *postChangeSignal,
				QGenericArgument val0 = QGenericArgument(), QGenericArgument val1 = QGenericArgument(),
				QGenericArgument val2 = QGenericArgument(), QGenericArgument val3 = QGenericArgument());
		void beginMacro(const QString& text);
		void endMacro();

		static QVariant global(const QString &key);
		static void setGlobal(const QString &key, const QVariant &value);
		static void setGlobalDefault(const QString &key, const QVariant &value);

		virtual void save(QXmlStreamWriter *) const {}
		virtual bool load(XmlStreamReader *) { return false; }
	protected:
		bool readBasicAttributes(XmlStreamReader * reader);
		void writeBasicAttributes(QXmlStreamWriter * writer) const;
		void writeCommentElement(QXmlStreamWriter * writer) const;
		bool readCommentElement(XmlStreamReader * reader);

	public slots:
		void setName(const QString &value);
		void setComment(const QString &value);
		void setCaptionSpec(const QString &value);
		void setHidden(bool value);
		virtual void remove() { if(parentAspect()) parentAspect()->removeChild(this); }
		QString uniqueNameFor(const QString &current_name) const;

	protected slots:
		//!called when a child aspect was selected in the model
		virtual void childSelected(){}
		//!called when a child's child aspect was selected in the model
		virtual void childSelected(const AbstractAspect*){}
		//!called when a child aspect was deselected in the model
		virtual void childDeselected(){}
		//!called when a child's child aspect was deselected in the model
		virtual void childDeselected(const AbstractAspect*){}
		
	signals:
		void aspectDescriptionAboutToChange(const AbstractAspect *aspect);
		void aspectDescriptionChanged(const AbstractAspect *aspect);
		void aspectAboutToBeAdded(const AbstractAspect *parent, const AbstractAspect *before, const AbstractAspect * child);
		void aspectAdded(const AbstractAspect *aspect);
		void aspectAboutToBeRemoved(const AbstractAspect *aspect);
		void aspectRemoved(const AbstractAspect *parent, const AbstractAspect * before, const AbstractAspect * child);
		void aspectHiddenAboutToChange(const AbstractAspect *aspect);
		void aspectHiddenChanged(const AbstractAspect *aspect);
		void statusInfo(const QString &text);
		void renameRequested();
		
		//selection/deselection in model (project explorer)
		void selected();
		void selected(const AbstractAspect*);
		void deselected();
		void deselected(const AbstractAspect*);
		
		//selection/deselection in view
		void childAspectSelectedInView(const AbstractAspect*);
		void childAspectDeselectedInView(const AbstractAspect*);
		
	protected:
		void setCreationTime(const QDateTime& time);
		void info(const QString &text) { emit statusInfo(text); }

	private:
		Private * m_aspect_private;
		const QList< AbstractAspect* > rawChildren() const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractAspect::ChildIndexFlags)

#endif // ifndef ABSTRACT_ASPECT_H

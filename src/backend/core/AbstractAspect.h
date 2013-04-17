/***************************************************************************
    File                 : AbstractAspect.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 by Knut Franke (knut.franke*gmx.de), Tilman Benkert (thzs*gmx.net)
	Copyright            : (C) 2010 by Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2011-2012 by Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses)
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

		class Private;
		friend class AspectChildAddCmd;
		friend class AspectChildRemoveCmd;
		
		static void staticInit();

		AbstractAspect(const QString& name);
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
		
		Folder* folder();
		bool isDescendantOf(AbstractAspect* other);

		void setSelected(bool);
		
		void addChild(AbstractAspect*);
		void insertChildBefore(AbstractAspect *child, AbstractAspect *before);
		void reparent(AbstractAspect* new_parent, int new_index=-1);
		void removeChild(AbstractAspect*);
		void removeAllChildren();
		
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

		virtual const Project* project() const { return parentAspect() ? parentAspect()->project() : 0; }
		virtual Project* project() { return parentAspect() ? parentAspect()->project() : 0; }
		virtual QString path() const { return parentAspect() ? parentAspect()->path() + "/" + name() : "";  }

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();

		QString name() const;
		QString comment() const;
		QString captionSpec() const;
		QDateTime creationTime() const;
		QString caption() const;
		bool hidden() const;

		//undo/redo related functions
		virtual QUndoStack* undoStack() const { return parentAspect() ? parentAspect()->undoStack() : 0; }
		void exec(QUndoCommand*);		
		void beginMacro(const QString& text);
		void endMacro();

		//access to global settings
		static QVariant global(const QString &key);
		static void setGlobal(const QString &key, const QVariant &value);
		static void setGlobalDefault(const QString &key, const QVariant &value);

		virtual void save(QXmlStreamWriter*) const {}
		virtual bool load(XmlStreamReader*) { return false; }

	protected:
		void setCreationTime(const QDateTime&);
		void info(const QString &text) { emit statusInfo(text); }

		//serialization/deserialization
		bool readBasicAttributes(XmlStreamReader*);
		void writeBasicAttributes(QXmlStreamWriter*) const;
		void writeCommentElement(QXmlStreamWriter*) const;
		bool readCommentElement(XmlStreamReader*);
		
	private:
		Private* m_aspect_private;
		const QList<AbstractAspect*> rawChildren() const;

	public slots:
		void setName(const QString&);
		void setComment(const QString&);
		void setCaptionSpec(const QString&);
		void setHidden(bool);
		virtual void remove() { if(parentAspect()) parentAspect()->removeChild(this); }
		QString uniqueNameFor(const QString&) const;

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

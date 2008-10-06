/***************************************************************************
    File                 : AbstractAspect.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
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

class AspectPrivate;
class Project;
class QUndoStack;
class QString;
class QDateTime;
class QUndoCommand;
class QIcon;
class QMenu;
class Folder;
class XmlStreamReader;
class QXmlStreamWriter;
class QAction;

//! Base class of all persistent objects in a Project.
/**
 * Before going into the details, it's useful to understand the ideas behind the
 * \ref aspect "Aspect Framework".
 *
 * Aspects organize themselves into trees, where a parent takes ownership of its children. Usually,
 * though not necessarily, a Project instance will sit at the root of the tree (without a Project
 * ancestor, project() will return 0 and undo does not work). Children are organized using
 * addChild(), removeChild(), child(), indexOfChild() and childCount() on the parent's side as well
 * as the equivalent convenience methods index() and remove() on the child's side.
 * 
 * AbstractAspect manages for every Aspect the properties #name, #comment, #caption_spec and
 * #creation_time. All of these translate into the caption() as described in the documentation
 * of setCaptionSpec().
 *
 * If an undoStack() can be found (usually it is managed by Project), changes to the properties
 * as well as adding/removing children support multi-level undo/redo. In order to support undo/redo
 * for problem-specific data in derived classes, make sure that all changes to your data are done
 * by handing appropriate commands to exec().
 *
 * Optionally,
 * you can supply an icon() to be used by different views (including the ProjectExplorer)
 * and/or reimplement createContextMenu() for a custom context menu of views.
 *
 * The private data of AbstractAspect is contained in a separate class AbstractAspect::Private. 
 * The write access to AbstractAspect::Private should always be done using aspect commands
 * to allow undo/redo.
 */
class AbstractAspect : public QObject
{
	Q_OBJECT

	public:
		class Private;
		friend class Private;

		AbstractAspect(const QString &name);
		virtual ~AbstractAspect();

		//! Return my parent Aspect or 0 if I currently don't have one.
		AbstractAspect * parentAspect() const;
		//! Return the folder the Aspect is contained in or 0 if not.
		/**
		 * The returned folder may be the aspect itself if it inherits Folder.
		*/
		Folder * folder();
		//! Return whether the there is a path upwards to the given aspect
		/**
		 * This also returns true if other==this.
		 */
		bool isDescendantOf(AbstractAspect *other);

		//! Add the given Aspect to my list of children.
		void addChild(AbstractAspect* child);
		//! Insert the given Aspect at a specific position in my list of children.
		void insertChild(AbstractAspect *child, int index);
		//! Remove the given Aspect from my list of children.
		/**
		 * The ownership of the child is transfered to the undo command,
		 * i.e., the aspect is deleted by the undo command.
		 * \sa reparentChild()
		 */
		void removeChild(AbstractAspect* child);
		//! Remove the Aspect at the given index from my list of children.
		/**
		 * The ownership of the child is transfered to the undo command,
		 * i.e., the aspect is deleted by the undo command.
		 * \sa reparentChild()
		 */
		void removeChild(int index);
		//! Get a child by its position in my list of children.
		AbstractAspect* child(int index) const;
		//! Return the number of child Aspects.
		int childCount() const;
		//! Return the position of child in my list of children.
		int indexOfChild(const AbstractAspect * child) const;
		//! Return my position in my parent's list of children.
		int index() const { return parentAspect() ? parentAspect()->indexOfChild(this) : 0; }
		//! Change the positon of a child in my list of children.
		void moveChild(int from, int to);
		//! Move a child to another aspect and transfer ownership.
		void reparentChild(AbstractAspect *new_parent, AbstractAspect *child, int new_index);
		//! Move a child to another aspect and transfer ownership.
		void reparentChild(AbstractAspect *new_parent, AbstractAspect *child);
		//! Get all descendents that inherit the given class
		QList<AbstractAspect *> descendantsThatInherit(const char *class_name);
		//! Remove all child aspects
		virtual void removeAllChildAspects();

		//! Return the Project this Aspect belongs to, or 0 if it is currently not part of one.
		virtual const Project *project() const { return parentAspect() ? parentAspect()->project() : 0; }
		//! Return the Project this Aspect belongs to, or 0 if it is currently not part of one.
		virtual Project *project() { return parentAspect() ? parentAspect()->project() : 0; }
		//! Return the path that leads from the top-most Aspect (usually a Project) to me.
		virtual QString path() const { return parentAspect() ? parentAspect()->path() + "/" + name() : "";  }

		//! Return an icon to be used for decorating my views.
		virtual QIcon icon() const;
		//! Return a new context menu.
		/**
		 * The caller takes ownership of the menu.
		 */
		virtual QMenu *createContextMenu();

		QString name() const;
		QString comment() const;
		//! Return the specification string used for constructing the caption().
		/**
		 * See setCaptionSpec() for format.
		 */
		QString captionSpec() const;
		QDateTime creationTime() const;
		QString caption() const;

		//! \name undo related
		//@{
		//! Return the undo stack of the Project, or 0 if this Aspect is not part of a Project.
		/**
		 * It's also possible to construct undo-enabled Aspect trees without Project.
		 * The only requirement is that the root Aspect reimplements undoStack() to get the
		 * undo stack from somewhere (the default implementation just delegates to parentAspect()).
		 */
		virtual QUndoStack *undoStack() const { return parentAspect() ? parentAspect()->undoStack() : 0; }
		//! Execute the given command, pushing it on the undoStack() if available.
		void exec(QUndoCommand *command);
		//! Begin an undo stack macro (series of commands)
		void beginMacro(const QString& text);
		//! End the undo stack macro
		void endMacro();
		//@}

		//! Retrieve a global setting.
		static QVariant global(const QString &key);
		//! Update a global setting.
		static void setGlobal(const QString &key, const QVariant &value);
		//! Set default value for a global setting.
		static void setGlobalDefault(const QString &key, const QVariant &value);

		//! \name serialize/deserialize
		//@{
		//! Save as XML
		virtual void save(QXmlStreamWriter *) const {};
		//! Load from XML
		/**
		 * XmlStreamReader supports errors as well as warnings. If only
		 * warnings (non-critial errors) occur, this function must return 
		 * the reader at the end element corresponding to the current 
		 * element at the time the function was called. 
		 *
		 * This function is normally intended to be called directly 
		 * after the ctor. If you want to call load on an aspect that
		 * has been altered, you must make sure beforehand that
		 * it is in the same state as after creation, e.g., remove
		 * all its child aspects.
		 *
		 * \return false on error
		 */
		virtual bool load(XmlStreamReader *) { return false; };
	protected:
		//! Load name, creation time and caption spec from XML
		/**
		 * \return false on error
		 */
		bool readBasicAttributes(XmlStreamReader * reader);
		//! Save name, creation time and caption spec to XML
		void writeBasicAttributes(QXmlStreamWriter * writer) const;
		//! Save the comment to XML
		void writeCommentElement(QXmlStreamWriter * writer) const;
		//! Load comment from an XML element
		bool readCommentElement(XmlStreamReader * reader);
		//@}

	public slots:
		void setName(const QString &value);
		void setComment(const QString &value);
		//! Set the specification string used for constructing the caption().
		/**
		 * The caption is constructed by substituting some magic tokens in the specification:
		 *
		 * - \%n - name()
		 * - \%c - comment()
		 * - \%t - creationTime()
		 *
		 * Additionally, you can use the construct %C{<text>}. <text> is only shown in the caption
		 * if comment() is not empty (name and creation time should never be empty).
		 *
		 * The default caption specification is "%n%C{ - }%c".
		 */
		void setCaptionSpec(const QString &value);
		//! Remove me from my parent's list of children.
		virtual void remove() { if(parentAspect()) parentAspect()->removeChild(parentAspect()->indexOfChild(this)); }
		//! Make the specified name unique among my children by incrementing a trailing number.
		QString uniqueNameFor(const QString &current_name) const;

	signals:
		//! Emit this before the name, comment or caption spec is changed
		void aspectDescriptionAboutToChange(const AbstractAspect *aspect);
		//! Emit this when the name, comment or caption spec changed
		void aspectDescriptionChanged(const AbstractAspect *aspect);
		//! Emit this when a parent aspect is about to get a new child inserted
		void aspectAboutToBeAdded(const AbstractAspect *parent, int index);
		//! Emit this from a newly added aspect
		void aspectAdded(const AbstractAspect *aspect);
		//! Emit this from a parent after adding a new child to it
		void aspectAdded(const AbstractAspect *parent, int index);
		//! Emit this from an aspect about to be removed from its parent's children
		void aspectAboutToBeRemoved(const AbstractAspect *aspect);
		//! Emit this from a parent before removing its child
		void aspectAboutToBeRemoved(const AbstractAspect *parent, int index);
		//! Emit this from the parent after removing a child
		void aspectRemoved(const AbstractAspect *parent, int index);
		//! Emit this to give status information to the user.
		void statusInfo(const QString &text);

	protected:
		//! Set the creation time
		/**
		 * The creation time will automatically be set when the aspect object
		 * is created. This function is usually only needed when the aspect
		 * is loaded from a file.
		 */
		void setCreationTime(const QDateTime& time);
		//! Called after a new child has been inserted or added.
		/**
		 * Unlike the aspectAdded() signals, this method does not get called inside undo/redo actions;
		 * allowing subclasses to execute undo commands of their own.
		 */
		virtual void completeAspectInsertion(AbstractAspect * aspect, int index) { Q_UNUSED(aspect); Q_UNUSED(index); }
		//! Called before a child is removed.
		/**
		 * Unlike the aspectAboutToBeRemoved() signals, this method does not get called inside undo/redo actions;
		 * allowing subclasses to execute undo commands of their own.
		 */
		virtual void prepareAspectRemoval(AbstractAspect * aspect) { Q_UNUSED(aspect); }
		//! Implementations should call this whenever status information should be given to the user.
		/**
		 * This will cause statusInfo() to be emitted. Typically, this will cause the specified string
		 * to be displayed in a status bar, a log window or some similar non-blocking way so as not to
		 * disturb the workflow.
		 */
		void info(const QString &text) { emit statusInfo(text); }

	private:
		Private * m_aspect_private;
};

#endif // ifndef ABSTRACT_ASPECT_H

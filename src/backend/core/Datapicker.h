#ifndef DATAPICKER_H
#define DATAPICKER_H

#include "backend/core/AbstractPart.h"
#include "backend/core/AbstractScriptingEngine.h"

class Spreadsheet;
class Worksheet;
class QXmlStreamWriter;
class XmlStreamReader;

class Datapicker : public AbstractPart, public scripted {
	Q_OBJECT

	public:
		Datapicker(AbstractScriptingEngine* engine, const QString& name);

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual QWidget* view() const;

        void initDefault();

		Spreadsheet* currentSpreadsheet() const;
        Worksheet* currentWorksheet() const;
		void setChildSelectedInView(int index, bool selected);

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

	private slots:
		virtual void childSelected(const AbstractAspect*);
		virtual void childDeselected(const AbstractAspect*);

	signals:
		void requestProjectContextMenu(QMenu*);
        void datapickerItemSelected(int);
};

#endif

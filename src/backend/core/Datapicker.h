#ifndef DATAPICKER_H
#define DATAPICKER_H

#include "backend/core/AbstractPart.h"
#include "backend/core/AbstractScriptingEngine.h"

class Spreadsheet;
class Column;
class Image;
class QXmlStreamWriter;
class XmlStreamReader;

class Datapicker : public AbstractPart, public scripted {
	Q_OBJECT

	public:
		Datapicker(AbstractScriptingEngine* engine, const QString& name);
        enum DataColumnType { PositionX, PositionY, PlusDeltaX, MinusDeltaX, PlusDeltaY, MinusDeltaY };

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual QWidget* view() const;
        void initDefault();

		Spreadsheet* currentSpreadsheet() const;
        Image* currentImage() const;
		void setChildSelectedInView(int index, bool selected);
        void addDataToSheet(double, int, const Datapicker::DataColumnType&);

        Spreadsheet* m_datasheet;
        Image* m_image;

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

    public slots:
        virtual void childSelected(const AbstractAspect*);

    private:
        static QString columnNameFromType(Datapicker::DataColumnType);
        int* columnIndexFromType(Datapicker::DataColumnType);
        void addDataToColumn(int, double, DataColumnType);
        void addDatasheet();

        //column index
        int m_positionX;
        int m_positionY;
        int m_plusDeltaX;
        int m_minusDeltaX;
        int m_plusDeltaY;
        int m_minusDeltaY;

    private slots:
		virtual void childDeselected(const AbstractAspect*);
        void handleColumnRemoved(const AbstractAspect*);

	signals:
        void datapickerItemSelected(int);
};

#endif

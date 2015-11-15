/***************************************************************************
    File                 : DataPickerCurve.cpp
    Project              : LabPlot
    Description          : container for Curve-Point and Datasheet/Spreadsheet
                           of datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
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

#include "DataPickerCurve.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/datapicker/CustomItem.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/datapicker/DataPickerCurvePrivate.h"

#include <QMenu>
#include <QVector3D>

#include <KLocale>
#include <KIcon>
#include <KAction>
#include <KConfigGroup>

/**
 * \class DataPickerCurve
 * \brief Top-level container for Curve-Point and Datasheet/Spreadsheet of datapicker.
 * \ingroup backend
 */

DataPickerCurve::DataPickerCurve(const QString &name) : AbstractAspect(name), d_ptr(new DataPickerCurvePrivate(this)) {

    init();
}

DataPickerCurve::DataPickerCurve(const QString &name, DataPickerCurvePrivate *dd) : AbstractAspect(name), d_ptr(dd) {

    init();
}

DataPickerCurve::~DataPickerCurve() {
}

void DataPickerCurve::init() {
    Q_D(DataPickerCurve);

    KConfig config;
    KConfigGroup group;
    group = config.group("DataPickerCurve");
    d->posXColumn = NULL;
    d->posYColumn = NULL;
    d->posZColumn = NULL;
    d->plusDeltaXColumn = NULL;
    d->minusDeltaXColumn = NULL;
    d->plusDeltaYColumn = NULL;
    d->minusDeltaYColumn = NULL;
    d->curveErrorTypes.x = (ErrorType) group.readEntry("CurveErrorType_X", (int) NoError);
    d->curveErrorTypes.y = (ErrorType) group.readEntry("CurveErrorType_X", (int) NoError);

    this->initAction();
}

void DataPickerCurve::initAction() {
    updateDatasheetAction = new KAction(KIcon("view-refresh"), i18n("Update Spreadsheet"), this);
    connect( updateDatasheetAction, SIGNAL(triggered()), this, SLOT(updateDatasheet()) );
}

/*!
    Returns an icon to be used in the project explorer.
*/
QIcon DataPickerCurve::icon() const {
    return  KIcon("labplot-xy-curve");
}

/*!
    Return a new context menu
*/
QMenu* DataPickerCurve::createContextMenu() {
    QMenu *menu = AbstractAspect::createContextMenu();
    Q_ASSERT(menu);

    QAction* firstAction = 0;
    if (menu->actions().size()>1)
        firstAction = menu->actions().at(1);

    menu->insertAction(firstAction, updateDatasheetAction);

    return menu;
}

Column* DataPickerCurve::appendColumn(const QString& name, Spreadsheet* datasheet) {
    Column* col = new Column(i18n("Column"), AbstractColumn::Numeric);
    col->setPlotDesignation(AbstractColumn::Y);
    col->insertRows(0, datasheet->rowCount());
    col->setName(name);
    datasheet->addChild(col);

    return col;
}

void DataPickerCurve::addCurvePoint(const QPointF& position) {
    QList<CustomItem*> childItems = children<CustomItem>(IncludeHidden);
    if (childItems.isEmpty())
        beginMacro(i18n("%1:add Curve Point", name()));
    else
        beginMacro(i18n("%1:add Curve Point %2", name(), childItems.count()));

    CustomItem* newItem = new CustomItem(i18n("Curve Point"));
    newItem->setPosition(position);
    newItem->setHidden(true);
    newItem->initErrorBar(curveErrorTypes());
    //set properties of added custom-item same as previous items
    if (!childItems.isEmpty()) {
        CustomItem* m_item = childItems.first();
        newItem->setItemsBrush(m_item->itemsBrush());
        newItem->setItemsOpacity(m_item->itemsOpacity());
        newItem->setItemsPen(m_item->itemsPen());
        newItem->setItemsRotationAngle(m_item->itemsRotationAngle());
        newItem->setItemsSize(m_item->itemsSize());
        newItem->setItemsStyle(m_item->itemsStyle());
        newItem->setErrorBarBrush(m_item->errorBarBrush());
        newItem->setErrorBarSize(m_item->errorBarSize());
        newItem->setErrorBarPen(m_item->errorBarPen());
    }

    addChild(newItem);
    updateData(newItem);
    endMacro();
}
//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(DataPickerCurve, DataPickerCurve::Errors, curveErrorTypes, curveErrorTypes)
BASIC_SHARED_D_READER_IMPL(DataPickerCurve, AbstractColumn*, posXColumn, posXColumn)
QString& DataPickerCurve::posXColumnPath() const { return d_ptr->posXColumnPath; }
BASIC_SHARED_D_READER_IMPL(DataPickerCurve, AbstractColumn*, posYColumn, posYColumn)
QString& DataPickerCurve::posYColumnPath() const { return d_ptr->posYColumnPath; }
BASIC_SHARED_D_READER_IMPL(DataPickerCurve, AbstractColumn*, posZColumn, posZColumn)
QString& DataPickerCurve::posZColumnPath() const { return d_ptr->posZColumnPath; }
BASIC_SHARED_D_READER_IMPL(DataPickerCurve, AbstractColumn*, plusDeltaXColumn, plusDeltaXColumn)
QString& DataPickerCurve::plusDeltaXColumnPath() const { return d_ptr->plusDeltaXColumnPath; }
BASIC_SHARED_D_READER_IMPL(DataPickerCurve, AbstractColumn*, minusDeltaXColumn, minusDeltaXColumn)
QString& DataPickerCurve::minusDeltaXColumnPath() const { return d_ptr->minusDeltaXColumnPath; }
BASIC_SHARED_D_READER_IMPL(DataPickerCurve, AbstractColumn*, plusDeltaYColumn, plusDeltaYColumn)
QString& DataPickerCurve::plusDeltaYColumnPath() const { return d_ptr->plusDeltaYColumnPath; }
BASIC_SHARED_D_READER_IMPL(DataPickerCurve, AbstractColumn*, minusDeltaYColumn, minusDeltaYColumn)
QString& DataPickerCurve::minusDeltaYColumnPath() const { return d_ptr->minusDeltaYColumnPath; }

//##############################################################################
//#########################  setter methods  ###################################
//##############################################################################
void DataPickerCurve::addDatasheet(const Image::GraphType& type) {
    Q_D(DataPickerCurve);

    m_datasheet = new Spreadsheet(0, i18n("Data"));
    addChild(m_datasheet);
    QString xLabel = "x";
    QString yLabel = "y";

    if (type == Image::PolarInDegree) {
        xLabel = "r";
        yLabel = "y(deg)";
    } else if (type == Image::PolarInRadians) {
        xLabel = "r";
        yLabel = "y(rad)";
    } else if (type == Image::LogarithmicX) {
        xLabel = "log(x)";
        yLabel = "y";
    } else if (type == Image::LogarithmicY) {
        xLabel = "x";
        yLabel = "log(y)";
    }

    if (type == Image::Ternary)
        d->posZColumn = appendColumn(i18n("c"), m_datasheet);

    d->posXColumn = m_datasheet->column(0);
    d->posXColumn->setName(xLabel);

    d->posYColumn = m_datasheet->column(1);
    d->posYColumn->setName(yLabel);
}

STD_SETTER_CMD_IMPL_S(DataPickerCurve, SetCurveErrorTypes, DataPickerCurve::Errors, curveErrorTypes)
void DataPickerCurve::setCurveErrorTypes(const DataPickerCurve::Errors errors) {
    Q_D(DataPickerCurve);
    if (d->curveErrorTypes.x != errors.x || d->curveErrorTypes.y != errors.y) {
        beginMacro(i18n("%1: set xy-error type", name()));
        exec(new DataPickerCurveSetCurveErrorTypesCmd(d, errors, i18n("%1: set xy-error type")));

        if ( errors.x != NoError && !d->plusDeltaXColumn ) {
            setPlusDeltaXColumn(appendColumn(i18n("+delta_x"), m_datasheet));
        } else if ( d->plusDeltaXColumn && errors.x == NoError ) {
            d->plusDeltaXColumn->remove();
            d->plusDeltaXColumn = 0;
        }

        if ( errors.x == AsymmetricError && !d->minusDeltaXColumn ) {
            setMinusDeltaXColumn(appendColumn(i18n("-delta_x"), m_datasheet));
        } else if ( d->minusDeltaXColumn && errors.x != AsymmetricError ) {
            d->minusDeltaXColumn->remove();
            d->minusDeltaXColumn = 0;
        }

        if ( errors.y != NoError && !d->plusDeltaYColumn ) {
            setPlusDeltaYColumn(appendColumn(i18n("+delta_y"), m_datasheet));
        } else if ( d->plusDeltaYColumn && errors.y == NoError ) {
            d->plusDeltaYColumn->remove();
            d->plusDeltaYColumn = 0;
        }

        if ( errors.y == AsymmetricError && !d->minusDeltaYColumn ) {
            setMinusDeltaYColumn(appendColumn(i18n("-delta_y"), m_datasheet));
        } else if ( d->minusDeltaYColumn && errors.y != AsymmetricError ) {
            d->minusDeltaYColumn->remove();
            d->minusDeltaYColumn = 0;
        }

        endMacro();
    }
}

STD_SETTER_CMD_IMPL_S(DataPickerCurve, SetPosXColumn, AbstractColumn*, posXColumn)
void DataPickerCurve::setPosXColumn(AbstractColumn* column) {
    Q_D(DataPickerCurve);
    if (d->posXColumn != column)
        exec(new DataPickerCurveSetPosXColumnCmd(d, column, i18n("%1: set position X column")));
}

STD_SETTER_CMD_IMPL_S(DataPickerCurve, SetPosYColumn, AbstractColumn*, posYColumn)
void DataPickerCurve::setPosYColumn(AbstractColumn* column) {
    Q_D(DataPickerCurve);
    if (d->posYColumn != column)
        exec(new DataPickerCurveSetPosYColumnCmd(d, column, i18n("%1: set position Y column")));
}

STD_SETTER_CMD_IMPL_S(DataPickerCurve, SetPosZColumn, AbstractColumn*, posZColumn)
void DataPickerCurve::setPosZColumn(AbstractColumn* column) {
    Q_D(DataPickerCurve);
    if (d->posZColumn != column)
        exec(new DataPickerCurveSetPosZColumnCmd(d, column, i18n("%1: set position Z column")));
}

STD_SETTER_CMD_IMPL_S(DataPickerCurve, SetPlusDeltaXColumn, AbstractColumn*, plusDeltaXColumn)
void DataPickerCurve::setPlusDeltaXColumn(AbstractColumn* column) {
    Q_D(DataPickerCurve);
    if (d->plusDeltaXColumn != column)
        exec(new DataPickerCurveSetPlusDeltaXColumnCmd(d, column, i18n("%1: set +delta_X column")));
}

STD_SETTER_CMD_IMPL_S(DataPickerCurve, SetMinusDeltaXColumn, AbstractColumn*, minusDeltaXColumn)
void DataPickerCurve::setMinusDeltaXColumn(AbstractColumn* column) {
    Q_D(DataPickerCurve);
    if (d->minusDeltaXColumn != column)
        exec(new DataPickerCurveSetMinusDeltaXColumnCmd(d, column, i18n("%1: set -delta_X column")));
}

STD_SETTER_CMD_IMPL_S(DataPickerCurve, SetPlusDeltaYColumn, AbstractColumn*, plusDeltaYColumn)
void DataPickerCurve::setPlusDeltaYColumn(AbstractColumn* column) {
    Q_D(DataPickerCurve);
    if (d->plusDeltaYColumn != column)
        exec(new DataPickerCurveSetPlusDeltaYColumnCmd(d, column, i18n("%1: set +delta_Y column")));
}

STD_SETTER_CMD_IMPL_S(DataPickerCurve, SetMinusDeltaYColumn, AbstractColumn*, minusDeltaYColumn)
void DataPickerCurve::setMinusDeltaYColumn(AbstractColumn* column) {
    Q_D(DataPickerCurve);
    if (d->minusDeltaYColumn != column)
        exec(new DataPickerCurveSetMinusDeltaYColumnCmd(d, column, i18n("%1: set -delta_Y column")));
}

void DataPickerCurve::setPrinting(bool on) {
    foreach (WorksheetElement* elem, children<WorksheetElement>(IncludeHidden))
        elem->setPrinting(on);
}

/*!
    Selects or deselects the Datapicker/Curve in the project explorer.
    This function is called in \c ImageView.
*/
void DataPickerCurve::setSelectedInView(const bool b) {
    if (b)
        emit childAspectSelectedInView(this);
    else
        emit childAspectDeselectedInView(this);
}
//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void DataPickerCurve::updateDatasheet() {
    beginMacro(i18n("%1:update datasheet", name()));

    foreach (CustomItem* item, children<CustomItem>(IncludeHidden))
        updateData(item);

    endMacro();
}

/*!
    Update datasheet for corresponding custom-item(curve-point),
    it is called every time whenever there is any change in position
    of curve-point or its error-bar so keep it undo unaware
    no need to create extra entry in undo stack
*/
void DataPickerCurve::updateData(const CustomItem* item) {
    Q_D(DataPickerCurve);
    Datapicker* datapicker = dynamic_cast<Datapicker*>(parentAspect());
    if (!datapicker)
        return;

    int row = indexOfChild<CustomItem>(item ,AbstractAspect::IncludeHidden);
    QVector3D data = datapicker->mapSceneToLogical(item->position().point);

    if(d->posXColumn)
        d->posXColumn->setValueAt(row, data.x());

    if(d->posYColumn)
        d->posYColumn->setValueAt(row, data.y());

    if(d->posZColumn)
        d->posZColumn->setValueAt(row, data.y());

    if (d->plusDeltaXColumn) {
        data = datapicker->mapSceneLengthToLogical(QPointF(item->plusDeltaXPos().x(), 0));
        d->plusDeltaXColumn->setValueAt(row, qAbs(data.x()));
    }

    if (d->minusDeltaXColumn) {
        data = datapicker->mapSceneLengthToLogical(QPointF(item->minusDeltaXPos().x(), 0));
        d->minusDeltaXColumn->setValueAt(row, qAbs(data.x()));
    }

    if (d->plusDeltaYColumn) {
        data = datapicker->mapSceneLengthToLogical(QPointF(0, item->plusDeltaYPos().y()));
        d->plusDeltaYColumn->setValueAt(row, qAbs(data.y()));
    }

    if (d->minusDeltaYColumn) {
        data = datapicker->mapSceneLengthToLogical(QPointF(0, item->minusDeltaYPos().y()));
        d->minusDeltaYColumn->setValueAt(row, qAbs(data.y()));
    }
}
//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void DataPickerCurve::save(QXmlStreamWriter* writer) const{
    Q_D(const DataPickerCurve);

    writer->writeStartElement( "dataPickerCurve" );
    writeBasicAttributes(writer);
    writeCommentElement(writer);

    //general
    writer->writeStartElement( "general" );
    WRITE_COLUMN(d->posXColumn, posXColumn);
    WRITE_COLUMN(d->posYColumn, posYColumn);
    WRITE_COLUMN(d->posZColumn, posZColumn);
    WRITE_COLUMN(d->plusDeltaXColumn, plusDeltaXColumn);
    WRITE_COLUMN(d->minusDeltaXColumn, minusDeltaXColumn);
    WRITE_COLUMN(d->plusDeltaYColumn, plusDeltaYColumn);
    WRITE_COLUMN(d->minusDeltaYColumn, minusDeltaYColumn);
    writer->writeAttribute( "curveErrorType_X", QString::number(d->curveErrorTypes.x) );
    writer->writeAttribute( "curveErrorType_Y", QString::number(d->curveErrorTypes.y) );
    writer->writeEndElement();

    //serialize all children
    QList<AbstractAspect *> childrenAspect = children<AbstractAspect>(IncludeHidden);
    foreach(AbstractAspect *child, childrenAspect)
        child->save(writer);

    writer->writeEndElement(); // close section
}

//! Load from XML
bool DataPickerCurve::load(XmlStreamReader* reader) {
    Q_D(DataPickerCurve);

    if(!reader->isStartElement() || reader->name() != "dataPickerCurve") {
        reader->raiseError(i18n("no image element found"));
        return false;
    }

    if (!readBasicAttributes(reader))
        return false;

    QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
    QXmlStreamAttributes attribs;
    QString str;

    while (!reader->atEnd()) {
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "dataPickerCurve")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "comment"){
            if (!readCommentElement(reader)) return false;
        } else if (reader->name() == "general") {
            attribs = reader->attributes();

            str = attribs.value("curveErrorType_X").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("curveErrorType_X"));
            else
                d->curveErrorTypes.x = ErrorType(str.toInt());

            str = attribs.value("curveErrorType_Y").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("curveErrorType_Y"));
            else
                d->curveErrorTypes.y = ErrorType(str.toInt());

            READ_COLUMN(posXColumn);
            READ_COLUMN(posYColumn);
            READ_COLUMN(posZColumn);
            READ_COLUMN(plusDeltaXColumn);
            READ_COLUMN(minusDeltaXColumn);
            READ_COLUMN(plusDeltaYColumn);
            READ_COLUMN(minusDeltaYColumn);

        } else if (reader->name() == "customItem") {
            CustomItem* curvePoint = new CustomItem("");
            curvePoint->setHidden(true);
            if (!curvePoint->load(reader)){
                delete curvePoint;
                return false;
            } else {
                addChild(curvePoint);
                curvePoint->initErrorBar(curveErrorTypes());
            }
        } else if (reader->name() == "spreadsheet") {
            Spreadsheet* datasheet = new Spreadsheet(0, "spreadsheet", true);
            if (!datasheet->load(reader)){
                delete datasheet;
                return false;
            } else {
                addChild(datasheet);
                m_datasheet = datasheet;
            }
        } else { // unknown element
            reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

    return true;
}

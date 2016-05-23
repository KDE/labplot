/***************************************************************************
File                 : AsciiFilter.cpp
Project              : LabPlot
Description          : ASCII I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2009-2015 Stefan Gerlach (stefan.gerlach@uni.kn)
Copyright            : (C) 2009-2015 Alexander Semke (alexander.semke@web.de)

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
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/filters/AsciiFilterPrivate.h"
#include "backend/datasources/FileDataSource.h"
#include "backend/core/column/Column.h"

#include <math.h>

#include <QTextStream>
#include <QDebug>
#include <KLocale>
#include <kfilterdev.h>

 /*!
	\class AsciiFilter
	\brief Manages the import/export of data organized as columns (vectors) from/to an ASCII-file.

	\ingroup datasources
 */

AsciiFilter::AsciiFilter():AbstractFileFilter(), d(new AsciiFilterPrivate(this)){

}

AsciiFilter::~AsciiFilter(){
	delete d;
}

/*!
  reads the content of the file \c fileName.
*/
QString AsciiFilter::readData(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode,  int lines){
	return d->readData(fileName, dataSource, importMode, lines);
}

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
void AsciiFilter::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode){
	d->read(fileName, dataSource, importMode);
}


/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void AsciiFilter::write(const QString & fileName, AbstractDataSource* dataSource){
 	d->write(fileName, dataSource);
// 	emit()
}

/*!
  loads the predefined filter settings for \c filterName
*/
void AsciiFilter::loadFilterSettings(const QString& filterName){
	Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void AsciiFilter::saveFilterSettings(const QString& filterName) const{
	Q_UNUSED(filterName);
}

/*!
  returns the list with the names of all saved
  (system wide or user defined) filter settings.
*/
QStringList AsciiFilter::predefinedFilters(){
	return QStringList();
}

/*!
  returns the list of all predefined separator characters.
*/
QStringList AsciiFilter::separatorCharacters(){
	return (QStringList()<<"auto"<<"TAB"<<"SPACE"<<","<<";"<<":"
				  <<",TAB"<<";TAB"<<":TAB"
				  <<",SPACE"<<";SPACE"<<":SPACE");
}

/*!
returns the list of all predefined comment characters.
*/
QStringList AsciiFilter::commentCharacters(){
	return (QStringList()<<"#"<<"!"<<"//"<<"+"<<"c"<<":"<<";");
}

/*!
    returns the number of columns in the file \c fileName.
*/
int AsciiFilter::columnNumber(const QString & fileName){
	QString line;
	QStringList lineStringList;

	QIODevice *device = KFilterDev::deviceForFile(fileName);
	if (!device->open(QIODevice::ReadOnly))
		return 0;

	QTextStream in(device);
	line = in.readLine();
	lineStringList = line.split( QRegExp("\\s+")); //TODO
	return lineStringList.size();
}


/*!
  returns the number of lines in the file \c fileName.
*/
long AsciiFilter::lineNumber(const QString & fileName){
	//TODO: compare the speed of this function with the speed of wc from GNU-coreutils.
	QIODevice *device = KFilterDev::deviceForFile(fileName);
	if (!device->open(QIODevice::ReadOnly))
		return 0;

	QTextStream in(device);
	long rows=0;
	while (!in.atEnd()){
		in.readLine();
		rows++;
	}

	return rows;
}

void AsciiFilter::setTransposed(const bool b){
	d->transposed=b;
}

bool AsciiFilter::isTransposed() const{
	return d->transposed;
}

void AsciiFilter::setCommentCharacter(const QString& s){
	d->commentCharacter=s;
}

QString AsciiFilter::commentCharacter() const{
	return d->commentCharacter;
}

void AsciiFilter::setSeparatingCharacter(const QString& s){
	d->separatingCharacter=s;
}

QString AsciiFilter::separatingCharacter() const{
	return d->separatingCharacter;
}

void AsciiFilter::setAutoModeEnabled(bool b){
	d->autoModeEnabled=b;
}

bool AsciiFilter::isAutoModeEnabled() const{
	return d->autoModeEnabled;
}

void AsciiFilter::setHeaderEnabled(bool b){
	d->headerEnabled=b;
}

bool AsciiFilter::isHeaderEnabled() const{
	return d->headerEnabled;
}

void AsciiFilter::setVectorNames(const QString s){
	d->vectorNames=s.simplified();
}

QString AsciiFilter::vectorNames() const{
	return d->vectorNames;
}

void AsciiFilter::setSkipEmptyParts(bool b){
	d->skipEmptyParts=b;
}

bool AsciiFilter::skipEmptyParts() const{
	return d->skipEmptyParts;
}

void AsciiFilter::setSimplifyWhitespacesEnabled(bool b){
	d->simplifyWhitespacesEnabled=b;
}

bool AsciiFilter::simplifyWhitespacesEnabled() const{
	return d->simplifyWhitespacesEnabled;
}

void AsciiFilter::setStartRow(const int r){
	d->startRow=r;
}

int AsciiFilter::startRow() const{
	return d->startRow;
}

void AsciiFilter::setEndRow(const int r){
	d->endRow=r;
}

int AsciiFilter::endRow() const{
	return d->endRow;
}

void AsciiFilter::setStartColumn(const int c){
	d->startColumn=c;
}

int AsciiFilter::startColumn() const{
	return d->startColumn;
}

void AsciiFilter::setEndColumn(const int c){
	d->endColumn=c;
}

int AsciiFilter::endColumn() const{
	return d->endColumn;
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################
AsciiFilterPrivate::AsciiFilterPrivate(AsciiFilter* owner) : q(owner),
	commentCharacter("#"),
	separatingCharacter("auto"),
	autoModeEnabled(true),
	headerEnabled(true),
	skipEmptyParts(false),
	simplifyWhitespacesEnabled(true),
	transposed(false),
	startRow(1),
	endRow(-1),
	startColumn(1),
	endColumn(-1) {
}

/*!
    reads the content of the file \c fileName to the data source \c dataSource or return as string for preview.
    Uses the settings defined in the data source.
*/
QString AsciiFilterPrivate::readData(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode, int lines){
	QStringList dataString;

	QIODevice *device = KFilterDev::deviceForFile(fileName);
	if (!device->open(QIODevice::ReadOnly))
		return QString();

	QTextStream in(device);

	//TODO implement
	// if (transposed)
	//...

	//skip rows, if required
	for (int i=0; i<startRow-1; i++){
        //if the number of rows to skip is bigger then the actual number
		//of the rows in the file, then quit the function.
		if( in.atEnd() ) {
			if (mode==AbstractFileFilter::Replace) {
				//file with no data to be imported. In replace-mode clear the data source
				if(dataSource != NULL)
					dataSource->clear();
			}
			return QString();
		}

		in.readLine();
	}

	//parse the first row:
	//use the first row to determine the number of columns,
	//create the columns and use (optionaly) the first row to name them
	if( in.atEnd() ) {
		if (mode==AbstractFileFilter::Replace) {
			//file with no data to be imported. In replace-mode clear the data source
			if(dataSource != NULL)
				dataSource->clear();
		}
		return QString();
	}

	QString line = in.readLine();
	if( simplifyWhitespacesEnabled)
		line = line.simplified();

	// determine separator
	QString separator;
	QStringList lineStringList;
	if( separatingCharacter == "auto" ){
		QRegExp regExp("(\\s+)|(,\\s+)|(;\\s+)|(:\\s+)");
		lineStringList = line.split( regExp, QString::SplitBehavior(skipEmptyParts) );

		//determine the separator
		if (lineStringList.size()){
			int length1 = lineStringList.at(0).length();
			if (lineStringList.size()>1){
				int pos2 = line.indexOf(lineStringList.at(1),length1);
				separator = line.mid(length1, pos2-length1);
			}else {
				separator = line.right(line.length()-length1);
			}
		}
	}else {
		separator = separatingCharacter.replace(QLatin1String("TAB"), QLatin1String("\t"), Qt::CaseInsensitive);
		separator = separatingCharacter.replace(QLatin1String("SPACE"), QLatin1String(" "), Qt::CaseInsensitive);
		lineStringList = line.split( separator, QString::SplitBehavior(skipEmptyParts) );
	}
#ifdef QT_DEBUG
 	qDebug() << "separator '"<<separator << "'";
#endif

	if (endColumn == -1)
		endColumn = lineStringList.size(); //use the last available column index

	QStringList vectorNameList;
	if ( headerEnabled ){
		vectorNameList = lineStringList;
	}else{
		//create vector names out of the space separated vectorNames-string, if not empty
		if (!vectorNames.isEmpty()){
			vectorNameList = vectorNames.split(' ');
		}
	}

	//qDebug()<<"	vector names ="<<vectorNameList;

	int actualRows = AsciiFilter::lineNumber(fileName);	// data rows
	int actualEndRow;
	if (endRow == -1)
		actualEndRow = actualRows;
	else if (endRow > actualRows-1)
		actualEndRow = actualRows-1;
	else
		actualEndRow = endRow;
	int actualCols=endColumn-startColumn+1;

	if (headerEnabled)
		actualRows = actualEndRow-startRow;
	else
		actualRows = actualEndRow-startRow+1;
	if (lines == -1)
		lines=actualRows;

#ifdef QT_DEBUG
	qDebug()<<"	start column ="<<startColumn;
	qDebug()<<"	end column ="<<endColumn;
	qDebug()<<"	actual cols ="<<actualCols;
	qDebug()<<"	start row ="<<startRow;
	qDebug()<<"	end row ="<<actualEndRow;
	qDebug()<<"	actual rows ="<<actualRows;
	qDebug()<<"	lines ="<<lines;
#endif

	int currentRow=0; //indexes the position in the vector(column)
	int columnOffset=0; //indexes the "start column" in the spreadsheet. Starting from this column the data will be imported.
	//pointers to the actual data containers
	QVector<QVector<double>*> dataPointers;

	if(dataSource != NULL)
		columnOffset = dataSource->create(dataPointers, mode, actualRows, actualCols);

	//header: import the values in the first line, if they were not used as the header (as the names for the columns)
	bool isNumber;
	if (!headerEnabled){
		for ( int n=0; n<actualCols; n++ ){
			if (n<lineStringList.size()) {
				const double value = lineStringList.at(n).toDouble(&isNumber);
				if (dataSource != NULL)
					isNumber ? dataPointers[n]->operator[](0) = value : dataPointers[n]->operator[](0) = NAN;
				else
					isNumber ? dataString<<QString::number(value)<<" " : dataString<<QLatin1String("NAN ");
			} else {
				if (dataSource != NULL)
					dataPointers[n]->operator[](0) = NAN;
				else
					dataString<<QLatin1String("NAN ");
			}
		}
		dataString<<"\n";
		currentRow++;
	}

	//Read the remainder of the file.
	for (int i=currentRow; i<qMin(lines,actualRows); i++){
		line = in.readLine();

		if(simplifyWhitespacesEnabled)
			line = line.simplified();

		//skip empty lines
		if (line.isEmpty())
			continue;

		if( line.startsWith(commentCharacter) == true ){
			currentRow++;
			continue;
		}

		lineStringList = line.split( separator, QString::SplitBehavior(skipEmptyParts) );

		// TODO : read strings (comments) or datetime too
		for ( int n=0; n<actualCols; n++ ){
			if (n<lineStringList.size()) {
				const double value = lineStringList.at(n).toDouble(&isNumber);
				if (dataSource != NULL)
					isNumber ? dataPointers[n]->operator[](currentRow) = value : dataPointers[n]->operator[](currentRow) = NAN;
				else
					isNumber ? dataString<<QString::number(value)<<" " : dataString<<QString("NAN ");
			} else {
				if (dataSource != NULL)
					dataPointers[n]->operator[](currentRow) = NAN;
				else
					dataString<<QLatin1String("NAN ");
			}
		}

		dataString<<"\n";
		currentRow++;
		emit q->completed(100*currentRow/actualRows);
	}

	if (!dataSource)
		return dataString.join("");

	//make everything undo/redo-able again
	//set the comments for each of the columns
	Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
	if (spreadsheet) {
		//TODO: generalize to different data types
		QString comment = i18np("numerical data, %1 element", "numerical data, %1 elements", headerEnabled ? currentRow : currentRow+1);
		for ( int n=startColumn; n<=endColumn; n++ ){
			Column* column = spreadsheet->column(columnOffset+n-startColumn);
			column->setComment(comment);
			column->setUndoAware(true);
			if (mode==AbstractFileFilter::Replace) {
				column->setSuppressDataChangedSignal(false);
				column->setChanged();
			}
		}
		spreadsheet->setUndoAware(true);
		return dataString.join("");
	}


	Matrix* matrix = dynamic_cast<Matrix*>(dataSource);
	if (matrix) {
		matrix->setSuppressDataChangedSignal(false);
		matrix->setChanged();
		matrix->setUndoAware(true);
	}

	return dataString.join("");
}

/*!
    reads the content of the file \c fileName to the data source \c dataSource.
*/
void AsciiFilterPrivate::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode){
	readData(fileName,dataSource,mode);
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void AsciiFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource){
	Q_UNUSED(fileName);
	Q_UNUSED(dataSource);
	//TODO
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
  Saves as XML.
 */
void AsciiFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement( "asciiFilter" );
	writer->writeAttribute( "commentCharacter", d->commentCharacter );
	writer->writeAttribute( "separatingCharacter", d->separatingCharacter );
	writer->writeAttribute( "autoMode", QString::number(d->autoModeEnabled) );
	writer->writeAttribute( "header", QString::number(d->headerEnabled) );
	writer->writeAttribute( "vectorNames", d->vectorNames );
	writer->writeAttribute( "skipEmptyParts", QString::number(d->skipEmptyParts) );
	writer->writeAttribute( "simplifyWhitespaces", QString::number(d->simplifyWhitespacesEnabled) );
	writer->writeAttribute( "transposed", QString::number(d->transposed) );
	writer->writeAttribute( "startRow", QString::number(d->startRow) );
	writer->writeAttribute( "endRow", QString::number(d->endRow) );
	writer->writeAttribute( "startColumn", QString::number(d->startColumn) );
	writer->writeAttribute( "endColumn", QString::number(d->endColumn) );
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool AsciiFilter::load(XmlStreamReader* reader) {
	if(!reader->isStartElement() || reader->name() != "asciiFilter"){
		reader->raiseError(i18n("no ascii filter element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs = reader->attributes();

	QString str = attribs.value("commentCharacter").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'commentCharacter'"));
	else
		d->commentCharacter = str;

	str = attribs.value("separatingCharacter").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'separatingCharacter'"));
	else
		d->separatingCharacter = str;

	str = attribs.value("autoMode").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'autoMode'"));
	else
		d->autoModeEnabled = str.toInt();

	str = attribs.value("header").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'header'"));
	else
		d->headerEnabled = str.toInt();

	str = attribs.value("vectorNames").toString();
	d->vectorNames = str; //may be empty

	str = attribs.value("simplifyWhitespaces").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'simplifyWhitespaces'"));
	else
		d->simplifyWhitespacesEnabled = str.toInt();

	str = attribs.value("skipEmptyParts").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'skipEmptyParts'"));
	else
		d->skipEmptyParts = str.toInt();

	str = attribs.value("transposed").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'transposed'"));
	else
		d->transposed = str.toInt();

	str = attribs.value("startRow").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'startRow'"));
	else
		d->startRow = str.toInt();

	str = attribs.value("endRow").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'endRow'"));
	else
		d->endRow = str.toInt();

	str = attribs.value("startColumn").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'startColumn'"));
	else
		d->startColumn = str.toInt();

	str = attribs.value("endColumn").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'endColumn'"));
	else
		d->endColumn = str.toInt();

	return true;
}

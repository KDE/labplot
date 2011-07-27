/***************************************************************************
File                 : AsciiFilter.cpp
Project              : LabPlot/SciDAVis
Description          : ASCII I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2009 by Stefan Gerlach
Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de

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
#include "AsciiFilter.h"
#include "AsciiFilterPrivate.h"
#include "datasources/FileDataSource.h"
#include "core/column/Column.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QProgressDialog>
#include <QDebug>

 /*!
	\class AsciiFilter
	\brief Manages the import/export of data organized as columns (vectors) from/to an ASCII-file.

	\ingroup datasources
 */

AsciiFilter::AsciiFilter():AbstractFileFilter(), d(new AsciiFilterPrivate){

}

AsciiFilter::~AsciiFilter(){
	delete d;
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

}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void AsciiFilter::saveFilterSettings(const QString& filterName) const{

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

  QFile file(fileName);
  if ( !file.exists() )
	  return 0;

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	  return 0;

  QTextStream in(&file);
  line = in.readLine();
  lineStringList = line.split( QRegExp("\\s+"));
  return lineStringList.size();
}


/*!
  returns the number of lines in the file \c fileName.
*/
long AsciiFilter::lineNumber(const QString & fileName){
  //TODO: compare the speed of this function with the speed of wc from GNU-coreutils.
  QFile file(fileName);
  if ( !file.exists() )
	  return 0;

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	  return 0;

  QTextStream in(&file);
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

void AsciiFilter::setEmptyLinesEnabled(bool b){
  d->emptyLinesEnabled=b;
}

bool AsciiFilter::emptyLinesEnabled() const{
  return d->emptyLinesEnabled;
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

//##########  Private-implementation  ############
AsciiFilterPrivate::AsciiFilterPrivate(){
    //default values
  transposed=false;;
  commentCharacter="#";
  separatingCharacter="auto";
  autoModeEnabled=true;
  headerEnabled=true;
  emptyLinesEnabled=true;
  simplifyWhitespacesEnabled=true;
  filterName="";

  startRow=0;
  endRow=-1;
  startColumn=0;
  endColumn=-1;
}

AsciiFilterPrivate::~AsciiFilterPrivate(){

}

/*!
    reads the content of the file \c fileName to the data source \c dataSource.
    Uses the settings defined in the data source.
*/
//TODO : read strings (comments) or datetime too
//TODO remove QProgressDialog and use QProgressBar in the main window instead
//TODO the terms vector and column are used synonymously, straighten the notation
void AsciiFilterPrivate::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode){
	int currentRow=0; //indexes the position in the vector(column)
	int columnOffset=0; //indexes the "start column" in the spreadsheet. Starting from this column the data will be imported.
	QString line;
	QStringList lineStringList;

	QFile file(fileName);
	if ( !file.exists() )
		return;

     if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

	QTextStream in(&file);
	QProgressDialog progressDialog( QObject::tr("Reading ASCII data ..."),
                                    QObject::tr("Cancel"), 0, 0 );
	progressDialog.setWindowModality(Qt::WindowModal);


	QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
	QApplication::processEvents(QEventLoop::AllEvents, 100);
	//TODO implement 
	// if (transposed)
	//...
	
	//skip rows, if required
	for (int i=0; i<startRow; i++){
        //if the number of rows to skip is bigger then the actual number
		//of the rows in the file, then quit the function.
		if( in.atEnd() ){
		  QApplication::restoreOverrideCursor();
		  return;
		}
		
		in.readLine();
	}

	//parse the first row:
    //use the first row to determine the number of columns,
	//create the columns and use (optionaly) the first row to name them
    if( in.atEnd() ){
	  QApplication::restoreOverrideCursor();
	  return;
	}
	
	line = in.readLine();
	if( simplifyWhitespacesEnabled)
		line = line.simplified();

	if( separatingCharacter == "auto" ){
	  lineStringList = line.split( QRegExp("\\s+"), (QString::SplitBehavior)emptyLinesEnabled );
    }else{
	  //TODO: doesn't work...
	  separatingCharacter.replace(QString("TAB"), QString("\t"), Qt::CaseInsensitive);
	  separatingCharacter.replace(QString("SPACE"), QString(" "), Qt::CaseInsensitive);
// 	  qDebug()<<"separator"<<separatingCharacter;
	  lineStringList = line.split( separatingCharacter, (QString::SplitBehavior)emptyLinesEnabled );
    }

	if (endColumn==-1)
		endColumn=lineStringList.size()-1;

	QStringList vectorNameList;
	if ( headerEnabled ){
		vectorNameList = lineStringList;
	}else{
		//create vector names out of the vectorNames-string, if not empty
		if (vectorNames != ""){
			vectorNameList = vectorNames.split( QRegExp("\\s+") );
		}

		//if there were no (or not enough) strings provided, add the default descriptions for the columns/vectors
		if (vectorNameList.size()<=endColumn-startColumn){
			int size=vectorNameList.size();
			for (int k=0; k<=endColumn-startColumn-size; k++ )
				vectorNameList.append( "Column " + QString::number(size+k) );
		}
	}


	//make sure we have enough columns in the data source.
	//we need in total (endColumn-startColumn) columns.
	//Create new columns, if needed.
	Column * newColumn;
	dataSource->beginMacro( QObject::tr("Import from %1").arg(fileName) );
	if (mode==AbstractFileFilter::Append){
		columnOffset=dataSource->childCount<Column>();
		for ( int n=startColumn; n<=endColumn; n++ ){
			newColumn = new Column(vectorNameList.at(n), SciDAVis::Numeric);
			dataSource->addChild(newColumn);
		}			
	}else if (mode==AbstractFileFilter::Prepend){
		Column* firstColumn = dataSource->child<Column>(0);
		for ( int n=startColumn; n<=endColumn; n++ ){
			newColumn = new Column(vectorNameList.at(n), SciDAVis::Numeric);
			dataSource->insertChildBefore(newColumn, firstColumn);
		}
	}else if (mode==AbstractFileFilter::Replace){
		//replace completely the previous content of the data source with the content to be imported.
		int columns = dataSource->childCount<Column>();
		if (columns>(endColumn-startColumn)){
			//there're more columns in the data source then required
			//-> remove the superfluous columns
			for(int i=0;i<columns-(endColumn-startColumn)-1;i++){
				dataSource->removeChild(dataSource->child<Column>(0));
			}
			
			//rename the columns, that are already available
			for (int i=0; i<=endColumn-startColumn; i++){
				dataSource->child<Column>(i)->setColumnMode( SciDAVis::Numeric);
				dataSource->child<Column>(i)->setName(vectorNameList.at(startColumn+i));
			}
		}else{
			//there're are not sufficient columns in the data source
			//rename the columns, that are already available
			for (int i=0; i<columns; i++){
				dataSource->child<Column>(i)->setColumnMode( SciDAVis::Numeric);
				dataSource->child<Column>(i)->setName(vectorNameList.at(startColumn+i));
			}
			
			//create additional columns
			for(int i=0;i<=(endColumn - startColumn - columns);i++){
				newColumn = new Column(vectorNameList.at(columns+startColumn+i), SciDAVis::Numeric);
				dataSource->addChild(newColumn);
			}
		}
	}


	progressDialog.setMinimum(0);
	int numLines=AsciiFilter::lineNumber(fileName);
	
	if (endRow == -1)
		endRow=numLines;
	
	if (endRow >numLines)
	  endRow=numLines;

	if (headerEnabled)
		numLines = endRow-startRow-1;
	else
		numLines = endRow-startRow;

	progressDialog.setMaximum( numLines );
	
	//resize the spreadsheet
	Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
	if (mode==AbstractFileFilter::Replace){
		spreadsheet->clear();
		spreadsheet->setRowCount(numLines);
	}else{
		if (spreadsheet->rowCount()<numLines)
			spreadsheet->setRowCount(numLines);
	}
	
	//import the values in the first line, if they were not used as the header (as the names for the columns)
	if (!headerEnabled){
		for ( int n=startColumn; n<=endColumn; n++ ){
		 spreadsheet->column(columnOffset+n-startColumn)->setValueAt(0, lineStringList.at(n).toDouble());
	  }
	  currentRow++;
	}

	//first line in the file is parsed. Read the remainder of the file.
	for (int i=0; i<numLines; i++){
		line = in.readLine();

		if( simplifyWhitespacesEnabled )
			line = line.simplified();

		if( line.startsWith(commentCharacter) == true ){
			currentRow++;
			continue;
		}

		if( separatingCharacter == "auto" )
			lineStringList = line.split( QRegExp("\\s+"), (QString::SplitBehavior)emptyLinesEnabled );
		else
			lineStringList = line.split( separatingCharacter, (QString::SplitBehavior)emptyLinesEnabled );

		// handle empty lines correctly
		if(lineStringList.count() == 0)
			continue;

		// TODO : read strings (comments) or datetime too
		for ( int n=startColumn; n<=endColumn; n++ ){
			dataSource->child<Column>(columnOffset+n-startColumn)->setValueAt(currentRow, lineStringList.at(n).toDouble());
		}
		
		progressDialog.setValue( currentRow );
		if ( progressDialog.wasCanceled() ){
		  //TODO
		  // 			kDebug()<<"WARNING: Import canceled()"<<endl;
		  break;
		}
		currentRow++;
    }
    
    //set the comments for each of the columns
    //TODO: generalize to different data types
    QString comment;
	if (headerEnabled)
		comment = "numerical data, " + QString::number(currentRow-1) + " elements";
	else
		comment = "numerical data, " + QString::number(currentRow) + " elements";
	
	for ( int n=startColumn; n<endColumn; n++ ){
		spreadsheet->column(columnOffset+n-startColumn)->setComment(comment);
	}

	spreadsheet->endMacro();
	QApplication::restoreOverrideCursor();
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void AsciiFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource){

}

Q_EXPORT_PLUGIN2(ioasciifilter, AsciiFilter)

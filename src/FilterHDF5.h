//LabPlot : FilterHDF5.h

#ifndef FILTERHDF5_H
#define FILTERHDF5_H

#include <vector>
#include <qstring.h>
#include <qstringlist.h>
#include "MainWin.h"

#ifdef HAVE_HDF5
#include <hdf5.h>
#endif

using namespace std;

class FilterHDF5
{
public:
	FilterHDF5(QString filename=0);
	bool fileOK() { return fileok; }
	void importFile();
	int exportFile(MainWin *mw, bool all, int datatype, int order, int start, int end);
#ifdef HAVE_HDF5
	int numGroups();
	int numSets();
	int numAttributes();
	int numSetAttributes(int set);
	// QString version() { return QString(nc_inq_libvers());}
	QString groupName(int set);
	QString datasetName(int set);
	QString columnName(int set, int col);
	double Data(int set, int row, int col);		       //!< get data of column c and rows r of spreadsheet s
	int Rows(int set);
	int Cols(int set);
	QString getAttribute(int i);
	QString getSetAttribute(int set,int i);
#endif
protected:
private:
	bool fileok;			// is a hdf5 file and is readable
	QString filename;
};

#endif //FILTERHDF5_H

//LabPlot : FilterCDF.h

#ifndef FILTERCDF_H
#define FILTERCDF_H

#include <qstring.h>
#include <qstringlist.h>

#ifdef HAVE_CDF
#include <cdf.h>
#endif

class FilterCDF
{
public:
	FilterCDF(QString filename=0);
	bool fileOK() { return fileok; }
#ifdef HAVE_CDF
	QString version();
	QString copyright();
	long NDims() { return ndims; }
	QString Maj();
	QString Compression();
	long MaxRec() { return maxrec; }
	long NVars() { return nvars; }
	long NAtts() { return natts; }
	QString Att(int aid);				 	// get attribute aid
	QString Var(int varid);					// get variable varid description
	QString VarName(int varid);			// get variable varid name 
	long VarLen(QString name);
	QStringList DataString(int varid);		// data info
	double Data(QString varname, int i);
	
	QString Comp(int c);
	QString Enc(int e);
protected:
	QString Type(int t);
	bool isZvar(int varid);
	bool isZvar(QString varname);
	double Value(void *value,long type);
#endif
private:
	QString filename;			// file name
	bool fileok;				// is a netcdf file and readable
#ifdef HAVE_CDF
	CDFid id;					// file id
	long ndims, enc, nvars, natts, maj, maxrec;  
	long inquire(int type);
#endif
};

#endif //FILTERCDF_H

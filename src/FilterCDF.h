//LabPlot : FilterCDF.h

#ifndef FILTERCDF_H
#define FILTERCDF_H

#include <QString>
#include <QStringList>

#ifdef HAVE_CDF
#include <cdf.h>
#endif

class FilterCDF
{
public:
	FilterCDF(QString filename=0);
	bool fileOK() const { return fileok; }
#ifdef HAVE_CDF
	QString version();
	QString copyright();
	long NDims() const { return ndims; }
	QString Maj() const;
	QString Compression() const;
	long MaxRec() const { return maxrec; }
	long NVars() const { return nvars; }
	long NAtts() const { return natts; }
	QString Att(int aid) const;			 	// get attribute aid
	QString Var(int varid) const;				// get variable varid description
	QString VarName(int varid) const;			// get variable varid name 
	long VarLen(QString name) const;
	QStringList DataString(int varid) const;		// data info
	double Data(QString varname, int i) const;
	
	QString Comp(int c) const;
	QString Enc(int e) const;
protected:
	QString Type(int t) const;
	bool isZvar(int varid) const;
	bool isZvar(QString varname) const;
	double Value(void *value,long type);
#endif
private:
	QString filename;
	bool fileok;			//!< is a netcdf file and readable
#ifdef HAVE_CDF
	CDFid id;					// file id
	long ndims, enc, nvars, natts, maj, maxrec;  
	long inquire(int type);
#endif
};

#endif //FILTERCDF_H

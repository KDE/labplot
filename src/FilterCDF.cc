//LabPlot : FilterCDF.cc

#include <klocale.h>
#include <kdebug.h>

#include "FilterCDF.h"

FilterCDF::FilterCDF(QString filename)
	: filename(filename)
{
#ifdef HAVE_CDF
	// TODO : write CDF file ?
	if (!filename.isEmpty()) {
		CDFstatus status = CDFopen(qPrintable(filename), &id);
		if (status != CDF_OK)		// not a cdf file or not readable
			fileok=false;
		else {
			fileok=true;

			long dimsizes[CDF_MAX_DIMS];
			CDFinquire(id,&ndims,dimsizes,&enc,&maj,&maxrec,&nvars,&natts);
		}
	}
#endif
}

#ifdef HAVE_CDF
QString FilterCDF::version() {
	long version, release;
	char copy[CDF_COPYRIGHT_LEN+1];

	CDFdoc(id,&version,&release,copy);

	QString v(QString::number(version)+'.'+QString::number(release));
	return v;
}

QString FilterCDF::copyright() {
	long version, release;
	char copy[CDF_COPYRIGHT_LEN+1];

	CDFdoc(id,&version,&release,copy);

	return QString(copy);
}

QString FilterCDF::Enc(int i) {
	if(i>=0) enc=i;		// get encoding with no open file
	switch(enc) {
	case HOST_ENCODING: return QString("HOST_ENCODING");break;
	case NETWORK_ENCODING: return QString("NETWORK_ENCODING");break;
	case VAX_ENCODING: return QString("VAX_ENCODING");break;
	case ALPHAVMSd_ENCODING: return QString("ALPHAVMSd_ENCODING");break;
	case ALPHAVMSg_ENCODING: return QString("ALPHAVMSg_ENCODING");break;
	case ALPHAVMSi_ENCODING: return QString("ALPHAVMSi_ENCODING");break;
	case ALPHAOSF1_ENCODING: return QString("ALPHAOSF1_ENCODING");break;
	case SUN_ENCODING: return QString("SUN_ENCODING");break;
	case SGi_ENCODING: return QString("SGi_ENCODING");break;
	case DECSTATION_ENCODING: return QString("DECSTATION_ENCODING");break;
	case IBMRS_ENCODING: return QString("IBMRS_ENCODING");break;
	case HP_ENCODING: return QString("HP_ENCODING");break;
	case IBMPC_ENCODING: return QString("IBMPC_ENCODING");break;
	case NeXT_ENCODING: return QString("NeXT_ENCODING");break;
	case MAC_ENCODING: return QString("MAC_ENCODING");break;
	}

	return 0;
}

QString FilterCDF::Type(int t) {
	switch(t) {
	case CDF_BYTE: return QString("BYTE");break;
	case CDF_CHAR: return QString("CHAR");break;
	case CDF_INT1: return QString("INT1");break;
	case CDF_UCHAR: return QString("UCHAR");break;
	case CDF_UINT1: return QString("UINT1");break;
	case CDF_INT2: return QString("INT2");break;
	case CDF_UINT2: return QString("UINT2");break;
	case CDF_INT4: return QString("INT4");break;
	case CDF_UINT4: return QString("UINT4");break;
	case CDF_REAL4: return QString("REAL4");break;
	case CDF_FLOAT: return QString("FLOAT");break;
	case CDF_REAL8: return QString("REAL8");break;
	case CDF_DOUBLE: return QString("DOUBLE");break;
	case CDF_EPOCH: return QString("EPOCH");break;
	}

	return 0;
}

QString FilterCDF::Comp(int c) {
	switch(c) {
	case NO_COMPRESSION: return QString("NO_COMPRESSION");break;
	case RLE_COMPRESSION: return QString("RLE_COMPRESSION");break;
	case HUFF_COMPRESSION: return QString("HUFF_COMPRESSION");break;
	case AHUFF_COMPRESSION: return QString("AHUFF_COMPRESSION");break;
	case GZIP_COMPRESSION: return QString("GZIP_COMPRESSION");break;
	}

	return 0;
}

QString FilterCDF::Maj() {
	switch(maj) {
	case ROW_MAJOR : return QString("ROW_MAJOR");break;
	case COLUMN_MAJOR : return QString("COLUMN_MAJOR");break;
	}

	return 0;
}

bool FilterCDF::isZvar(int varid) {

	int status = CDFlib(SELECT_, zVAR_, varid);
   	if (status < CDF_OK) {	// Rvar
		return false;
	}
	return true;
}

bool FilterCDF::isZvar(QString varname) {
	int status = CDFlib(SELECT_, zVAR_NAME_, qPrintable(varname));
   	if (status < CDF_OK) {	// Rvar
		return false;
	}
	return true;
}

QString FilterCDF::Var(int varid) {
	char name[CDF_VAR_NAME_LEN+1];
	long type, nelems, recvary;
	long dimvary[CDF_MAX_DIMS];

	int status = CDFvarInquire(id,varid,name,&type,&nelems,&recvary,dimvary);
	if (status < CDF_OK)
		return 0;

	QString var = QString(name)+i18n(" ( ")+Type(type)+i18n(" ) ")+i18n(" : ")+QString::number(nelems);
	var += ' '+QString::number(recvary);

	return var;
}

QString FilterCDF::VarName(int varid) {
	char name[CDF_VAR_NAME_LEN+1];
	long type, nelems, recvary;
	long dimvary[CDF_MAX_DIMS];

	int status = CDFvarInquire(id,varid,name,&type,&nelems,&recvary,dimvary);
	if (status < CDF_OK)
		return 0;

	return QString(name);
}

QString FilterCDF::Att(int aid) {
	char name[CDF_ATTR_NAME_LEN+1];
	long scope, entry;

	CDFattrInquire(id,aid,name,&scope,&entry);

	QString att = QString(name)+i18n(" : ")+QString::number(scope)+' '+QString::number(entry);
	return att;
}

QString FilterCDF::Compression( ) {
	long type, pct;
	long parms[CDF_MAX_PARMS];

	CDFlib(SELECT_,CDF_,id,GET_,CDF_COMPRESSION_,&type, parms, &pct);

	QString c = Comp(type)+i18n(" ( ")+QString::number(pct)+QString(" % )");

	return c;
}

QStringList FilterCDF::DataString(int varid){
	QStringList list;
	QString line;
  	long type;
	maxrec=0;
	bool iszvar = isZvar(varid);

	// get type
	if(iszvar) {
		CDFlib(SELECT_, zVAR_, varid, GET_, zVAR_DATATYPE_, &type, NULL_);
		CDFlib (GET_, zVAR_MAXREC_, &maxrec, NULL_);
  	}
	else {
		CDFlib(SELECT_, rVAR_, varid, GET_, rVAR_DATATYPE_, &type, NULL_);
		CDFlib (GET_, rVAR_MAXREC_, &maxrec, NULL_);
	}

	double v;
	long seqpos;
	void *binary=new double;
	long indices[CDF_MAX_DIMS];

	kdDebug() << "Reading " << maxrec << " value(s) from "<<Var(varid)<<" of type "<<Type(type)<<endl;

	for (int i = 0; i < maxrec; i++) {	// maxrec+1 valid value ?
		if (iszvar) {
			// TODO : check this
			CDFlib (SELECT_, zVAR_RECNUMBER_, (long) i, GET_, zVAR_DATA_, binary, NULL_);
		}
		else {
			CDFlib(CONFIRM_,rVAR_SEQPOS_,&seqpos,indices);
			//kdDebug()<<"rVAR seqpos = "<<seqpos<<endl;
			CDFlib(SELECT_, rVAR_SEQPOS_, (long) i, indices);
			CDFlib(GET_, rVAR_SEQDATA_, binary, NULL_);
		}
		v = Value(binary,type);

 	    	//kdDebug() << getVar(varid) << "[" << i << "]=" << v << endl;
		line += QString::number(v)+' ';
	}

	list += line;
	return list;
}

long FilterCDF::VarLen(QString name) {
  	long maxrec=0;

	int status = CDFlib(SELECT_, zVAR_NAME_, qPrintable(name));
	if (status < CDF_OK)	// rvar
		CDFlib (SELECT_,rVAR_NAME_,qPrintable(name),GET_, rVAR_MAXREC_, &maxrec, NULL_);
	else					// zvar
		CDFlib (GET_, zVAR_MAXREC_, &maxrec, NULL_);

	return maxrec;
}

double FilterCDF::Value(void *value,long type) {
	double v=0;

	switch (type) {
	case CDF_INT2: v = (double) *((short *) value);break;
	case CDF_INT4: v = (double) *((int *) value); break;
	case CDF_UINT1: v = (double) *((unsigned char *) value); break;
	case CDF_UINT2: v = (double) *((unsigned short *) value); break;
	case CDF_UINT4: v = (double) *((unsigned int *) value); break;
	case CDF_REAL4: case CDF_FLOAT: v = (double) *((float *) value); break;
	case CDF_REAL8: case CDF_DOUBLE:  v = (double) *((double *) value); break;
	}
	return v;
}

double FilterCDF::Data(QString varname, int i){
	if(i>VarLen(varname))
		return 0;

	// get type
	long type;
	if(isZvar(qPrintable(varname)))
		CDFlib(SELECT_, zVAR_NAME_, qPrintable(varname), GET_, zVAR_DATATYPE_, &type, NULL_);
  	else
		CDFlib(SELECT_, rVAR_NAME_, qPrintable(varname), GET_, rVAR_DATATYPE_, &type, NULL_);

	void *binary = new double;
	long seqpos, indices[CDF_MAX_DIMS];

	if (isZvar(qPrintable(varname))) {
		// TODO : check this
		CDFlib (SELECT_, zVAR_RECNUMBER_, (long) i, GET_, zVAR_DATA_, binary,NULL_);
	}
	else {
		CDFlib(CONFIRM_,rVAR_SEQPOS_,&seqpos,indices);
		CDFlib (SELECT_, rVAR_SEQPOS_, (long) i,indices);
		CDFlib(GET_, rVAR_SEQDATA_, binary,NULL_);
    	}

	return  Value(binary,type);
}
#endif


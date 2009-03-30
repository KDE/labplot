/***************************************************************************
    File                 : FilterNETCDF.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : NetCDF import/export filter
                           
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

#include <cstdlib>
#include <KLocale>
#include <KDebug>

#include "FilterNETCDF.h"

FilterNETCDF::FilterNETCDF(QString fn)
	: filename(fn)
{
	fileok=false;
#ifdef HAVE_NETCDF
	int status = nc_open(filename.toAscii(), NC_NOWRITE, &ncid);
	if (status == NC_NOERR)	{	// a netcdf file and readable
		fileok=true;

		nc_inq(ncid, &ndims, &nvars, &natts, &xdimid);

		for (int i=0;i<ndims;i++)
			nc_inq_dim(ncid, i, dims[i].name, &dims[i].size);
	}
#endif
}

#ifdef HAVE_NETCDF
//! return type name
QString FilterNETCDF::typeName(nc_type type) const{
    switch (type) {
      case NC_BYTE:
	return i18n("byte");
      case NC_CHAR:
	return i18n("char");
      case NC_SHORT:
	return i18n("short");
      case NC_INT:
	return i18n("int");
      case NC_FLOAT:
	return i18n("float");
      case NC_DOUBLE:
	return i18n("double");
      default:
	return i18n("bogus");
    }
}

QString FilterNETCDF::Dim(int dimid) const{
	if (dimid == xdimid)
		return (QString(dims[dimid].name)+QString(" = ")+i18n("UNLIMITED")+QString(" (")+
				QString::number(dims[dimid].size)+QString(")"));
	else
		return (QString(dims[dimid].name)+QString(" = ")+QString::number(dims[dimid].size));

}

QString FilterNETCDF::Var(int varid) {
	QString line;

	ncvar var;

	nc_inq_var(ncid, varid, var.name, &var.type, &var.ndims, var.dims, &var.natts);
	line += typeName(var.type)+' '+QString(var.name)+'(';
	for (int id = 0; id < var.ndims; id++) {
		line +=QString(dims[var.dims[id]].name);
		if (id < var.ndims-1)
			line += QString(", ");
	}
	line +=QString(") ");

	// get variable attributes
	for (int ia = 0; ia < var.natts; ia++)
		line += QString("   ") + pr_att(ncid, varid, var.name, ia);

	return line;
}

QString FilterNETCDF::VarName(int varid) const{
	ncvar var;

	nc_inq_var(ncid, varid, var.name, &var.type, &var.ndims, var.dims, &var.natts);
	return QString(var.name);
}

//! return length of variable var
int FilterNETCDF::VarLen(QString name) const{
	ncvar var;
	int varid;

	nc_inq_varid(ncid,name.toAscii(),&varid);	// get varid
	if (varid>nvars)
		return 0;

	nc_inq_var(ncid, varid, var.name, &var.type, &var.ndims, var.dims, &var.natts);

	if (var.ndims==1)
		return dims[var.dims[0]].size;
	if (var.ndims==2)
		return dims[var.dims[0]].size*dims[var.dims[1]].size;
	if (var.ndims==3)
		return dims[var.dims[0]].size*dims[var.dims[1]].size*dims[var.dims[2]].size;
	if (var.ndims==4)
		return dims[var.dims[0]].size*dims[var.dims[1]].size*dims[var.dims[2]].size*dims[var.dims[3]].size;

	return 0;
}

QStringList FilterNETCDF::DataString(int varid) const{
	QString line;
	QStringList list;
	double *data;
	ncvar var;
	nc_inq_var(ncid, varid, var.name, &var.type, &var.ndims, var.dims, &var.natts);

	if(var.ndims==0)
		return QStringList();

	int sizex = dims[var.dims[0]].size;

	// TODO : get value corresponding to type (double, short, int, ...)
	if (var.ndims==1) {
		data = new double[sizex];
		nc_get_var_double(ncid,varid,data);
		line=QString(" ");
		for (unsigned int i=0;i<dims[var.dims[0]].size;i++)
			line += QString::number(data[i])+' ';
		list += line;//selvarlb->insertItem(line);
	}
	else if (var.ndims==2) {
		int sizey = dims[var.dims[1]].size;
		data = new double[sizex*sizey];
		//kDebug()<<"x/y size : "<<sizex<<'/'<<sizey<<endl;
		for (int j=0;j<sizey;j++) {
			line=QString(" ");
			for (int i=0;i<sizex;i++) {
				nc_get_var_double(ncid,varid,data);
				//kDebug()<<"value ("<<i<<','<<j<<") : "<<data[i+j*sizex]<<" / index = "<<index<<endl;
				line += QString::number(data[i+j*sizex])+' ';
			}
			list += line;//selvarlb->insertItem(line);

		}
	}
	else {
		//TODO : care about more data
	}

	return list;
}

double FilterNETCDF::Data(QString name, const size_t index) const{
	ncvar var;
	int varid;

	nc_inq_varid(ncid,name.toAscii(),&varid);	// get varid
//	kDebug()<<"varid = "<<varid<<endl;
	if (varid>nvars)
		return 0;

	nc_inq_var(ncid, varid, var.name, &var.type, &var.ndims, var.dims, &var.natts);

	double value=0;

	if(var.ndims==1) {
		size_t i[1]; i[0]=index;
		nc_get_var1_double(ncid,varid,i,&value);
	}
	else if(var.ndims==2) {
		int sizey = dims[var.dims[1]].size;
		size_t i[2];
		i[0]=index/sizey;i[1]=index%sizey;
		nc_get_var1_double(ncid,varid,i,&value);
	}
	else {
		// TODO
	}

//	kDebug()<<"var "<<varid<<"("<<index<<") = "<<value<<endl;

	return value;
}

// Print attribute string, for text attributes.
QString FilterNETCDF::pr_att_string(size_t len,const char *string) {
	QString line;
	const char *cp;
	const char *sp;
	unsigned char uc;

	cp = string;
	line += "\"";
	/* adjust len so trailing nulls don't get printed */
	sp = cp + len - 1;
	while (len != 0 && *sp-- == '\0')
		len--;
	for (unsigned int iel = 0; iel < len; iel++)
		switch (uc = *cp++ & 0377) {
		case '\b':
			line +="\b";
			printf ("\\b");break;
		case '\f':
			line +="\f";
			printf ("\\f");break;
		case '\n':		/* generate linebreaks after new-lines */
			line +=QString("\\n\",\n    \"");
			printf ("\\n\",\n    \"");break;
		case '\r':
			line +="\r";
			printf ("\\r");break;
		case '\t':
			line +="\t";
			printf ("\\t");break;
		case '\v':
			line +="\v";
			printf ("\\v");break;
		case '\\':
			line +="\\";
			printf ("\\\\");break;
		case '\'':
			line +="\'";
			printf ("\\'");break;
		case '\"':
			line +="\"";
			printf ("\\\"");break;
		default:
			line += uc;
			printf ("%c",uc);break;
	}
	line +="\"";
	return line;
}

// Print list of attribute values, for numeric attributes.
QString FilterNETCDF::pr_att_vals(nc_type type, size_t len, const double *vals) {
	QString line;

	for (unsigned int i = 0; i < len; i++) {
		line += QString::number(vals[i]);
		if (type ==NC_BYTE )
			line +='b';
		else if (type ==NC_SHORT )
			line +='s';
		else if (type ==NC_FLOAT )
			line +='f';

		if (i<len-1)
			line +=QString(", ");
    	}
	return line;
}

QString FilterNETCDF::pr_att(int incid, int varid, const char *, int ia) {
	QString line;
	struct ncatt att;		// attribute

	nc_inq_attname(incid, varid, ia, att.name);
	line += QString(att.name)+QString(" = ");

	nc_inq_att(incid, varid, att.name, &att.type, &att.len);

	if (att.len == 0) {	// show 0-length attributes as empty strings
		att.type = NC_CHAR;
		att.len = 1;
	}
	switch (att.type) {
	case NC_CHAR:
		att.string = (char *) malloc(att.len);
		if (!att.string) {
			nc_close(incid);
			return line;
		}
		nc_get_att_text(incid, varid, att.name, att.string);
		line += pr_att_string(att.len, att.string);
		free(att.string);
		break;
	default:
		att.vals = (double *) malloc(att.len * sizeof(double));
		if (!att.vals) {
		    nc_close(incid);
		    return line;
		}
		nc_get_att_double(incid, varid, att.name, att.vals );
		line += pr_att_vals(att.type, att.len, att.vals);
		free(att.vals);
		break;
	}

	return line;
}

#endif

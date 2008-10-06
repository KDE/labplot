/***************************************************************************
    File                 : FilterHDF5.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : HDF5 import/export filter
                           
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

#include "FilterHDF5.h"

using namespace std;

#ifdef HAVE_HDF5
int numgroups=0, numsets=0, numattr=0;
vector < vector < vector <double> > > data;
vector < vector <int> > dim;
// TODO vector < vector <int> > types;
vector < vector <QString> > columnname;
vector <QString> groups;
vector <QString> sets;
vector <QString> attributes;
vector < vector <QString> > setattributes;

int FilterHDF5::numSets() const {
	return numsets;
}

int FilterHDF5::numGroups() const{
	return numgroups;
}

int FilterHDF5::numAttributes() const{
	return attributes.size();
}

int FilterHDF5::numSetAttributes(int set) const{
	return setattributes[set].size();
}

QString FilterHDF5::groupName(int set) const{
	return groups[set];
}

QString FilterHDF5::datasetName(int set) const{
	return sets[set];
}

QString FilterHDF5::columnName(int set, int col) const{
	return columnname[set][col];
}

QString FilterHDF5::getAttribute(int i) const{
	if(i>numAttributes())
		return 0;
	return attributes[i];
}

QString FilterHDF5::getSetAttribute(int set, int i) const{
	if(i>numSetAttributes(set))
		return 0;
	return setattributes[set][i];
}

double FilterHDF5::Data(int set, int row, int col) const { return data[set][row][col]; }

int FilterHDF5::Rows(int s) const { return dim[s].size(); }
int FilterHDF5::Cols(int s) const{
	// sanity check
	if(dim[s][0]>100000)
		return 100000;
	return dim[s][0];
}	// number of cols of first row

herr_t scanfile(hid_t loc_id, const char *name, void *) {
	// printf("scanfile(%s)\n",name);
	H5G_stat_t statbuf;
	H5Gget_objinfo(loc_id, name, 0, &statbuf);
	switch (statbuf.type) {
	case H5G_GROUP: {
		printf(" GROUP \"%s\" \n", name);
		numgroups++;

		hid_t group = H5Gopen(loc_id, name);

		int nrattr = H5Aget_num_attrs(group);
		printf(" GROUP has %d attributes\n",nrattr);

		int oldsize = attributes.size();
		attributes.resize(oldsize+nrattr+1);
		attributes[oldsize] = QString("GROUP "+ QString(name) +":");

		for(int i=0;i<nrattr;i++) {
			hid_t attr = H5Aopen_idx(group, i);
			char tmp[100];
			H5Aget_name(attr,100,tmp);
			hid_t type = H5Aget_type(attr);

			hid_t ret;
			switch(H5Tget_class(type)) {
			case H5T_INTEGER: {
				long long value;
				ret = H5Aread(attr, type, &value);
				char tmp2[1000];
				sprintf(tmp2,"	%s = %lld",tmp,value);
				attributes[oldsize+i+1]=QString(tmp2);
				}; break;
			case H5T_FLOAT: {
				double value;
				ret = H5Aread(attr, type, &value);
				char tmp2[1000];
				sprintf(tmp2,"	%s = %g",tmp,value);
				attributes[oldsize+i+1]=QString(tmp2);
				}; break;
			case H5T_STRING: {
				char value[1000];
				ret = H5Aread(attr, type, &value);
				char tmp2[1100];
				sprintf(tmp2,"	%s = %s",tmp,value);
				attributes[oldsize+i+1]=QString(tmp2);
				}; break;
			default:
				 printf("UNKNOWN ATTRIBUTE\n");
				 break;
			}

			ret = H5Aclose(attr);
		}

		H5Giterate(group, ".", NULL, scanfile, NULL);
		H5Gclose(group);
		}; break;
	case H5G_DATASET: {
		printf("	DATASET \"%s\"\n", name);

		hid_t dataset;
		dataset = H5Dopen(loc_id, name);

		numsets++;
		data.resize(numsets);
		dim.resize(numsets);
		sets.resize(numsets);
		columnname.resize(numsets);
		setattributes.resize(numsets);
		sets[numsets-1]=QString(name);

		// dataset attributes
		int nrattr = H5Aget_num_attrs(dataset);
		printf("	DATASET has %d attributes\n",nrattr);
		setattributes[numsets-1].resize(nrattr);
		for(int i=0;i<nrattr;i++) {
			hid_t attr = H5Aopen_idx(dataset, i);
			char tmp[100];
			H5Aget_name(attr,100,tmp);
			hid_t type = H5Aget_type(attr);

			hid_t ret;
			switch(H5Tget_class(type)) {
			case H5T_INTEGER: {
				long long value;
				ret = H5Aread(attr, type, &value);
				// printf("The value of integer attribute %s is %lld\n", tmp,value);
				char tmp2[1000];
				sprintf(tmp2,"%s = %lld",tmp,value);
				setattributes[numsets-1][i]=QString(tmp2);
				}; break;
			case H5T_FLOAT: {
				double value;
				ret = H5Aread(attr, type, &value);
				// printf("The value of float attribute %s is %g\n", tmp, value);
				char tmp2[1000];
				sprintf(tmp2,"%s = %g",tmp,value);
				setattributes[numsets-1][i]=QString(tmp2);
				}; break;
			case H5T_STRING: {
				char value[1000];
				ret = H5Aread(attr, type, &value);
				// printf("The value of string attribute %s is %s\n", tmp, value);
				char tmp2[1100];
				sprintf(tmp2,"%s = %s",tmp,value);
				setattributes[numsets-1][i]=QString(tmp2);
				}; break;
			default:
				 printf("UNKNOWN ATTRIBUTE\n");
				 break;
			}

			ret = H5Aclose(attr);
		}

		hid_t datatype = H5Dget_type(dataset);     // datatype identifier
		H5T_class_t dataclass = H5Tget_class(datatype);

		size_t size = H5Tget_size(datatype);
		printf("		Data size is %d \n", size);

		int DX,DY;
		switch (dataclass) {
		case H5T_COMPOUND: {
			printf("		Data set has COMPOUND type\n");
			printf(" 		Number of Members = %d \n",H5Tget_nmembers(datatype));

			hid_t dataspace = H5Dget_space(dataset);    // dataspace identifier
			int rank = H5Sget_simple_extent_ndims(dataspace);
			hsize_t dims_out[2];
			int status = H5Sget_simple_extent_dims(dataspace, dims_out, NULL);
			printf("		rank %d, dimensions %lu , status=%d\n", rank,
					(unsigned long)(dims_out[0]), status);

			DY = dims_out[0];
			DX = H5Tget_nmembers(datatype);
			data[numsets-1].resize(DY);
			dim[numsets-1].resize(DY);
			columnname[numsets-1].resize(DX);
			for(int i=0;i<DY;i++) {
				data[numsets-1][i].resize(DX);
				dim[numsets-1][i]=DX;
			}

			for(int j=0;j < DX;j++) {
				printf("		Member %d = %s \n",j+1, H5Tget_member_name (datatype,j));
				columnname[numsets-1][j] = QString(H5Tget_member_name (datatype,j));

				hid_t dtype = H5Tget_member_type(datatype,j);
				switch(H5Tget_class(dtype)) {
				case H5T_INTEGER:{
					printf("			Member has INTEGER type\n");
					int *matrix = new int[DY];
					hid_t s3_tid = H5Tcreate(H5T_COMPOUND, sizeof(int));
					status = H5Tinsert(s3_tid, H5Tget_member_name (datatype,j), 0, H5T_NATIVE_INT);
					status = H5Dread(dataset, s3_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, matrix);
					for(int i = 0; i < 10; i++)
						printf("%d ", matrix[i]);
					printf("... \n");
					for (int i=0; i < DY; i++)
						data[numsets-1][i][j] = (double) matrix[i];
					delete [] matrix;

					};break;
				case H5T_FLOAT: {
					printf("			Member has FLOAT type\n");
					float *matrix = new float[DY];
					hid_t s3_tid = H5Tcreate(H5T_COMPOUND, sizeof(float));

					status = H5Tinsert(s3_tid, H5Tget_member_name (datatype,j), 0, H5T_NATIVE_FLOAT);
					status = H5Dread(dataset, s3_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, matrix);
					for(int i = 0; i < 10; i++)
						printf("%.4f ", matrix[i]);
					printf("... \n");
					for (int i=0; i < DY; i++)
						data[numsets-1][i][j] = (double) matrix[i];

					delete [] matrix;

					};break;
				case H5T_ARRAY: {
					int ndims = H5Tget_array_ndims(dtype);
					printf("			Member has ARRAY type (ndims=%d)\n",ndims);
					// H5Tget_array_dims( dtype, hsize_t *dims[], NULL );

					hid_t super = H5Tget_super(dtype);
					switch(H5Tget_class(super)) {
					case H5T_INTEGER:{
						printf("			Array has INTEGER type\n");

						int *matrix = new int[DY];
						hsize_t tdims[] = {1};
						hid_t s4_tid = H5Tarray_create(H5T_NATIVE_INT,1,tdims,NULL);
						hid_t s3_tid = H5Tcreate(H5T_COMPOUND, sizeof(s4_tid));
						status = H5Tinsert(s3_tid, H5Tget_member_name (datatype,j), 0, s4_tid);

						status = H5Dread(dataset, s3_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, matrix);
						// printf("H5Dread()=%d\n",status);
						for(int i = 0; i < 10; i++)
							printf("%d ", matrix[i]);
						printf("... \n");
						for (int i=0; i < DY; i++)
							data[numsets-1][i][j] = (double) matrix[i];

						delete [] matrix;
						};break;
					case H5T_FLOAT: {
						printf("			Array has FLOAT type\n");

						float *matrix = new float[DY];
						hsize_t tdims[] = {1};
						hid_t s4_tid = H5Tarray_create(H5T_NATIVE_FLOAT,1,tdims,NULL);
						hid_t s3_tid = H5Tcreate(H5T_COMPOUND, sizeof(s4_tid));
						status = H5Tinsert(s3_tid, H5Tget_member_name (datatype,j), 0, s4_tid);

						status = H5Dread(dataset, s3_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, matrix);
						// printf("H5Dread()=%d\n",status);
						for(int i = 0; i < 10; i++)
							printf("%.4f ", matrix[i]);
						printf("... \n");
						for (int i=0; i < DY; i++)
							data[numsets-1][i][j] = (double) matrix[i];

						delete [] matrix;

						}; break;
					default:
						printf("			Array has UNKNOWN type %d\n",H5Tget_class(super));
						break;
					}

					}; break;
				case H5T_TIME:
					printf("			Member has TIME type\n");
					printf("			NOT SUPPORTED YET !!!\n");
					break;
				case H5T_STRING:
					printf("			Member has STRING type\n");
					printf("			NOT SUPPORTED YET !!!\n");

					// TODO : read string values
					/*
					char *matrix=???;
					hid_t s3_tid = H5Tcreate(H5T_COMPOUND, sizeof(char)*DX);

					status = H5Tinsert(s3_tid, H5Tget_member_name (datatype,j), 0, H5T_NATIVE_STRING);
					status = H5Dread(dataset, s3_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, matrix);
					for(int i = 0; i < 10; i++)
						printf("%.4f ", matrix[i]);
					*/
				default:
					printf("		Member has UNKNOWN type %d\n",H5Tget_member_type (datatype,j));
				}
			}

			}; break;
		case H5T_INTEGER:
			printf("		Data set has INTEGER type\n");
			// data read later
			break;
		case H5T_FLOAT:
			printf("		Data set has FLOAT type\n");
			// data read later
			break;
		case H5T_ARRAY:
			printf("		Data set has ARRAY type\n");
			printf("		NOT SUPPORTED YET !!!\n");
			break;
		case H5T_TIME:
			printf("		Data set has TIME type\n");
			printf("		NOT SUPPORTED YET !!!\n");
			break;
		case H5T_STRING: {
			printf("		Data set has STRING type\n");
			// imported into notes field	(only one data value ???)

			char *tmp = (char *) malloc((size+1)*sizeof(char));
			tmp[size]='\0';
			hid_t err = H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, tmp);
			printf("	%s = %s (err=%d)\n",name,tmp,err);

			//import into notes
			setattributes[numsets-1].resize(nrattr+2);	// add string to notes
			setattributes[numsets-1][nrattr]=QString(QString(name) + " :");
			setattributes[numsets-1][nrattr+1]=QString(tmp);
			free(tmp);
			}; break;
		case H5T_BITFIELD:
			printf("		Data set has BITFIELD type\n");
			printf("		NOT SUPPORTED YET !!!\n");
			break;
		case H5T_OPAQUE:
			printf("		Data set has OPAQUE type\n");
			printf("		NOT SUPPORTED YET !!!\n");
			break;
		case H5T_REFERENCE:
			printf("		Data set has REFERENCE type\n");
			printf("		NOT SUPPORTED YET !!!\n");
			break;
		case H5T_ENUM:
			printf("		Data set has ENUM type\n");
			printf("		NOT SUPPORTED YET !!!\n");
			break;
		case H5T_VLEN:
			printf("		Data set has VLEN type\n");
			printf("		NOT SUPPORTED YET !!!\n");
			break;
		default:
			printf("		Date set has UNKNOWN type %d\n",dataclass);
			break;
		}

		if(dataclass == H5T_INTEGER || dataclass == H5T_FLOAT) {

			hid_t dataspace = H5Dget_space(dataset);    // dataspace identifier
			int rank = H5Sget_simple_extent_ndims(dataspace);
			hsize_t dims_out[2];
			int status = H5Sget_simple_extent_dims(dataspace, dims_out, NULL);
			printf("		rank %d, dimensions %lu x %lu, status=%d\n", rank,
					(unsigned long)(dims_out[0]), (unsigned long)(dims_out[1]), status);

			DY=dims_out[0];
			DX=dims_out[1];
			printf("DX/DY = %d/%d\n",DX,DY);
			columnname[numsets-1].resize(DX);
			data[numsets-1].resize(DY);
			dim[numsets-1].resize(DY);
			for(int i=0;i<DY;i++) {
				data[numsets-1][i].resize(DX);
				dim[numsets-1][i]=DX;
			}

			H5T_order_t dataorder = H5Tget_order(datatype);
			switch (dataorder) {
				case H5T_ORDER_LE:
					printf("		Little endian order \n");
					break;
				case H5T_ORDER_BE:
					printf("		Big endian order \n");
					break;
				default:
					printf("		Unknown order \n");
			}

			herr_t err=0;
			switch(dataclass) {
			case H5T_INTEGER: {
				int *matrix = new int[DX*DY];
				err = H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, matrix);
				for(int i=0;i<DY;i++) {
					for (int j=0; j < DX; j++)
						data[numsets-1][i][j] = (double) matrix[j+i*DX];
				}
				}; break;
			case H5T_FLOAT: {
				double *matrix = new double[DX*DY];
				err = H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, matrix);
				for(int i=0;i<DY;i++) {
					for (int j=0; j < DX; j++)
						data[numsets-1][i][j] = matrix[j+i*DX];
				}
				}; break;
			default: break;
			}

			printf("status = %d\n",err);
		}

		}; break;
	case H5G_TYPE:
		printf("	DATATYP \"%s\"\n", name);
		break;
	default:
		printf(" 	Unable to identify an object\n");
	}

	return 0;
}

#endif

FilterHDF5::FilterHDF5(QString fn)
	: filename(fn)
{
	fileok=false;

#ifdef HAVE_HDF5
	// try to open the file
	if( H5Fis_hdf5(fn.toAscii()) )
		fileok=true;
#endif
}

void FilterHDF5::importFile() {
#ifdef HAVE_HDF5
	kDebug()<< "Opening hdf5 file"<<endl;
	hid_t file = H5Fopen(filename.toAscii(), H5F_ACC_RDONLY, H5P_DEFAULT);

	printf(" ROOT GROUP :\n");
	hid_t group = H5Gopen(file, "/");

	int nrattr = H5Aget_num_attrs(group);
	attributes.resize(nrattr);
	printf(" ROOT GROUP has %d attributes\n",nrattr);
	for(int i=0;i<nrattr;i++) {
		hid_t attr = H5Aopen_idx(group, i);
		char tmp[100];
		H5Aget_name(attr,100,tmp);
		hid_t type = H5Aget_type(attr);

		hid_t ret;
		switch(H5Tget_class(type)) {
			case H5T_INTEGER: {
				long long value;
				ret = H5Aread(attr, type, &value);
				// printf("The value of integer attribute %s is %lld\n", tmp,value);
				char tmp2[1000];
				sprintf(tmp2,"%s = %lld",tmp,value);
				attributes[numattr++]=QString(tmp2);
				}; break;
			case H5T_FLOAT: {
				double value;
				ret = H5Aread(attr, type, &value);
				// printf("The value of float attribute %s is %g\n", tmp, value);
				char tmp2[1000];
				sprintf(tmp2,"%s = %g",tmp,value);
				attributes[numattr++]=QString(tmp2);
				}; break;
			case H5T_STRING: {
				char value[1000];
				ret = H5Aread(attr, type, &value);
				// printf("The value of string attribute %s is %s\n", tmp, value);
				char tmp2[1100];
				sprintf(tmp2,"%s = %s",tmp,value);
				attributes[numattr++]=QString(tmp2);
				}; break;
			default:
				printf("UNKNOWN ATTRIBUTE\n");
				break;
		}

		ret = H5Aclose(attr);
	}

	H5Giterate(group, "/", NULL, scanfile, NULL);
	H5Gclose(group);
#endif
}

//! export data to the file
int FilterHDF5::exportFile(MainWin *mw, bool all, int datatype, int order, int start, int end) {
	kDebug()<<"FilterHDF5::exportFile()	: not implemented yet!"<<endl;
#ifdef HAVE_HDF5
//	hid_t file = H5Fcreate(filename.toAscii(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

	//TODO
/*	QWorkspace *ws = mw->getWorkspace();
	QWidgetList list = ws->windowList();
	for ( int i = 0; i < int(list.count()); i++) {
		Spreadsheet *s = (Spreadsheet *) list.at(i);
		if(s->getWidgetType() == SPREADSHEET) {
			// "only selected spreadsheet"
			if (all==false && s != (Spreadsheet *) ws->activeWindow())
				continue;

			if (end==0) end=s->numRows();
			int NX = end - start;
			int NY = s->numCols();

			hsize_t dims[2];
			dims[0]=NX;
			dims[1]=NY;
			int RANK=2;

			// Create dataspace for datasets
			hid_t dataspace = H5Screate_simple(RANK, dims, NULL);

			// Create a dataset (inside Group1)
			hid_t type;
			switch(datatype) {
			case 0:
				type = H5Tcopy(H5T_NATIVE_DOUBLE);
				break;
			case 1:
				type = H5Tcopy(H5T_NATIVE_INT);
				break;
			case 2:
				type = H5Tcopy(H5T_NATIVE_UCHAR);
				break;
			}

			switch(order) {
			case 0:
				H5Tset_order(datatype, H5T_ORDER_LE);
				break;
			case 1:
				H5Tset_order(datatype, H5T_ORDER_BE);
				break;
			}
			hid_t dataset = H5Dcreate(file,s->Title().latin1(),type,dataspace,H5P_DEFAULT);

			hid_t status;
			switch(datatype) {
			case 0: {
				double *d = new double[NX*NY];
				for(int j=0;j<NY;j++)
					for(int i=0;i<NX;i++)
						d[j+i*NY] = table->text(i+start,j).toDouble();

				status = H5Dwrite(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT,(double *) d);
				delete [] d;
				}; break;
			case 1: {
				int *d = new int[NX*NY];

				for(int j=0;j<NY;j++)
					for(int i=0;i<NX;i++)
						d[j+i*NY] = table->text(i+start,j).toInt();

				status = H5Dwrite(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT,(int *) d);
				delete [] d;
				}; break;
			case 2: {
				unsigned char *d = new unsigned char[NX*NY];

				for(int j=0;j<NY;j++)
					for(int i=0;i<NX;i++)
						d[j+i*NY] = table->text(i+start,j).toInt();

				status = H5Dwrite(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT,(unsigned char *) d);
				delete [] d;
				}; break;
			}
		}
	}

	return H5Fclose(file);
*/
	
#else
	return -1;
#endif
}

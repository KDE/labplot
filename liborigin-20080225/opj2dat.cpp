/***************************************************************************
    File                 : opj2dat.cpp
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : Origin project converter

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

#include <stdio.h>

#ifndef WIN32
#include <libgen.h>
#endif

#include <libgen.h>
#include <string.h>
#include <math.h>
#include <cstring>
#include "OPJFile.h"

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Usage : ./opj2dat <file.opj>\n");
		return -1;
	}

	printf("opj2dat %s, Copyright (C) 2008 Stefan Gerlach\n",LIBORIGIN_VERSION_STRING);
	
	if(!strcmp(argv[1],"-v"))
		return 0;

	OPJFile opj(argv[1]);
	int status = opj.Parse();
	printf("Parsing status = %d\n",status);
	printf("OPJ PROJECT \"%s\" VERSION = %.2f\n",argv[1],opj.Version());
	
	printf("NUMBER OF SPREADSHEETS = %d\n",opj.numSpreads());
	for (int s=0;s<opj.numSpreads();s++) {
		int nr_cols=opj.numCols(s);
		
		printf("SPREADSHEET %d : %s\n",s+1,opj.spreadName(s));
		printf("	COLUMNS = %d\n",nr_cols);
		for (int j=0;j<nr_cols;j++) {
			printf("	COLUMN %d : %s / TYPE = %d,ROWS = %d\n",
				j+1,opj.colName(s,j),opj.colType(s,j),opj.numRows(s,j));
			
		}
		FILE *out;
		char filename[255];
#ifndef WIN32
		sprintf(filename,"%s.%d.dat",argv[1],s+1);
#else
		sprintf(filename,"%s.%d.dat",basename(argv[1]),s+1);
#endif
		printf("saved to %s\n",filename);
		if((out=fopen(filename,"w")) == NULL ) {
			printf("Could not open %s",filename);
			return -1;
		}
		// header
		for (int j=0;j<nr_cols;j++)
			fprintf(out,"%s ",opj.colName(s,j));	
		fprintf(out,"\n");
		// data
		for (int i=0;i<opj.maxRows(s);i++) {
			for (int j=0;j<nr_cols;j++) {
				if(opj.colType(s,j) == Label) {
					fprintf(out,"%s ",(char *)opj.oData(s,j,i));
				}
				else {
					double *v=0;
					if(i<opj.numRows(s,j))
						v = (double *) opj.oData(s,j,i);
					if(fabs(*v)>2.0e-300)
						fprintf(out,"%g ",*v);
				}		
			}
			fprintf(out,"\n");
		}
	}
}

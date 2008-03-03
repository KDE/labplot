// opj2dat.cpp
#include <stdio.h>

#ifndef WIN32
#include <libgen.h>
#endif

#include <libgen.h>
#include <string.h>
#include <math.h>
#include "OPJFile.h"

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Usage : ./opj2dat <file.opj>\n");
		return -1;
	}

	printf("opj2dat %s, Copyright (C) 2007 Stefan Gerlach\n",LIBORIGIN_VERSION_STRING);
	
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
			printf("	COLUMN %d : %s / TYPE = %s,ROWS = %d\n",
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
				if(!strcmp(opj.colType(s,j),"LABEL")) {
					fprintf(out,"%s ",(char *)opj.oData(s,j,i));
				}
				else {
					double *v;
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

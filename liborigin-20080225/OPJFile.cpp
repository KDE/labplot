/***************************************************************************
    File                 : OPJFile.cpp
    --------------------------------------------------------------------
    Copyright            : (C) 2005-2008 Stefan Gerlach
			   (C) 2007-2008 by Alex Kargovsky, Ion Vasilief
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, 
			   kargovsky*yumr.phys.msu.su, ion_vasilief*yahoo.fr
    Description          : Origin project import class

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
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <cstring>
#include <algorithm> //required for std::swap
#include "OPJFile.h"

const char* colTypeNames[] = {"X", "Y", "Z", "XErr", "YErr", "Label", "None"};
#define MAX_LEVEL 20
#define ERROR_MSG "Please send the OPJ file and the opjfile.log to the author of liborigin!\n"

#define SwapBytes(x) ByteSwap((unsigned char *) &x,sizeof(x))

int strcmp_i(const char *s1, const char *s2) { //compare two strings ignoring case
#ifdef _WINDOWS
	return stricmp(s1,s2);
#else
	return strcasecmp(s1,s2);
#endif
}

void OPJFile::ByteSwap(unsigned char * b, int n) {
	register int i = 0;
	register int j = n-1;
	while (i<j) {
		std::swap(b[i], b[j]);
		i++, j--;
	}
}

OPJFile::OPJFile(const char *fn) {
	filename = string(fn);
	version=0;
	dataIndex=0;
	objectIndex=0;
}

int OPJFile::compareSpreadnames(char *sname) const {
	for(unsigned int i=0;i<SPREADSHEET.size();i++)
		if (0==strcmp_i(SPREADSHEET[i].name.c_str(),sname))
			return i;
	return -1;
}

int OPJFile::compareExcelnames(char *sname) const {
	for(unsigned int i=0;i<EXCEL.size();i++)
		if (0==strcmp_i(EXCEL[i].name.c_str(),sname))
			return i;
	return -1;
}


int OPJFile::compareColumnnames(int spread, char *sname) const {
	for(unsigned int i=0;i<SPREADSHEET[spread].column.size();i++)
		if (SPREADSHEET[spread].column[i].name == sname)
			return i;
	return -1;
}
int OPJFile::compareExcelColumnnames(int iexcel, int isheet, char *sname) const {
	for(unsigned int i=0;i<EXCEL[iexcel].sheet[isheet].column.size();i++)
		if (EXCEL[iexcel].sheet[isheet].column[i].name == sname)
			return i;
	return -1;
}

int OPJFile::compareMatrixnames(char *sname) const {
	for(unsigned int i=0;i<MATRIX.size();i++)
		if (0==strcmp_i(MATRIX[i].name.c_str(),sname))
			return i;
	return -1;
}

int OPJFile::compareFunctionnames(const char *sname) const {
	for(unsigned int i=0;i<FUNCTION.size();i++)
		if (0==strcmp_i(FUNCTION[i].name.c_str(),sname))
			return i;
	return -1;
}

vector<string> OPJFile::findDataByIndex(int index) const {
	vector<string> str;
	for(unsigned int spread=0;spread<SPREADSHEET.size();spread++)
		for(unsigned int i=0;i<SPREADSHEET[spread].column.size();i++)
			if (SPREADSHEET[spread].column[i].index == index)
			{
				str.push_back(SPREADSHEET[spread].column[i].name);
				str.push_back("T_" + SPREADSHEET[spread].name);
				return str;
			}
	for(unsigned int i=0;i<MATRIX.size();i++)
		if (MATRIX[i].index == index)
		{
			str.push_back(MATRIX[i].name);
			str.push_back("M_" + MATRIX[i].name);
			return str;
		}
	for(unsigned int i=0;i<EXCEL.size();i++)
		for(unsigned int j=0;j<EXCEL[i].sheet.size();j++)
			for(unsigned int k=0;k<EXCEL[i].sheet[j].column.size();k++)
				if (EXCEL[i].sheet[j].column[k].index == index)
				{
					str.push_back(EXCEL[i].sheet[j].column[k].name);
					str.push_back("E_" + EXCEL[i].name);
					return str;
				}
	for(unsigned int i=0;i<FUNCTION.size();i++)
		if (FUNCTION[i].index == index)
		{
			str.push_back(FUNCTION[i].name);
			str.push_back("F_" + FUNCTION[i].name);
			return str;
		}
	return str;
}

string OPJFile::findObjectByIndex(int index) {
	for(unsigned int i=0;i<SPREADSHEET.size();i++)
		if (SPREADSHEET[i].objectID == index)
		{
			return SPREADSHEET[i].name;
		}

	for(unsigned int i=0;i<MATRIX.size();i++)
		if (MATRIX[i].objectID == index)
		{
			return MATRIX[i].name;
		}

	for(unsigned int i=0;i<EXCEL.size();i++)
		if (EXCEL[i].objectID == index)
		{
			return EXCEL[i].name;
		}

	for(unsigned int i=0;i<GRAPH.size();i++)
		if (GRAPH[i].objectID == index)
		{
			return GRAPH[i].name;
		}

	return "";
}

void OPJFile::convertSpreadToExcel(int spread)
{
	//add new Excel sheet
	EXCEL.push_back(excel(SPREADSHEET[spread].name, SPREADSHEET[spread].label, SPREADSHEET[spread].maxRows, SPREADSHEET[spread].bHidden, SPREADSHEET[spread].bLoose));
	for (unsigned int i=0; i<SPREADSHEET[spread].column.size(); ++i)
	{
		string name=SPREADSHEET[spread].column[i].name;
		int pos=name.find_last_of("@");
		string col=name;
		unsigned int index=0;
		if(pos!=-1)
		{
			col=name.substr(0, pos);
			index=atoi(name.substr(pos+1).c_str())-1;
		}

		if(EXCEL.back().sheet.size()<=index)
			EXCEL.back().sheet.resize(index+1);
		SPREADSHEET[spread].column[i].name=col;
		EXCEL.back().sheet[index].column.push_back(SPREADSHEET[spread].column[i]);
	}
	SPREADSHEET.erase(SPREADSHEET.begin()+spread);
}
// set default name for columns starting from spreadsheet spread
void OPJFile::setColName(int spread) {
	for(unsigned int j=spread;j<SPREADSHEET.size();j++) {
		SPREADSHEET[j].column[0].type=X;
		for (unsigned int k=1;k<SPREADSHEET[j].column.size();k++)
			SPREADSHEET[j].column[k].type=Y;
	}
}

/* File Structure :
filepre +
	+ pre + head + data	col A
	+ pre + head + data	col B
*/

/* parse file "filename" completely and save values */
int OPJFile::Parse() {
	FILE *f;
	printf("FILENAME=%s\n",filename.c_str());
	if((f=fopen(filename.c_str(),"rb")) == NULL ) {
		printf("Could not open %s!\n",filename.c_str());
		return -1;
	}

	char vers[5];
	vers[4]=0;

	// get version
	fseek(f,0x7,SEEK_SET);
	fread(&vers,4,1,f);
	fclose(f);
	version = atoi(vers);

	if(version >= 2766 && version <= 2769) 	// 7.5
		return ParseFormatNew();
	else
		return ParseFormatOld();
}


int OPJFile::ParseFormatOld() {
	int i;
	FILE *f, *debug;
	if((f=fopen(filename.c_str(),"rb")) == NULL ) {
		printf("Could not open %s!\n",filename.c_str());
		return -1;
	}

	if((debug=fopen("opjfile.log","w")) == NULL ) {
		printf("Could not open log file!\n");
		return -1;
	}

	////////////////////////////// check version from header ///////////////////////////////
	char vers[5];
	vers[4]=0;

	// get version
	fseek(f,0x7,SEEK_SET);
	fread(&vers,4,1,f);
	version = atoi(vers);
	fprintf(debug,"	[version = %d]\n",version);

	// translate version
	if(version >= 130 && version <= 140) 		// 4.1
		version=410;
	else if(version == 210) 	// 5.0
		version=500;
	else if(version == 2625) 	// 6.0
		version=600;
	else if(version == 2627) 	// 6.0 SR1
		version=601;
	else if(version == 2630 ) 	// 6.0 SR4
		version=604;
	else if(version == 2635 ) 	// 6.1
		version=610;
	else if(version == 2656) 	// 7.0
		version=700;
	else if(version == 2672) 	// 7.0 SR3
		version=703;
	else {
		fprintf(debug,"Found unknown project version %d\n",version);
		fprintf(debug,"Please contact the author of opj2dat\n");
	}
	fprintf(debug,"Found project version %.2f\n",version/100.0);

	unsigned char c=0;	// tmp char

	fprintf(debug,"HEADER :\n");
	for(i=0;i<0x16;i++) {	// skip header + 5 Bytes ("27")
		fread(&c,1,1,f);
		fprintf(debug,"%.2X ",c);
		if(!((i+1)%16)) fprintf(debug,"\n");
	}
	fprintf(debug,"\n");

	do{
		fread(&c,1,1,f);
	} while (c != '\n');
	fprintf(debug,"	[file header @ 0x%X]\n", (unsigned int) ftell(f));

	/////////////////// find column ///////////////////////////////////////////////////////////
	if(version>410)
		for(i=0;i<5;i++)	// skip "0"
			fread(&c,1,1,f);

	int col_found;
	fread(&col_found,4,1,f);
	if(IsBigEndian()) SwapBytes(col_found);

	fread(&c,1,1,f);	// skip '\n'
	fprintf(debug,"	[column found = %d/0x%X @ 0x%X]\n",col_found,col_found,(unsigned int) ftell(f));

	int current_col=1, nr=0, nbytes=0;
	double a;
	char name[25], valuesize;
	while(col_found > 0 && col_found < 0x84) {	// should be 0x72, 0x73 or 0x83
		//////////////////////////////// COLUMN HEADER /////////////////////////////////////////////
		fprintf(debug,"COLUMN HEADER :\n");
		for(i=0;i < 0x3D;i++) {	// skip 0x3C chars to value size
			fread(&c,1,1,f);
			//if(i>21 && i<27) {
			fprintf(debug,"%.2X ",c);
			if(!((i+1)%16)) fprintf(debug,"\n");
			//}
		}
		fprintf(debug,"\n");

		fread(&valuesize,1,1,f);
		fprintf(debug,"	[valuesize = %d @ 0x%X]\n",valuesize,(unsigned int) ftell(f)-1);
		if(valuesize <= 0) {
			fprintf(debug,"	WARNING : found strange valuesize of %d\n",valuesize);
			valuesize=10;
		}

		fprintf(debug,"SKIP :\n");
		for(i=0;i<0x1A;i++) {	// skip to name
			fread(&c,1,1,f);
			fprintf(debug,"%.2X ",c);
			if(!((i+1)%16)) fprintf(debug,"\n");
		}
		fprintf(debug,"\n");

		// read name
		fprintf(debug,"	[Spreadsheet @ 0x%X]\n",(unsigned int) ftell(f));
		fflush(debug);
		fread(&name,25,1,f);
		//char* sname = new char[26];
		char sname[26];
		sprintf(sname,"%s",strtok(name,"_"));	// spreadsheet name
		char* cname = strtok(NULL,"_");	// column name
		while(char* tmpstr = strtok(NULL,"_")) {	// get multiple-"_" title correct
			strcat(sname,"_");
			strcat(sname,cname);
			strcpy(cname,tmpstr);
		}
		int spread=0;
		if(SPREADSHEET.size() == 0 || compareSpreadnames(sname) == -1) {
			fprintf(debug,"NEW SPREADSHEET\n");
			current_col=1;
			SPREADSHEET.push_back(spreadSheet(sname));
			spread=SPREADSHEET.size()-1;
			SPREADSHEET.back().maxRows=0;
		}
		else {

			spread=compareSpreadnames(sname);

			current_col=SPREADSHEET[spread].column.size();

			if(!current_col)
				current_col=1;
			current_col++;
		}
		fprintf(debug,"SPREADSHEET = %s COLUMN %d NAME = %s (@0x%X)\n",
			sname, current_col, cname, (unsigned int) ftell(f));
		fflush(debug);

		if(cname == 0) {
			fprintf(debug,"NO COLUMN NAME FOUND! Must be a matrix or function.\n");
			////////////////////////////// READ MATRIX or FUNCTION ////////////////////////////////////
			fprintf(debug,"Reading MATRIX.\n");
			fflush(debug);

			fprintf(debug,"	[position @ 0x%X]\n",(unsigned int) ftell(f));
			// TODO
			fprintf(debug,"	SIGNATURE : ");
			for(i=0;i<2;i++) {	// skip header
				fread(&c,1,1,f);
				fprintf(debug,"%.2X ",c);
			}
			fflush(debug);

			do{	// skip until '\n'
				fread(&c,1,1,f);
				// fprintf(debug,"%.2X ",c);
			} while (c != '\n');
			fprintf(debug,"\n");
			fflush(debug);

			// read size
			int size;
			fread(&size,4,1,f);
			fread(&c,1,1,f);	// skip '\n'
			// TODO : use entry size : double, float, ...
			size /= 8;
			fprintf(debug,"	SIZE = %d\n",size);
			fflush(debug);

			// catch exception
			if(size>10000)
				size=1000;

			fprintf(debug,"VALUES :\n");
			SPREADSHEET[SPREADSHEET.size()-1].maxRows=1;

			double value=0;
			for(i=0;i<size;i++) {	// read data
				string stmp;
				if(i<26)
					stmp=i+0x41;
				else if(i<26*26) {
					stmp = 0x40+i/26;
					stmp[1] = i%26+0x41;
				}
				else {
					stmp = 0x40+i/26/26;
					stmp[1] = i/26%26+0x41;
					stmp[2] = i%26+0x41;
				}
				SPREADSHEET[SPREADSHEET.size()-1].column.push_back(stmp);
				fread(&value,8,1,f);
				SPREADSHEET[SPREADSHEET.size()-1].column[i].odata.push_back(originData(value));

				fprintf(debug,"%g ",value);
			}
			fprintf(debug,"\n");
			fflush(debug);

		}
		else {	// worksheet
			SPREADSHEET[spread].column.push_back(spreadColumn(cname));

			////////////////////////////// SIZE of column /////////////////////////////////////////////
			do{	// skip until '\n'
				fread(&c,1,1,f);
			} while (c != '\n');

			fread(&nbytes,4,1,f);
			if(IsBigEndian()) SwapBytes(nbytes);
			if(fmod(nbytes,(double)valuesize)>0)
				fprintf(debug,"WARNING: data section could not be read correct\n");
			nr = nbytes / valuesize;
			fprintf(debug,"	[number of rows = %d (%d Bytes) @ 0x%X]\n",nr,nbytes,(unsigned int) ftell(f));
			fflush(debug);

			SPREADSHEET[spread].maxRows<nr?SPREADSHEET[spread].maxRows=nr:0;

			////////////////////////////////////// DATA ////////////////////////////////////////////////
			fread(&c,1,1,f);	// skip '\n'
			if(valuesize != 8 && valuesize <= 16) {	// skip 0 0
				fread(&c,1,1,f);
				fread(&c,1,1,f);
			}
			fprintf(debug,"	[data @ 0x%X]\n",(unsigned int) ftell(f));
			fflush(debug);

			for (i=0;i<nr;i++) {
				if(valuesize <= 16) {	// value
					fread(&a,valuesize,1,f);
					if(IsBigEndian()) SwapBytes(a);
					fprintf(debug,"%g ",a);
					SPREADSHEET[spread].column[(current_col-1)].odata.push_back(originData(a));
				}
				else {			// label
					char *stmp = new char[valuesize+1];
					fread(stmp,valuesize,1,f);
					fprintf(debug,"%s ",stmp);
					SPREADSHEET[spread].column[(current_col-1)].odata.push_back(originData(stmp));
					delete stmp;
				}
			}
		}	// else
		//		fprintf(debug,"	[now @ 0x%X]\n",ftell(f));
		fprintf(debug,"\n");
		fflush(debug);

		for(i=0;i<4;i++)	// skip "0"
			fread(&c,1,1,f);
		if(valuesize == 8 || valuesize > 16) {	// skip 0 0
			fread(&c,1,1,f);
			fread(&c,1,1,f);
		}
		fread(&col_found,4,1,f);
		if(IsBigEndian()) SwapBytes(col_found);
		fread(&c,1,1,f);	// skip '\n'
		fprintf(debug,"	[column found = %d/0x%X (@ 0x%X)]\n",col_found,col_found,(unsigned int) ftell(f)-5);
		fflush(debug);
	}

	////////////////////// HEADER SECTION //////////////////////////////////////
	// TODO : use new method ('\n')

	int POS = ftell(f)-11;
	fprintf(debug,"\nHEADER SECTION\n");
	fprintf(debug,"	nr_spreads = %d\n",SPREADSHEET.size());
	fprintf(debug,"	[position @ 0x%X]\n",POS);
	fflush(debug);

///////////////////// SPREADSHEET INFOS ////////////////////////////////////
	int LAYER=0;
	int COL_JUMP = 0x1ED;
	for(unsigned int i=0; i < SPREADSHEET.size(); i++) {
	fprintf(debug,"		reading	Spreadsheet %d/%zd properties\n",i+1,SPREADSHEET.size());
	fflush(debug);
	if(i > 0) {
		if (version == 700 )
			POS += 0x2530 + SPREADSHEET[i-1].column.size()*COL_JUMP;
		else if (version == 610 )
			POS += 0x25A4 + SPREADSHEET[i-1].column.size()*COL_JUMP;
		else if (version == 604 )
			POS += 0x25A0 + SPREADSHEET[i-1].column.size()*COL_JUMP;
		else if (version == 601 )
			POS += 0x2560 + SPREADSHEET[i-1].column.size()*COL_JUMP;	// ?
		else if (version == 600 )
			POS += 0x2560 + SPREADSHEET[i-1].column.size()*COL_JUMP;
		else if (version == 500 )
			POS += 0x92C + SPREADSHEET[i-1].column.size()*COL_JUMP;
		else if (version == 410 )
			POS += 0x7FB + SPREADSHEET[i-1].column.size()*COL_JUMP;
	}

	fprintf(debug,"			reading	Header\n");
	fflush(debug);
	// HEADER
	// check header
	int ORIGIN = 0x55;
	if(version == 500)
		ORIGIN = 0x58;
	fseek(f,POS + ORIGIN,SEEK_SET);	// check for 'O'RIGIN
	char c;
	fread(&c,1,1,f);
	int jump=0;
	if( c == 'O')
		fprintf(debug,"			\"ORIGIN\" found ! (@ 0x%X)\n",POS+ORIGIN);
	while( c != 'O' && jump < MAX_LEVEL) {	// no inf loop
		fprintf(debug,"		TRY %d	\"O\"RIGIN not found ! : %c (@ 0x%X)",jump+1,c,POS+ORIGIN);
		fprintf(debug,"			POS=0x%X | ORIGIN = 0x%X\n",POS,ORIGIN);
		fflush(debug);
		POS+=0x1F2;
		fseek(f,POS + ORIGIN,SEEK_SET);
		fread(&c,1,1,f);
		jump++;
	}

	int spread=i;
	if(jump == MAX_LEVEL){
		fprintf(debug,"		Spreadsheet SECTION not found ! 	(@ 0x%X)\n",POS-10*0x1F2+0x55);
		// setColName(spread);
		return -5;
	}

	fprintf(debug,"			[Spreadsheet SECTION (@ 0x%X)]\n",POS);
	fflush(debug);

	// check spreadsheet name
	fseek(f,POS + 0x12,SEEK_SET);
	fread(&name,25,1,f);

	spread=compareSpreadnames(name);
	if(spread == -1)
		spread=i;

	fprintf(debug,"			SPREADSHEET %d NAME : %s	(@ 0x%X) has %d columns\n",
		spread+1,name,POS + 0x12,SPREADSHEET[spread].column.size());
	fflush(debug);

	int ATYPE=0;
	LAYER = POS;
	if (version == 700)
		ATYPE = 0x2E4;
	else if (version == 610)
		ATYPE = 0x358;
	else if (version == 604)
		ATYPE = 0x354;
	else if (version == 601)
		ATYPE = 0x500;	// ?
	else if (version == 600)
		ATYPE = 0x314;
	else if (version == 500) {
		COL_JUMP=0x5D;
		ATYPE = 0x300;
	}
	else if (version == 410) {
		COL_JUMP = 0x58;
		ATYPE = 0x229;
	}
	fflush(debug);

	/////////////// COLUMN Types ///////////////////////////////////////////
	fprintf(debug,"			Spreadsheet has %d columns\n",SPREADSHEET[spread].column.size());
	for (unsigned int j=0;j<SPREADSHEET[spread].column.size();j++) {
		fprintf(debug,"			reading	COLUMN %d/%zd type\n",j+1,SPREADSHEET[spread].column.size());
		fflush(debug);
		fseek(f,LAYER+ATYPE+j*COL_JUMP, SEEK_SET);
		fread(&name,25,1,f);

		fseek(f,LAYER+ATYPE+j*COL_JUMP-1, SEEK_SET);
		fread(&c,1,1,f);
		ColumnType type;
		switch(c) {
			case 3:
				type = X;
				break;
			case 0:
				type = Y;
				break;
			case 5:
				type = Z;
				break;
			case 6:
				type = XErr;
				break;
			case 2:
				type = YErr;
				break;
			case 4:
				type = Label;
				break;
			default:
				type = NONE;
				break;
		}

		SPREADSHEET[spread].column[j].type=type;

		fprintf(debug,"				COLUMN \"%s\" type = %d (@ 0x%X)\n",
			SPREADSHEET[spread].column[j].name.c_str(),type,LAYER+ATYPE+j*COL_JUMP);
		fflush(debug);

		// check column name
		int max_length=11;	// only first 11 chars are saved here !
		int name_length = SPREADSHEET[spread].column[j].name.length();
                    int length = (name_length < max_length) ? name_length : max_length;

		if(SPREADSHEET[spread].column[j].name.substr(0,length) == name) {
			fprintf(debug,"				TEST : column name = \"%s\". OK!\n",
				SPREADSHEET[spread].column[j].name.c_str());
		}
		else {
			fprintf(debug,"				TEST : COLUMN %d name mismatch (\"%s\" != \"%s\")\n",
				j+1,name,SPREADSHEET[spread].column[j].name.c_str());
			//fprintf(debug,"ERROR : column name mismatch! Continue anyway.\n"ERROR_MSG);
		}
		fflush(debug);
	}
		fprintf(debug,"		Done with spreadsheet %d\n",spread);
		fflush(debug);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO : GRAPHS

	fprintf(debug,"Done parsing\n");
	fclose(debug);

	return 0;
}


int OPJFile::ParseFormatNew() {
	int i;
	FILE *f, *debug;
	if((f=fopen(filename.c_str(),"rb")) == NULL ) {
		printf("Could not open %s!\n",filename.c_str());
		return -1;
	}

	if((debug=fopen("opjfile.log","w")) == NULL ) {
		printf("Could not open log file!\n");
		return -1;
	}

////////////////////////////// check version from header ///////////////////////////////
	char vers[5];
	vers[4]=0;

	// get version
	fseek(f,0x7,SEEK_SET);
	fread(&vers,4,1,f);
	version = atoi(vers);
	fprintf(debug,"	[version = %d]\n",version);

	// translate version
	if(version >= 130 && version <= 140) 		// 4.1
		version=410;
	else if(version == 210) 	// 5.0
		version=500;
	else if(version == 2625) 	// 6.0
		version=600;
	else if(version == 2627) 	// 6.0 SR1
		version=601;
	else if(version == 2630 ) 	// 6.0 SR4
		version=604;
	else if(version == 2635 ) 	// 6.1
		version=610;
	else if(version == 2656) 	// 7.0
		version=700;
	else if(version == 2672) 	// 7.0 SR3
		version=703;
	else if(version >= 2766 && version <= 2769) 	// 7.5
		version=750;
	else {
		fprintf(debug,"Found unknown project version %d\n",version);
		fprintf(debug,"Please contact the author of opj2dat\n");
	}
	fprintf(debug,"Found project version %.2f\n",version/100.0);

	unsigned char c=0;	// tmp char

	fprintf(debug,"HEADER :\n");
	for(i=0;i<0x16;i++) {	// skip header + 5 Bytes ("27")
		fread(&c,1,1,f);
		fprintf(debug,"%.2X ",c);
		if(!((i+1)%16)) fprintf(debug,"\n");
	}
	fprintf(debug,"\n");

	do{
		fread(&c,1,1,f);
	} while (c != '\n');
	fprintf(debug,"	[file header @ 0x%X]\n", (unsigned int) ftell(f));

/////////////////// find column ///////////////////////////////////////////////////////////
	if(version>410)
		for(i=0;i<5;i++)	// skip "0"
			fread(&c,1,1,f);

	int col_found;
	fread(&col_found,4,1,f);
	if(IsBigEndian()) SwapBytes(col_found);

	fread(&c,1,1,f);	// skip '\n'
	fprintf(debug,"	[column found = %d/0x%X @ 0x%X]\n",col_found,col_found,(unsigned int) ftell(f));
	int colpos=ftell(f);

	int current_col=1, nr=0, nbytes=0;
	double a;
	char name[25], valuesize;
	while(col_found > 0 && col_found < 0x84) {	// should be 0x72, 0x73 or 0x83
//////////////////////////////// COLUMN HEADER /////////////////////////////////////////////
		short data_type;
		char data_type_u;
		int oldpos=ftell(f);
		fseek(f,oldpos+0x16,SEEK_SET);
		fread(&data_type,2,1,f);
		if(IsBigEndian()) SwapBytes(data_type);
		fseek(f,oldpos+0x3F,SEEK_SET);
		fread(&data_type_u,1,1,f);
		fseek(f,oldpos,SEEK_SET);

		fprintf(debug,"COLUMN HEADER :\n");
		for(i=0;i < 0x3D;i++) {	// skip 0x3C chars to value size
			fread(&c,1,1,f);
			//if(i>21 && i<27) {
				fprintf(debug,"%.2X ",c);
				if(!((i+1)%16)) fprintf(debug,"\n");
			//}
		}
		fprintf(debug,"\n");

		fread(&valuesize,1,1,f);
		fprintf(debug,"	[valuesize = %d @ 0x%X]\n",valuesize,(unsigned int) ftell(f)-1);
		if(valuesize <= 0) {
			fprintf(debug,"	WARNING : found strange valuesize of %d\n",valuesize);
			valuesize=10;
		}

		fprintf(debug,"SKIP :\n");
		for(i=0;i<0x1A;i++) {	// skip to name
			fread(&c,1,1,f);
			fprintf(debug,"%.2X ",c);
			if(!((i+1)%16)) fprintf(debug,"\n");
		}
		fprintf(debug,"\n");

		// read name
		fprintf(debug,"	[Spreadsheet @ 0x%X]\n",(unsigned int) ftell(f));
		fflush(debug);
		fread(&name,25,1,f);
		//char* sname = new char[26];
		char sname[26];
		sprintf(sname,"%s",strtok(name,"_"));	// spreadsheet name
		char* cname = strtok(NULL,"_");	// column name
		while(char* tmpstr = strtok(NULL,"_")) {	// get multiple-"_" title correct
			strcat(sname,"_");
			strcat(sname,cname);
			strcpy(cname,tmpstr);
		}
		int spread=0;
		if(cname == 0) {
			fprintf(debug,"NO COLUMN NAME FOUND! Must be a matrix or function.\n");
////////////////////////////// READ MATRIX or FUNCTION ////////////////////////////////////

			fprintf(debug,"	[position @ 0x%X]\n",(unsigned int) ftell(f));
			// TODO
			short signature;
			fread(&signature,2,1,f);
			if(IsBigEndian()) SwapBytes(signature);
			fprintf(debug,"	SIGNATURE : ");
			fprintf(debug,"%.2X ",signature);
			fflush(debug);

			do{	// skip until '\n'
				fread(&c,1,1,f);
				// fprintf(debug,"%.2X ",c);
			} while (c != '\n');
			fprintf(debug,"\n");
			fflush(debug);

			// read size
			int size;
			fread(&size,4,1,f);
			if(IsBigEndian()) SwapBytes(size);
			fread(&c,1,1,f);	// skip '\n'
			// TODO : use entry size : double, float, ...
			size /= valuesize;
			fprintf(debug,"	SIZE = %d\n",size);
			fflush(debug);

			// catch exception
			/*if(size>10000)
				size=1000;*/
			switch(signature)
			{
			case 0x50CA:
			case 0x70CA:
			case 0x50F2:
			case 0x50E2:
				fprintf(debug,"NEW MATRIX\n");
				MATRIX.push_back(matrix(sname, dataIndex));
				dataIndex++;

				fprintf(debug,"VALUES :\n");

				switch(data_type)
				{
				case 0x6001://double
					for(i=0;i<size;i++) {
						double value;
						fread(&value,valuesize,1,f);
						if(IsBigEndian()) SwapBytes(value);
						MATRIX.back().data.push_back((double)value);
						fprintf(debug,"%g ",MATRIX.back().data.back());
					}
					break;
				case 0x6003://float
					for(i=0;i<size;i++) {
						float value;
						fread(&value,valuesize,1,f);
						if(IsBigEndian()) SwapBytes(value);
						MATRIX.back().data.push_back((double)value);
						fprintf(debug,"%g ",MATRIX.back().data.back());
					}
					break;
				case 0x6801://int
					if(data_type_u==8)//unsigned
						for(i=0;i<size;i++) {
							unsigned int value;
							fread(&value,valuesize,1,f);
							if(IsBigEndian()) SwapBytes(value);
							MATRIX.back().data.push_back((double)value);
							fprintf(debug,"%g ",MATRIX.back().data.back());
						}
					else
						for(i=0;i<size;i++) {
							int value;
							fread(&value,valuesize,1,f);
							if(IsBigEndian()) SwapBytes(value);
							MATRIX.back().data.push_back((double)value);
							fprintf(debug,"%g ",MATRIX.back().data.back());
						}
					break;
				case 0x6803://short
					if(data_type_u==8)//unsigned
						for(i=0;i<size;i++) {
							unsigned short value;
							fread(&value,valuesize,1,f);
							if(IsBigEndian()) SwapBytes(value);
							MATRIX.back().data.push_back((double)value);
							fprintf(debug,"%g ",MATRIX.back().data.back());
						}
					else
						for(i=0;i<size;i++) {
							short value;
							fread(&value,valuesize,1,f);
							if(IsBigEndian()) SwapBytes(value);
							MATRIX.back().data.push_back((double)value);
							fprintf(debug,"%g ",MATRIX.back().data.back());
						}
					break;
				case 0x6821://char
					if(data_type_u==8)//unsigned
						for(i=0;i<size;i++) {
							unsigned char value;
							fread(&value,valuesize,1,f);
							if(IsBigEndian()) SwapBytes(value);
							MATRIX.back().data.push_back((double)value);
							fprintf(debug,"%g ",MATRIX.back().data.back());
						}
					else
						for(i=0;i<size;i++) {
							char value;
							fread(&value,valuesize,1,f);
							if(IsBigEndian()) SwapBytes(value);
							MATRIX.back().data.push_back((double)value);
							fprintf(debug,"%g ",MATRIX.back().data.back());
						}
					break;
				default:
					fprintf(debug,"UNKNOWN MATRIX DATATYPE: %.2X SKIP DATA\n", data_type);
					fseek(f, valuesize*size, SEEK_CUR);
					MATRIX.pop_back();
				}

				break;
			case 0x10C8:
				fprintf(debug,"NEW FUNCTION\n");
				FUNCTION.push_back(function(sname, dataIndex));
				dataIndex++;

				char *cmd;
				cmd=new char[valuesize+1];
				cmd[valuesize]='\0';
				fread(cmd,valuesize,1,f);
				FUNCTION.back().formula=cmd;
				int oldpos;
				oldpos=ftell(f);
				short t;
				fseek(f,colpos+0xA,SEEK_SET);
				fread(&t,2,1,f);
				if(IsBigEndian()) SwapBytes(t);
				if(t==0x1194)
					FUNCTION.back().type=1;
				int N;
				fseek(f,colpos+0x21,SEEK_SET);
				fread(&N,4,1,f);
				if(IsBigEndian()) SwapBytes(N);
				FUNCTION.back().points=N;
				double d;
				fread(&d,8,1,f);
				if(IsBigEndian()) SwapBytes(d);
				FUNCTION.back().begin=d;
				fread(&d,8,1,f);
				if(IsBigEndian()) SwapBytes(d);
				FUNCTION.back().end=FUNCTION.back().begin+d*(FUNCTION.back().points-1);
				fprintf(debug,"FUNCTION %s : %s \n", FUNCTION.back().name.c_str(), FUNCTION.back().formula.c_str());
				fprintf(debug," interval %g : %g, number of points %d \n", FUNCTION.back().begin, FUNCTION.back().end, FUNCTION.back().points);
				fseek(f,oldpos,SEEK_SET);

				delete [] cmd;
				break;
			default:
				fprintf(debug,"UNKNOWN SIGNATURE: %.2X SKIP DATA\n", signature);
				fseek(f, valuesize*size, SEEK_CUR);
				if(valuesize != 8 && valuesize <= 16)
					fseek(f, 2, SEEK_CUR);
			}

			fprintf(debug,"\n");
			fflush(debug);
		}
		else {	// worksheet
			if(SPREADSHEET.size() == 0 || compareSpreadnames(sname) == -1) {
				fprintf(debug,"NEW SPREADSHEET\n");
				current_col=1;
				SPREADSHEET.push_back(spreadSheet(sname));
				spread=SPREADSHEET.size()-1;
				SPREADSHEET.back().maxRows=0;
			}
			else {

				spread=compareSpreadnames(sname);

				current_col=SPREADSHEET[spread].column.size();

				if(!current_col)
					current_col=1;
				current_col++;
			}
			fprintf(debug,"SPREADSHEET = %s COLUMN NAME = %s (%d) (@0x%X)\n",
				sname, cname,current_col,(unsigned int) ftell(f));
			fflush(debug);
			SPREADSHEET[spread].column.push_back(spreadColumn(cname, dataIndex));
			int sheetpos=SPREADSHEET[spread].column.back().name.find_last_of("@");
			if(!SPREADSHEET[spread].bMultisheet && sheetpos!=-1)
				if(atoi(string(cname).substr(sheetpos+1).c_str())>1)
				{
					SPREADSHEET[spread].bMultisheet=true;
					fprintf(debug,"SPREADSHEET \"%s\" IS MULTISHEET \n", sname);
				}
			dataIndex++;

////////////////////////////// SIZE of column /////////////////////////////////////////////
			do{	// skip until '\n'
				fread(&c,1,1,f);
			} while (c != '\n');

			fread(&nbytes,4,1,f);
			if(IsBigEndian()) SwapBytes(nbytes);
			if(fmod(nbytes,(double)valuesize)>0)
				fprintf(debug,"WARNING: data section could not be read correct\n");
			nr = nbytes / valuesize;
			fprintf(debug,"	[number of rows = %d (%d Bytes) @ 0x%X]\n",nr,nbytes,(unsigned int) ftell(f));
			fflush(debug);

			SPREADSHEET[spread].maxRows<nr?SPREADSHEET[spread].maxRows=nr:0;

////////////////////////////////////// DATA ////////////////////////////////////////////////
			fread(&c,1,1,f);	// skip '\n'
			/*if(valuesize != 8 && valuesize <= 16 && nbytes>0) {	// skip 0 0
				fread(&c,1,1,f);
				fread(&c,1,1,f);
			}*/
			fprintf(debug,"	[data @ 0x%X]\n",(unsigned int) ftell(f));
			fflush(debug);

			for (i=0;i<nr;i++) {
				if(valuesize <= 8) {	// Numeric, Time, Date, Month, Day
					fread(&a,valuesize,1,f);
					if(IsBigEndian()) SwapBytes(a);
					fprintf(debug,"%g ",a);
					SPREADSHEET[spread].column[(current_col-1)].odata.push_back(originData(a));
				}
				else if((data_type&0x100)==0x100) // Text&Numeric
				{
					fread(&c,1,1,f);
					fseek(f,1,SEEK_CUR);
					if(c==0) //value
					{
						//fread(&a,valuesize-2,1,f);
						fread(&a,8,1,f);
						if(IsBigEndian()) SwapBytes(a);
						fprintf(debug,"%g ",a);
						SPREADSHEET[spread].column[(current_col-1)].odata.push_back(originData(a));
						fseek(f,valuesize-10,SEEK_CUR);
					}
					else //text
					{
						char *stmp = new char[valuesize-1];
						fread(stmp,valuesize-2,1,f);
						if(strchr(stmp,0x0E)) // try find non-printable symbol - garbage test
							stmp[0]='\0';
						SPREADSHEET[spread].column[(current_col-1)].odata.push_back(originData(stmp));
						fprintf(debug,"%s ",stmp);
						delete stmp;
					}
				}
				else //Text
				{
					char *stmp = new char[valuesize+1];
					fread(stmp,valuesize,1,f);
					if(strchr(stmp,0x0E)) // try find non-printable symbol - garbage test
						stmp[0]='\0';
					SPREADSHEET[spread].column[(current_col-1)].odata.push_back(originData(stmp));
					fprintf(debug,"%s ",stmp);
					delete stmp;
				}
			}

		}	// else

		fprintf(debug,"\n");
		fflush(debug);

		if(nbytes>0||cname==0)
			fseek(f,1,SEEK_CUR);

		int tailsize;
		fread(&tailsize,4,1,f);
		if(IsBigEndian()) SwapBytes(tailsize);
		fseek(f,1+tailsize+(tailsize>0?1:0),SEEK_CUR); //skip tail
		//fseek(f,5+((nbytes>0||cname==0)?1:0),SEEK_CUR);
		fread(&col_found,4,1,f);
		if(IsBigEndian()) SwapBytes(col_found);
		fseek(f,1,SEEK_CUR);	// skip '\n'
		fprintf(debug,"	[column found = %d/0x%X (@ 0x%X)]\n",col_found,col_found,(unsigned int) ftell(f)-5);
		colpos=ftell(f);
		fflush(debug);
	}

////////////////////////////////////////////////////////////////////////////
	for(unsigned int i=0; i<SPREADSHEET.size(); ++i)
		if(SPREADSHEET[i].bMultisheet)
		{
			fprintf(debug,"		CONVERT SPREADSHEET \"%s\" to EXCEL\n", SPREADSHEET[i].name.c_str());
			fflush(debug);
			convertSpreadToExcel(i);
			i--;
		}
////////////////////////////////////////////////////////////////////////////
////////////////////// HEADER SECTION //////////////////////////////////////

	int POS = ftell(f)-11;
	fprintf(debug,"\nHEADER SECTION\n");
	fprintf(debug,"	nr_spreads = %d\n",SPREADSHEET.size());
	fprintf(debug,"	[position @ 0x%X]\n",POS);
	fflush(debug);

//////////////////////// OBJECT INFOS //////////////////////////////////////
	POS+=0xB;
	fseek(f,POS,SEEK_SET);
	while(1) {

		fprintf(debug,"			reading	Header\n");
		fflush(debug);
		// HEADER
		// check header
		POS=ftell(f);
		int headersize;
		fread(&headersize,4,1,f);
		if(IsBigEndian()) SwapBytes(headersize);
		if(headersize==0)
			break;
		char object_type[10];
		char object_name[25];
		fseek(f,POS + 0x7,SEEK_SET);
		fread(&object_name,25,1,f);
		fseek(f,POS + 0x4A,SEEK_SET);
		fread(&object_type,10,1,f);

		fseek(f,POS,SEEK_SET);

		if(compareSpreadnames(object_name)!=-1)
			readSpreadInfo(f, debug);
		else if(compareMatrixnames(object_name)!=-1)
			readMatrixInfo(f, debug);
		else if(compareExcelnames(object_name)!=-1)
			readExcelInfo(f, debug);
		else
			readGraphInfo(f, debug);
	}


	fseek(f,1,SEEK_CUR);
	fprintf(debug,"Some Origin params @ 0x%X:\n", (unsigned int)ftell(f));
	fread(&c,1,1,f);
	while(c!=0)
	{
		fprintf(debug,"		");
		while(c!='\n'){
			fprintf(debug,"%c",c);
			fread(&c,1,1,f);
		}
		double parvalue;
		fread(&parvalue,8,1,f);
		if(IsBigEndian()) SwapBytes(parvalue);
		fprintf(debug,": %g\n", parvalue);
		fseek(f,1,SEEK_CUR);
		fread(&c,1,1,f);
	}
	fseek(f,1+5,SEEK_CUR);
	while(1)
	{
		//fseek(f,5+0x40+1,SEEK_CUR);
		int size;
		fread(&size,4,1,f);
		if(IsBigEndian()) SwapBytes(size);
		if(size!=0x40)
			break;

		double creation_date, modification_date;

		fseek(f,1+0x20,SEEK_CUR);
		fread(&creation_date,8,1,f);
		if(IsBigEndian()) SwapBytes(creation_date);

		fread(&modification_date,8,1,f);
		if(IsBigEndian()) SwapBytes(modification_date);

		fseek(f,0x10-4,SEEK_CUR);
		unsigned char labellen;
		fread(&labellen,1,1,f);

		fseek(f,4,SEEK_CUR);
		fread(&size,4,1,f);
		if(IsBigEndian()) SwapBytes(size);
		fseek(f,1,SEEK_CUR);
		char *stmp = new char[size+1];
		fread(stmp,size,1,f);
		if(0==strcmp(stmp,"ResultsLog"))
		{
			delete stmp;
			fseek(f,1,SEEK_CUR);
			fread(&size,4,1,f);
			if(IsBigEndian()) SwapBytes(size);
			fseek(f,1,SEEK_CUR);
			stmp = new char[size+1];
			fread(stmp,size,1,f);
			resultsLog=stmp;
			fprintf(debug,"Results Log: %s\n", resultsLog.c_str());
			delete stmp;
			break;
		}
		else
		{
			NOTE.push_back(note(stmp));
			NOTE.back().objectID=objectIndex;
			NOTE.back().creation_date=creation_date;
			NOTE.back().modification_date=modification_date;
			objectIndex++;
			delete stmp;
			fseek(f,1,SEEK_CUR);
			fread(&size,4,1,f);
			if(IsBigEndian()) SwapBytes(size);
			fseek(f,1,SEEK_CUR);
			if(labellen>1)
			{
				stmp = new char[labellen];
				stmp[labellen-1]='\0';
				fread(stmp,labellen-1,1,f);
				NOTE.back().label=stmp;
				delete stmp;
				fseek(f,1,SEEK_CUR);
			}
			stmp = new char[size-labellen+1];
			fread(stmp,size-labellen,1,f);
			NOTE.back().text=stmp;
			fprintf(debug,"NOTE %d NAME: %s\n", NOTE.size(), NOTE.back().name.c_str());
			fprintf(debug,"NOTE %d LABEL: %s\n", NOTE.size(), NOTE.back().label.c_str());
			fprintf(debug,"NOTE %d TEXT:\n%s\n", NOTE.size(), NOTE.back().text.c_str());
			delete stmp;
			fseek(f,1,SEEK_CUR);
		}
	}

	fseek(f,1+4*5+0x10+1,SEEK_CUR);
	try
	{
		readProjectTree(f, debug);
	}
	catch(...)
	{}
	fprintf(debug,"Done parsing\n");
	fclose(debug);

	return 0;
}

void OPJFile::readSpreadInfo(FILE *f, FILE *debug)
{
	int POS=ftell(f);

	int headersize;
	fread(&headersize,4,1,f);
	if(IsBigEndian()) SwapBytes(headersize);

	POS+=5;

	fprintf(debug,"			[Spreadsheet SECTION (@ 0x%X)]\n",POS);
	fflush(debug);

	// check spreadsheet name
	char name[25];
	fseek(f,POS + 0x2,SEEK_SET);
	fread(&name,25,1,f);

	int spread=compareSpreadnames(name);
	SPREADSHEET[spread].name=name;
	readWindowProperties(SPREADSHEET[spread], f, debug, POS, headersize);
	SPREADSHEET[spread].bLoose=false;
	char c = 0;

	int LAYER = POS;
	{
		// LAYER section
		LAYER += headersize + 0x1 + 0x5/* length of block = 0x12D + '\n'*/ + 0x12D + 0x1;
		//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
		//possible sections: column formulas, __WIPR, __WIOTN, __LayerInfoStorage etc
		//section name(column name in formula case) starts with 0x46 position
		while(1)
		{
			int sec_size;
		//section_header_size=0x6F(4 bytes) + '\n'
			LAYER+=0x5;

		//section_header
			fseek(f,LAYER+0x46,SEEK_SET);
			char sec_name[42];
			sec_name[41]='\0';
			fread(&sec_name,41,1,f);

			fprintf(debug,"				DEBUG SECTION NAME: %s (@ 0x%X)\n", sec_name, LAYER+0x46);
			fflush(debug);

		//section_body_1_size
			LAYER+=0x6F+0x1;
			fseek(f,LAYER,SEEK_SET);
			fread(&sec_size,4,1,f);
			if(IsBigEndian()) SwapBytes(sec_size);

		//section_body_1
			LAYER+=0x5;
			fseek(f,LAYER,SEEK_SET);
			//check if it is a formula
			int col_index=compareColumnnames(spread,sec_name);
			if(col_index!=-1)
			{
				char *stmp=new char[sec_size+1];
				stmp[sec_size]='\0';
				fread(stmp,sec_size,1,f);
				SPREADSHEET[spread].column[col_index].command=stmp;
				delete stmp;
			}

		//section_body_2_size
			LAYER+=sec_size+0x1;
			fseek(f,LAYER,SEEK_SET);
			fread(&sec_size,4,1,f);
			if(IsBigEndian()) SwapBytes(sec_size);

		//section_body_2
			LAYER+=0x5;

		//close section 00 00 00 00 0A
			LAYER+=sec_size+(sec_size>0?0x1:0)+0x5;

			if(0==strcmp(sec_name,"__LayerInfoStorage"))
				break;

		}
		LAYER+=0x5;

	}

	fflush(debug);

	/////////////// COLUMN Types ///////////////////////////////////////////
	fprintf(debug,"			Spreadsheet has %d columns\n",SPREADSHEET[spread].column.size());

	while(1)
	{
		LAYER+=0x5;
		fseek(f,LAYER+0x12, SEEK_SET);
		fread(&name,12,1,f);

		fseek(f,LAYER+0x11, SEEK_SET);
		fread(&c,1,1,f);
		short width=0;
		fseek(f,LAYER+0x4A, SEEK_SET);
		fread(&width,2,1,f);
		if(IsBigEndian()) SwapBytes(width);
		int col_index=compareColumnnames(spread,name);
		if(col_index!=-1)
		{
			ColumnType type;
			switch(c) {
				case 3:
					type = X;
					break;
				case 0:
					type = Y;
					break;
				case 5:
					type = Z;
					break;
				case 6:
					type = XErr;
					break;
				case 2:
					type = YErr;
					break;
				case 4:
					type = Label;
					break;
				default:
					type = NONE;
					break;
			}
			SPREADSHEET[spread].column[col_index].type=type;
			width/=0xA;
			if(width==0)
				width=8;
			SPREADSHEET[spread].column[col_index].width=width;
			fseek(f,LAYER+0x1E, SEEK_SET);
			unsigned char c1,c2;
			fread(&c1,1,1,f);
			fread(&c2,1,1,f);
			switch(c1)
			{
			case 0x00: // Numeric	   - Dec1000
			case 0x09: // Text&Numeric - Dec1000
			case 0x10: // Numeric	   - Scientific
			case 0x19: // Text&Numeric - Scientific
			case 0x20: // Numeric	   - Engeneering
			case 0x29: // Text&Numeric - Engeneering
			case 0x30: // Numeric	   - Dec1,000
			case 0x39: // Text&Numeric - Dec1,000
				SPREADSHEET[spread].column[col_index].value_type=(c1%0x10==0x9)?6:0;
				SPREADSHEET[spread].column[col_index].value_type_specification=c1/0x10;
				if(c2>=0x80)
				{
					SPREADSHEET[spread].column[col_index].significant_digits=c2-0x80;
					SPREADSHEET[spread].column[col_index].numeric_display_type=2;
				}
				else if(c2>0)
				{
					SPREADSHEET[spread].column[col_index].decimal_places=c2-0x03;
					SPREADSHEET[spread].column[col_index].numeric_display_type=1;
				}
				break;
			case 0x02: // Time
				SPREADSHEET[spread].column[col_index].value_type=3;
				SPREADSHEET[spread].column[col_index].value_type_specification=c2-0x80;
				break;
			case 0x03: // Date
				SPREADSHEET[spread].column[col_index].value_type=2;
				SPREADSHEET[spread].column[col_index].value_type_specification=c2-0x80;
				break;
			case 0x31: // Text
				SPREADSHEET[spread].column[col_index].value_type=1;
				break;
			case 0x4: // Month
			case 0x34:
				SPREADSHEET[spread].column[col_index].value_type=4;
				SPREADSHEET[spread].column[col_index].value_type_specification=c2;
				break;
			case 0x5: // Day
			case 0x35:
				SPREADSHEET[spread].column[col_index].value_type=5;
				SPREADSHEET[spread].column[col_index].value_type_specification=c2;
				break;
			default: // Text
				SPREADSHEET[spread].column[col_index].value_type=1;
				break;
			}
			fprintf(debug,"				COLUMN \"%s\" type = %s(%d) (@ 0x%X)\n",
				SPREADSHEET[spread].column[col_index].name.c_str(),colTypeNames[type],c,LAYER+0x11);
			fflush(debug);
		}
		LAYER+=0x1E7+0x1;
		fseek(f,LAYER,SEEK_SET);
		int comm_size=0;
		fread(&comm_size,4,1,f);
		if(IsBigEndian()) SwapBytes(comm_size);
		LAYER+=0x5;
		if(comm_size>0)
		{
			char* comment=new char[comm_size+1];
			comment[comm_size]='\0';
			fseek(f,LAYER,SEEK_SET);
			fread(comment,comm_size,1,f);
			if(col_index!=-1)
				SPREADSHEET[spread].column[col_index].comment=comment;
			LAYER+=comm_size+0x1;
			delete comment;
		}
		fseek(f,LAYER,SEEK_SET);
		int ntmp;
		fread(&ntmp,4,1,f);
		if(IsBigEndian()) SwapBytes(ntmp);
		if(ntmp!=0x1E7)
			break;
	}
	fprintf(debug,"		Done with spreadsheet %d\n",spread);
	fflush(debug);

	POS = LAYER+0x5*0x6+0x1ED*0x12;
	fseek(f,POS,SEEK_SET);
}

void OPJFile::readExcelInfo(FILE *f, FILE *debug)
{
	int POS=ftell(f);

	int headersize;
	fread(&headersize,4,1,f);
	if(IsBigEndian()) SwapBytes(headersize);

	POS+=5;

	fprintf(debug,"			[EXCEL SECTION (@ 0x%X)]\n",POS);
	fflush(debug);

	// check spreadsheet name
	char name[25];
	fseek(f,POS + 0x2,SEEK_SET);
	fread(&name,25,1,f);

	int iexcel=compareExcelnames(name);
	EXCEL[iexcel].name=name;
	readWindowProperties(EXCEL[iexcel], f, debug, POS, headersize);
	EXCEL[iexcel].bLoose=false;
	char c = 0;

	int LAYER = POS;
	LAYER += headersize + 0x1;
	int sec_size;
	int isheet=0;
	while(1)// multisheet loop
	{
		// LAYER section
		LAYER += 0x5/* length of block = 0x12D + '\n'*/ + 0x12D + 0x1;
		//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
		//possible sections: column formulas, __WIPR, __WIOTN, __LayerInfoStorage etc
		//section name(column name in formula case) starts with 0x46 position
		while(1)
		{
		//section_header_size=0x6F(4 bytes) + '\n'
			LAYER+=0x5;

		//section_header
			fseek(f,LAYER+0x46,SEEK_SET);
			char sec_name[42];
			sec_name[41]='\0';
			fread(&sec_name,41,1,f);

			fprintf(debug,"				DEBUG SECTION NAME: %s (@ 0x%X)\n", sec_name, LAYER+0x46);
			fflush(debug);

		//section_body_1_size
			LAYER+=0x6F+0x1;
			fseek(f,LAYER,SEEK_SET);
			fread(&sec_size,4,1,f);
			if(IsBigEndian()) SwapBytes(sec_size);

		//section_body_1
			LAYER+=0x5;
			fseek(f,LAYER,SEEK_SET);
			//check if it is a formula
			int col_index=compareExcelColumnnames(iexcel, isheet, sec_name);
			if(col_index!=-1)
			{
				char *stmp=new char[sec_size+1];
				stmp[sec_size]='\0';
				fread(stmp,sec_size,1,f);
				EXCEL[iexcel].sheet[isheet].column[col_index].command=stmp;
				delete stmp;
			}

		//section_body_2_size
			LAYER+=sec_size+0x1;
			fseek(f,LAYER,SEEK_SET);
			fread(&sec_size,4,1,f);
			if(IsBigEndian()) SwapBytes(sec_size);

		//section_body_2
			LAYER+=0x5;

		//close section 00 00 00 00 0A
			LAYER+=sec_size+(sec_size>0?0x1:0)+0x5;

			if(0==strcmp(sec_name,"__LayerInfoStorage"))
				break;

		}
		LAYER+=0x5;

		fflush(debug);

		/////////////// COLUMN Types ///////////////////////////////////////////
		fprintf(debug,"			Excel sheet %d has %d columns\n",isheet,EXCEL[iexcel].sheet[isheet].column.size());

		while(1)
		{
			LAYER+=0x5;
			fseek(f,LAYER+0x12, SEEK_SET);
			fread(&name,12,1,f);

			fseek(f,LAYER+0x11, SEEK_SET);
			fread(&c,1,1,f);
			short width=0;
			fseek(f,LAYER+0x4A, SEEK_SET);
			fread(&width,2,1,f);
			if(IsBigEndian()) SwapBytes(width);
			//char col_name[30];
			//sprintf(col_name, "%s@%d", name, isheet);
			int col_index=compareExcelColumnnames(iexcel, isheet, name);
			if(col_index!=-1)
			{
				ColumnType type;
				switch(c) {
					case 3:
						type = X;
						break;
					case 0:
						type = Y;
						break;
					case 5:
						type = Z;
						break;
					case 6:
						type = XErr;
						break;
					case 2:
						type = YErr;
						break;
					case 4:
						type = Label;
						break;
					default:
						type = NONE;
						break;
				}
				EXCEL[iexcel].sheet[isheet].column[col_index].type=type;
				width/=0xA;
				if(width==0)
					width=8;
				EXCEL[iexcel].sheet[isheet].column[col_index].width=width;
				fseek(f,LAYER+0x1E, SEEK_SET);
				unsigned char c1,c2;
				fread(&c1,1,1,f);
				fread(&c2,1,1,f);
				switch(c1)
				{
				case 0x00: // Numeric	   - Dec1000
				case 0x09: // Text&Numeric - Dec1000
				case 0x10: // Numeric	   - Scientific
				case 0x19: // Text&Numeric - Scientific
				case 0x20: // Numeric	   - Engeneering
				case 0x29: // Text&Numeric - Engeneering
				case 0x30: // Numeric	   - Dec1,000
				case 0x39: // Text&Numeric - Dec1,000
					EXCEL[iexcel].sheet[isheet].column[col_index].value_type=(c1%0x10==0x9)?6:0;
					EXCEL[iexcel].sheet[isheet].column[col_index].value_type_specification=c1/0x10;
					if(c2>=0x80)
					{
						EXCEL[iexcel].sheet[isheet].column[col_index].significant_digits=c2-0x80;
						EXCEL[iexcel].sheet[isheet].column[col_index].numeric_display_type=2;
					}
					else if(c2>0)
					{
						EXCEL[iexcel].sheet[isheet].column[col_index].decimal_places=c2-0x03;
						EXCEL[iexcel].sheet[isheet].column[col_index].numeric_display_type=1;
					}
					break;
				case 0x02: // Time
					EXCEL[iexcel].sheet[isheet].column[col_index].value_type=3;
					EXCEL[iexcel].sheet[isheet].column[col_index].value_type_specification=c2-0x80;
					break;
				case 0x03: // Date
					EXCEL[iexcel].sheet[isheet].column[col_index].value_type=2;
					EXCEL[iexcel].sheet[isheet].column[col_index].value_type_specification=c2-0x80;
					break;
				case 0x31: // Text
					EXCEL[iexcel].sheet[isheet].column[col_index].value_type=1;
					break;
				case 0x4: // Month
				case 0x34:
					EXCEL[iexcel].sheet[isheet].column[col_index].value_type=4;
					EXCEL[iexcel].sheet[isheet].column[col_index].value_type_specification=c2;
					break;
				case 0x5: // Day
				case 0x35:
					EXCEL[iexcel].sheet[isheet].column[col_index].value_type=5;
					EXCEL[iexcel].sheet[isheet].column[col_index].value_type_specification=c2;
					break;
				default: // Text
					EXCEL[iexcel].sheet[isheet].column[col_index].value_type=1;
					break;
				}
				fprintf(debug,"				COLUMN \"%s\" type = %d(%d) (@ 0x%X)\n",
					EXCEL[iexcel].sheet[isheet].column[col_index].name.c_str(),type,c,LAYER+0x11);
				fflush(debug);
			}
			LAYER+=0x1E7+0x1;
			fseek(f,LAYER,SEEK_SET);
			int comm_size=0;
			fread(&comm_size,4,1,f);
			if(IsBigEndian()) SwapBytes(comm_size);
			LAYER+=0x5;
			if(comm_size>0)
			{
				char* comment=new char[comm_size+1];
				comment[comm_size]='\0';
				fseek(f,LAYER,SEEK_SET);
				fread(comment,comm_size,1,f);
				if(col_index!=-1)
					EXCEL[iexcel].sheet[isheet].column[col_index].comment=comment;
				LAYER+=comm_size+0x1;
				delete comment;
			}
			fseek(f,LAYER,SEEK_SET);
			int ntmp;
			fread(&ntmp,4,1,f);
			if(IsBigEndian()) SwapBytes(ntmp);
			if(ntmp!=0x1E7)
				break;
		}
		fprintf(debug,"		Done with excel %d\n", iexcel);
		fflush(debug);

		//POS = LAYER+0x5*0x6+0x1ED*0x12;
		LAYER+=0x5*0x5+0x1ED*0x12;
		fseek(f,LAYER,SEEK_SET);
		fread(&sec_size,4,1,f);
		if(IsBigEndian()) SwapBytes(sec_size);
		if(sec_size==0)
			break;
		isheet++;
	}
	POS = LAYER+0x5;

	fseek(f,POS,SEEK_SET);
}

void OPJFile::readMatrixInfo(FILE *f, FILE *debug)
{
	int POS=ftell(f);

	int headersize;
	fread(&headersize,4,1,f);
	if(IsBigEndian()) SwapBytes(headersize);
	POS+=5;

	fprintf(debug,"			[Matrix SECTION (@ 0x%X)]\n",POS);
	fflush(debug);

	// check spreadsheet name
	char name[25];
	fseek(f,POS + 0x2,SEEK_SET);
	fread(&name,25,1,f);

	int idx=compareMatrixnames(name);
	MATRIX[idx].name=name;
	readWindowProperties(MATRIX[idx], f, debug, POS, headersize);

	unsigned char h = 0;
	fseek(f,POS+0x87,SEEK_SET);
	fread(&h,1,1,f);
	switch(h)
	{
	case 1:
		MATRIX[idx].view = matrix::ImageView;
		break;
	case 2:
		MATRIX[idx].header = matrix::XY;
		break;
	}

	int LAYER = POS;
	LAYER += headersize + 0x1;
	int sec_size;
	// LAYER section
	LAYER +=0x5;
	fseek(f,LAYER+0x2B,SEEK_SET);
	short w=0;
	fread(&w,2,1,f);
	if(IsBigEndian()) SwapBytes(w);
	MATRIX[idx].nr_cols=w;
	fseek(f,LAYER+0x52,SEEK_SET);
	fread(&w,2,1,f);
	if(IsBigEndian()) SwapBytes(w);
	MATRIX[idx].nr_rows=w;

	LAYER +=0x12D + 0x1;
	//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
	//possible sections: column formulas, __WIPR, __WIOTN, __LayerInfoStorage
	//section name(column name in formula case) starts with 0x46 position
	while(1)
	{
	//section_header_size=0x6F(4 bytes) + '\n'
		LAYER+=0x5;

	//section_header
		fseek(f,LAYER+0x46,SEEK_SET);
		char sec_name[42];
		sec_name[41]='\0';
		fread(&sec_name,41,1,f);

	//section_body_1_size
		LAYER+=0x6F+0x1;
		fseek(f,LAYER,SEEK_SET);
		fread(&sec_size,4,1,f);
		if(IsBigEndian()) SwapBytes(sec_size);

	//section_body_1
		LAYER+=0x5;
		//check if it is a formula
		if(0==strcmp(sec_name,"MV"))
		{
			fseek(f,LAYER,SEEK_SET);
			char *stmp=new char[sec_size+1];
			stmp[sec_size]='\0';
			fread(stmp,sec_size,1,f);
			MATRIX[idx].command=stmp;
			delete stmp;
		}

	//section_body_2_size
		LAYER+=sec_size+0x1;
		fseek(f,LAYER,SEEK_SET);
		fread(&sec_size,4,1,f);
		if(IsBigEndian()) SwapBytes(sec_size);

	//section_body_2
		LAYER+=0x5;

	//close section 00 00 00 00 0A
		LAYER+=sec_size+(sec_size>0?0x1:0)+0x5;

		if(0==strcmp(sec_name,"__LayerInfoStorage"))
			break;

	}
	LAYER+=0x5;

	while(1)
	{
		LAYER+=0x5;

		short width=0;
		fseek(f,LAYER+0x2B, SEEK_SET);
		fread(&width,2,1,f);
		if(IsBigEndian()) SwapBytes(width);
		width=(width-55)/0xA;
		if(width==0)
			width=8;
		MATRIX[idx].width=width;
		fseek(f,LAYER+0x1E, SEEK_SET);
		unsigned char c1,c2;
		fread(&c1,1,1,f);
		fread(&c2,1,1,f);

		MATRIX[idx].value_type_specification=c1/0x10;
		if(c2>=0x80)
		{
			MATRIX[idx].significant_digits=c2-0x80;
			MATRIX[idx].numeric_display_type=2;
		}
		else if(c2>0)
		{
			MATRIX[idx].decimal_places=c2-0x03;
			MATRIX[idx].numeric_display_type=1;
		}

		LAYER+=0x1E7+0x1;
		fseek(f,LAYER,SEEK_SET);
		int comm_size=0;
		fread(&comm_size,4,1,f);
		if(IsBigEndian()) SwapBytes(comm_size);
		LAYER+=0x5;
		if(comm_size>0)
		{
			LAYER+=comm_size+0x1;
		}
		fseek(f,LAYER,SEEK_SET);
		int ntmp;
		fread(&ntmp,4,1,f);
		if(IsBigEndian()) SwapBytes(ntmp);
		if(ntmp!=0x1E7)
			break;
	}

	LAYER+=0x5*0x5+0x1ED*0x12;
	POS = LAYER+0x5;

	fseek(f,POS,SEEK_SET);
}


void OPJFile::readGraphInfo(FILE *f, FILE *debug)
{
	int POS=ftell(f);

	int headersize;
	fread(&headersize,4,1,f);
	if(IsBigEndian()) SwapBytes(headersize);
	POS+=5;

	fprintf(debug,"			[Graph SECTION (@ 0x%X)]\n",POS);
	fflush(debug);

	char name[25];
	fseek(f,POS + 0x2,SEEK_SET);
	fread(&name,25,1,f);

	GRAPH.push_back(graph(name));
	readWindowProperties(GRAPH.back(), f, debug, POS, headersize);

	unsigned short graph_width;
	fseek(f,POS + 0x23,SEEK_SET);
	fread(&graph_width,2,1,f);
	if(IsBigEndian()) SwapBytes(graph_width);
	GRAPH.back().width = graph_width;

	unsigned short graph_height;
	fread(&graph_height,2,1,f);
	if(IsBigEndian()) SwapBytes(graph_height);
	GRAPH.back().height = graph_height;

	int LAYER = POS;
	LAYER += headersize + 0x1;
	int sec_size;
	while(1)// multilayer loop
	{
		GRAPH.back().layer.push_back(graphLayer());
		// LAYER section
		LAYER +=0x5;
		double range=0.0;
		unsigned char m=0;
		fseek(f, LAYER+0xF, SEEK_SET);
		fread(&range,8,1,f);
		if(IsBigEndian()) SwapBytes(range);
		GRAPH.back().layer.back().xAxis.min=range;
		fread(&range,8,1,f);
		if(IsBigEndian()) SwapBytes(range);
		GRAPH.back().layer.back().xAxis.max=range;
		fread(&range,8,1,f);
		if(IsBigEndian()) SwapBytes(range);
		GRAPH.back().layer.back().xAxis.step=range;
		fseek(f, LAYER+0x2B, SEEK_SET);
		fread(&m,1,1,f);
		GRAPH.back().layer.back().xAxis.majorTicks=m;
		fseek(f, LAYER+0x37, SEEK_SET);
		fread(&m,1,1,f);
		GRAPH.back().layer.back().xAxis.minorTicks=m;
		fread(&m,1,1,f);
		GRAPH.back().layer.back().xAxis.scale=m;

		fseek(f, LAYER+0x3A, SEEK_SET);
		fread(&range,8,1,f);
		if(IsBigEndian()) SwapBytes(range);
		GRAPH.back().layer.back().yAxis.min=range;
		fread(&range,8,1,f);
		if(IsBigEndian()) SwapBytes(range);
		GRAPH.back().layer.back().yAxis.max=range;
		fread(&range,8,1,f);
		if(IsBigEndian()) SwapBytes(range);
		GRAPH.back().layer.back().yAxis.step=range;
		fseek(f, LAYER+0x56, SEEK_SET);
		fread(&m,1,1,f);
		GRAPH.back().layer.back().yAxis.majorTicks=m;
		fseek(f, LAYER+0x62, SEEK_SET);
		fread(&m,1,1,f);
		GRAPH.back().layer.back().yAxis.minorTicks=m;
		fread(&m,1,1,f);
		GRAPH.back().layer.back().yAxis.scale=m;

		rect r;
		fseek(f, LAYER+0x71, SEEK_SET);
		fread(&r,sizeof(rect),1,f);
		if(IsBigEndian()) SwapBytes(r);
		GRAPH.back().layer.back().clientRect=r;

		LAYER += 0x12D + 0x1;
		//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
		//possible sections: axes, legend, __BC02, _202, _231, _232, __LayerInfoStorage etc
		//section name starts with 0x46 position
		while(1)
		{
		//section_header_size=0x6F(4 bytes) + '\n'
			LAYER+=0x5;

		//section_header
			fseek(f,LAYER+0x46,SEEK_SET);
			char sec_name[42];
			sec_name[41]='\0';
			fread(&sec_name,41,1,f);

			fseek(f, LAYER+0x3, SEEK_SET);
			fread(&r,sizeof(rect),1,f);
			if(IsBigEndian()) SwapBytes(r);

			unsigned char attach=0;
			fseek(f,LAYER+0x28,SEEK_SET);
			fread(&attach,1,1,f);

			unsigned char border=0;
			fseek(f, LAYER+0x29, SEEK_SET);
			fread(&border,1,1,f);

			unsigned char color=0;
			fseek(f,LAYER+0x33,SEEK_SET);
			fread(&color,1,1,f);

		//section_body_1_size
			LAYER+=0x6F+0x1;
			fseek(f,LAYER,SEEK_SET);
			fread(&sec_size,4,1,f);
			if(IsBigEndian()) SwapBytes(sec_size);

		//section_body_1
			LAYER+=0x5;
			int size=sec_size;

			unsigned char type=0;
			fseek(f,LAYER,SEEK_SET);
			fread(&type,1,1,f);

			//text properties
			short rotation=0;
			fseek(f,LAYER+2,SEEK_SET);
			fread(&rotation,2,1,f);
			if(IsBigEndian()) SwapBytes(rotation);

			unsigned char fontsize=0;
			fread(&fontsize,1,1,f);

			unsigned char tab=0;
			fseek(f,LAYER+0xA,SEEK_SET);
			fread(&tab,1,1,f);

			//line properties
			unsigned char line_style = 0;
			double width = 0.0;
			lineVertex begin, end;
			unsigned int w = 0;

			fseek(f,LAYER+0x12,SEEK_SET);
			fread(&line_style,1,1,f);

			fseek(f,LAYER+0x13,SEEK_SET);
			fread(&w,2,1,f);
			if(IsBigEndian()) SwapBytes(w);
			width = (double)w/500.0;

			fseek(f,LAYER+0x20,SEEK_SET);
			fread(&begin.x,8,1,f);
			if(IsBigEndian()) SwapBytes(begin.x);

			fread(&end.x,8,1,f);
			if(IsBigEndian()) SwapBytes(end.x);

			fseek(f,LAYER+0x40,SEEK_SET);
			fread(&begin.y,8,1,f);
			if(IsBigEndian()) SwapBytes(begin.y);

			fread(&end.y,8,1,f);
			if(IsBigEndian()) SwapBytes(end.y);

			fseek(f,LAYER+0x60,SEEK_SET);
			fread(&begin.shape_type,1,1,f);

			fseek(f,LAYER+0x64,SEEK_SET);
			fread(&w,4,1,f);
			if(IsBigEndian()) SwapBytes(w);
			begin.shape_width = (double)w/500.0;

			fread(&w,4,1,f);
			if(IsBigEndian()) SwapBytes(w);
			begin.shape_length = (double)w/500.0;

			fseek(f,LAYER+0x6C,SEEK_SET);
			fread(&end.shape_type,1,1,f);

			fseek(f,LAYER+0x70,SEEK_SET);
			fread(&w,4,1,f);
			if(IsBigEndian()) SwapBytes(w);
			end.shape_width = (double)w/500.0;

			fread(&w,4,1,f);
			if(IsBigEndian()) SwapBytes(w);
			end.shape_length = (double)w/500.0;

			// bitmap properties
			short bitmap_width = 0;
			fseek(f,LAYER+0x1,SEEK_SET);
			fread(&bitmap_width,2,1,f);
			if(IsBigEndian()) SwapBytes(bitmap_width);

			short bitmap_height = 0;
			fread(&bitmap_height,2,1,f);
			if(IsBigEndian()) SwapBytes(bitmap_height);

			double bitmap_left = 0.0;
			fseek(f,LAYER+0x13,SEEK_SET);
			fread(&bitmap_left,8,1,f);
			if(IsBigEndian()) SwapBytes(bitmap_left);

			double bitmap_top = 0.0;
			fseek(f,LAYER+0x1B,SEEK_SET);
			fread(&bitmap_top,8,1,f);
			if(IsBigEndian()) SwapBytes(bitmap_top);

		//section_body_2_size
			LAYER+=sec_size+0x1;
			fseek(f,LAYER,SEEK_SET);
			fread(&sec_size,4,1,f);
			if(IsBigEndian()) SwapBytes(sec_size);

		//section_body_2
			LAYER+=0x5;
			//check if it is a axis or legend
			fseek(f,1,SEEK_CUR);
			char stmp[255];
			if(0==strcmp(sec_name,"XB"))
			{
				stmp[sec_size]='\0';
				fread(&stmp,sec_size,1,f);
				GRAPH.back().layer.back().xAxis.pos = Bottom;
				GRAPH.back().layer.back().xAxis.label = text(stmp, r, color, fontsize, rotation/10, tab, (border >= 0x80 ? border-0x80 : None), attach);
			}
			else if(0==strcmp(sec_name,"XT"))
			{
				stmp[sec_size]='\0';
				fread(&stmp,sec_size,1,f);
				GRAPH.back().layer.back().xAxis.pos=Top;
				GRAPH.back().layer.back().xAxis.label = text(stmp, r, color, fontsize, rotation/10, tab, (border >= 0x80 ? border-0x80 : None), attach);
			}
			else if(0==strcmp(sec_name,"YL"))
			{
				stmp[sec_size]='\0';
				fread(&stmp,sec_size,1,f);
				GRAPH.back().layer.back().yAxis.pos = Left;
				GRAPH.back().layer.back().yAxis.label = text(stmp, r, color, fontsize, rotation/10, tab, (border >= 0x80 ? border-0x80 : None), attach);
			}
			else if(0==strcmp(sec_name,"YR"))
			{
				stmp[sec_size]='\0';
				fread(&stmp,sec_size,1,f);
				GRAPH.back().layer.back().yAxis.pos = Right;
				GRAPH.back().layer.back().yAxis.label = text(stmp, r, color, fontsize, rotation/10, tab, (border >= 0x80 ? border-0x80 : None), attach);
			}
			else if(0==strcmp(sec_name,"Legend"))
			{
				stmp[sec_size]='\0';
				fread(&stmp,sec_size,1,f);
				GRAPH.back().layer.back().legend = text(stmp, r, color, fontsize, rotation/10, tab, (border >= 0x80 ? border-0x80 : None), attach);
			}
			else if(0==strcmp(sec_name,"__BCO2")) // histogram
			{
				double d;
				fseek(f,LAYER+0x10,SEEK_SET);
				fread(&d,8,1,f);
				if(IsBigEndian()) SwapBytes(d);
				GRAPH.back().layer.back().histogram_bin=d;
				fseek(f,LAYER+0x20,SEEK_SET);
				fread(&d,8,1,f);
				if(IsBigEndian()) SwapBytes(d);
				GRAPH.back().layer.back().histogram_end=d;
				fseek(f,LAYER+0x28,SEEK_SET);
				fread(&d,8,1,f);
				if(IsBigEndian()) SwapBytes(d);
				GRAPH.back().layer.back().histogram_begin=d;
			}
			else if(size==0x3E) // text
			{
				stmp[sec_size]='\0';
				fread(&stmp,sec_size,1,f);
				GRAPH.back().layer.back().texts.push_back(text(stmp));
				GRAPH.back().layer.back().texts.back().color=color;
				GRAPH.back().layer.back().texts.back().clientRect=r;
				GRAPH.back().layer.back().texts.back().tab=tab;
				GRAPH.back().layer.back().texts.back().fontsize=fontsize;
				GRAPH.back().layer.back().texts.back().rotation=rotation/10;
				GRAPH.back().layer.back().texts.back().attach=attach;
				if(border>=0x80)
					GRAPH.back().layer.back().texts.back().border_type=border-0x80;
				else
					GRAPH.back().layer.back().texts.back().border_type=None;
			}
			else if(size==0x78 && type==2) // line
			{
				GRAPH.back().layer.back().lines.push_back(line());
				GRAPH.back().layer.back().lines.back().color=color;
				GRAPH.back().layer.back().lines.back().clientRect=r;
				GRAPH.back().layer.back().lines.back().attach=attach;
				GRAPH.back().layer.back().lines.back().width=width;
				GRAPH.back().layer.back().lines.back().line_style=line_style;
				GRAPH.back().layer.back().lines.back().begin=begin;
				GRAPH.back().layer.back().lines.back().end=end;
			}
			else if(size==0x28 && type==4) // bitmap
			{
				unsigned long filesize=sec_size+14;
				GRAPH.back().layer.back().bitmaps.push_back(bitmap());
				GRAPH.back().layer.back().bitmaps.back().left=bitmap_left;
				GRAPH.back().layer.back().bitmaps.back().top=bitmap_top;
				GRAPH.back().layer.back().bitmaps.back().width=
					(GRAPH.back().layer.back().xAxis.max - GRAPH.back().layer.back().xAxis.min)*bitmap_width/10000;
				GRAPH.back().layer.back().bitmaps.back().height=
					(GRAPH.back().layer.back().yAxis.max - GRAPH.back().layer.back().yAxis.min)*bitmap_height/10000;
				GRAPH.back().layer.back().bitmaps.back().attach=attach;
				GRAPH.back().layer.back().bitmaps.back().size=filesize;
				GRAPH.back().layer.back().bitmaps.back().data=new unsigned char[filesize];
				unsigned char *data=GRAPH.back().layer.back().bitmaps.back().data;
				//add Bitmap header
				memcpy(data, "BM", 2);
				data+=2;
				memcpy(data, &filesize, 4);
				data+=4;
				unsigned int d=0;
				memcpy(data, &d, 4);
				data+=4;
				d=0x36;
				memcpy(data, &d, 4);
				data+=4;
				fread(data,sec_size,1,f);
			}

		//close section 00 00 00 00 0A
			LAYER+=sec_size+(sec_size>0?0x1:0);

		//section_body_3_size
			fseek(f,LAYER,SEEK_SET);
			fread(&sec_size,4,1,f);
			if(IsBigEndian()) SwapBytes(sec_size);

		//section_body_3
			LAYER+=0x5;

		//close section 00 00 00 00 0A
			LAYER+=sec_size+(sec_size>0?0x1:0);

			if(0==strcmp(sec_name,"__LayerInfoStorage"))
				break;

		}
		LAYER+=0x5;
		unsigned char h;
		short w;

		fseek(f,LAYER,SEEK_SET);
		fread(&sec_size,4,1,f);
		if(IsBigEndian()) SwapBytes(sec_size);
		if(sec_size==0x1E7)//check layer is not empty
		{
			while(1)
			{
				LAYER+=0x5;

				graphCurve curve;

				vector<string> col;
				fseek(f,LAYER+0x4,SEEK_SET);
				fread(&w,2,1,f);
				if(IsBigEndian()) SwapBytes(w);
				col=findDataByIndex(w-1);
				short nColY = w;
				if(col.size()>0)
				{
					fprintf(debug,"			GRAPH %d layer %d curve %d Y : %s.%s\n",GRAPH.size(),GRAPH.back().layer.size(),GRAPH.back().layer.back().curve.size(),col[1].c_str(),col[0].c_str());
					fflush(debug);
					curve.yColName=col[0];
					curve.dataName=col[1];
				}

				fseek(f,LAYER+0x23,SEEK_SET);
				fread(&w,2,1,f);
				if(IsBigEndian()) SwapBytes(w);
				col=findDataByIndex(w-1);
				if(col.size()>0)
				{
					fprintf(debug,"			GRAPH %d layer %d curve %d X : %s.%s\n",GRAPH.size(),GRAPH.back().layer.size(),GRAPH.back().layer.back().curve.size(),col[1].c_str(),col[0].c_str());
					fflush(debug);
					curve.xColName=col[0];
					if(curve.dataName!=col[1])
						fprintf(debug,"			GRAPH %d X and Y from different tables\n",GRAPH.size());
				}

				fseek(f,LAYER+0x4C,SEEK_SET);
				fread(&h,1,1,f);
				curve.type=h;

				fseek(f,LAYER+0x11,SEEK_SET);
				fread(&h,1,1,f);
				curve.line_connect=h;

				fseek(f,LAYER+0x12,SEEK_SET);
				fread(&h,1,1,f);
				curve.line_style=h;

				fseek(f,LAYER+0x15,SEEK_SET);
				fread(&w,2,1,f);
				if(IsBigEndian()) SwapBytes(w);
				curve.line_width=(double)w/500.0;

				fseek(f,LAYER+0x19,SEEK_SET);
				fread(&w,2,1,f);
				if(IsBigEndian()) SwapBytes(w);
				curve.symbol_size=(double)w/500.0;

				fseek(f,LAYER+0x1C,SEEK_SET);
				fread(&h,1,1,f);
				curve.fillarea=(h==2?true:false);

				fseek(f,LAYER+0x1E,SEEK_SET);
				fread(&h,1,1,f);
				curve.fillarea_type=h;

				//vector
				if(curve.type == FlowVector || curve.type == Vector)
				{
					fseek(f,LAYER+0x56,SEEK_SET);
					fread(&curve.vector.multiplier,4,1,f);
					if(IsBigEndian()) SwapBytes(curve.vector.multiplier);

					fseek(f,LAYER+0x5E,SEEK_SET);
					fread(&h,1,1,f);
					col=findDataByIndex(nColY - 1 + h - 0x64);
					if(col.size()>0)
					{
						curve.vector.endXColName = col[0];
					}

					fseek(f,LAYER+0x62,SEEK_SET);
					fread(&h,1,1,f);
					col=findDataByIndex(nColY - 1 + h - 0x64);
					if(col.size()>0)
					{
						curve.vector.endYColName = col[0];
					}

					fseek(f,LAYER+0x18,SEEK_SET);
					fread(&h,1,1,f);
					if(h >= 0x64)
					{
						col=findDataByIndex(nColY - 1 + h - 0x64);
						if(col.size()>0)
							curve.vector.angleColName = col[0];
					}
					else if(h <= 0x08)
					{
						curve.vector.const_angle = 45*h;
					}

					fseek(f,LAYER+0x19,SEEK_SET);
					fread(&h,1,1,f);
					if(h >= 0x64)
					{
						col=findDataByIndex(nColY - 1 + h - 0x64);
						if(col.size()>0)
							curve.vector.magnitudeColName = col[0];
					}
					else
					{
						curve.vector.const_magnitude = (int) curve.symbol_size;
					}

					fseek(f,LAYER+0x66,SEEK_SET);
					fread(&curve.vector.arrow_lenght,2,1,f);
					if(IsBigEndian()) SwapBytes(curve.vector.arrow_lenght);

					fread(&curve.vector.arrow_angle,1,1,f);

					fread(&h,1,1,f);
					curve.vector.arrow_closed = !(h&0x1);

					fread(&w,2,1,f);
					if(IsBigEndian()) SwapBytes(w);
					curve.vector.width=(double)w/500.0;

					fseek(f,LAYER+0x142,SEEK_SET);
					fread(&h,1,1,f);
					switch(h)
					{
					case 2:
						curve.vector.position = Midpoint;
						break;
					case 4:
						curve.vector.position = Head;
					    break;
					default:
						curve.vector.position = Tail;
					    break;
					}
					
				}

				//pie
				if(curve.type == Pie)
				{
					fseek(f,LAYER+0x92,SEEK_SET);
					fread(&h,1,1,f);
					curve.pie.format_percentages = (h&0x01);
					curve.pie.format_values = (h&0x02);
					curve.pie.position_associate = (h&0x08);
					curve.pie.clockwise_rotation = (h&0x20);
					curve.pie.format_categories = (h&0x80);

					fread(&h,1,1,f);
					curve.pie.format_automatic = h;

					fread(&curve.pie.distance,2,1,f);
					if(IsBigEndian()) SwapBytes(curve.pie.distance);

					fread(&curve.pie.view_angle,1,1,f);

					fseek(f,LAYER+0x98,SEEK_SET);
					fread(&curve.pie.thickness,1,1,f);

					fseek(f,LAYER+0x9A,SEEK_SET);
					fread(&curve.pie.rotation,2,1,f);
					if(IsBigEndian()) SwapBytes(curve.pie.rotation);

					fseek(f,LAYER+0x9E,SEEK_SET);
					fread(&curve.pie.displacement,2,1,f);
					if(IsBigEndian()) SwapBytes(curve.pie.displacement);

					fseek(f,LAYER+0xA0,SEEK_SET);
					fread(&curve.pie.radius,2,1,f);
					if(IsBigEndian()) SwapBytes(curve.pie.radius);

					fseek(f,LAYER+0xA2,SEEK_SET);
					fread(&curve.pie.horizontal_offset,2,1,f);
					if(IsBigEndian()) SwapBytes(curve.pie.horizontal_offset);

					fseek(f,LAYER+0xA6,SEEK_SET);
					fread(&curve.pie.displaced_sections,4,1,f);
					if(IsBigEndian()) SwapBytes(curve.pie.displaced_sections);
				}
				
				fseek(f,LAYER+0xC2,SEEK_SET);
				fread(&h,1,1,f);
				curve.fillarea_color=h;

				fseek(f,LAYER+0xC3,SEEK_SET);
				fread(&h,1,1,f);
				curve.fillarea_first_color=h;

				fseek(f,LAYER+0xCE,SEEK_SET);
				fread(&h,1,1,f);
				curve.fillarea_pattern=h;

				fseek(f,LAYER+0xCA,SEEK_SET);
				fread(&h,1,1,f);
				curve.fillarea_pattern_color=h;

				fseek(f,LAYER+0xC6,SEEK_SET);
				fread(&w,2,1,f);
				if(IsBigEndian()) SwapBytes(w);
				curve.fillarea_pattern_width=(double)w/500.0;

				fseek(f,LAYER+0xCF,SEEK_SET);
				fread(&h,1,1,f);
				curve.fillarea_pattern_border_style=h;

				fseek(f,LAYER+0xD2,SEEK_SET);
				fread(&h,1,1,f);
				curve.fillarea_pattern_border_color=h;

				fseek(f,LAYER+0xD0,SEEK_SET);
				fread(&w,2,1,f);
				if(IsBigEndian()) SwapBytes(w);
				curve.fillarea_pattern_border_width=(double)w/500.0;

				fseek(f,LAYER+0x16A,SEEK_SET);
				fread(&h,1,1,f);
				curve.line_color=h;

				fseek(f,LAYER+0x17,SEEK_SET);
				fread(&w,2,1,f);
				if(IsBigEndian()) SwapBytes(w);
				curve.symbol_type=w;

				fseek(f,LAYER+0x12E,SEEK_SET);
				fread(&h,1,1,f);
				curve.symbol_fill_color=h;

				fseek(f,LAYER+0x132,SEEK_SET);
				fread(&h,1,1,f);
				curve.symbol_color=h;
				curve.vector.color=h;

				fseek(f,LAYER+0x136,SEEK_SET);
				fread(&h,1,1,f);
				curve.symbol_thickness=(h==255?1:h);

				fseek(f,LAYER+0x137,SEEK_SET);
				fread(&h,1,1,f);
				curve.point_offset=h;

				GRAPH.back().layer.back().curve.push_back(curve);

				LAYER+=0x1E7+0x1;
				fseek(f,LAYER,SEEK_SET);
				int comm_size=0;
				fread(&comm_size,4,1,f);
				if(IsBigEndian()) SwapBytes(comm_size);
				LAYER+=0x5;
				if(comm_size>0)
				{
					LAYER+=comm_size+0x1;
				}
				fseek(f,LAYER,SEEK_SET);
				int ntmp;
				fread(&ntmp,4,1,f);
				if(IsBigEndian()) SwapBytes(ntmp);
				if(ntmp!=0x1E7)
					break;
			}

		}
		//LAYER+=0x5*0x5+0x1ED*0x12;
		//LAYER+=2*0x5;

		LAYER+=0x5;
		//read axis breaks
		while(1)
		{
			fseek(f,LAYER,SEEK_SET);
			fread(&sec_size,4,1,f);
			if(IsBigEndian()) SwapBytes(sec_size);
			if(sec_size == 0x2D)
			{
				LAYER+=0x5;
				fseek(f,LAYER+2,SEEK_SET);
				fread(&h,1,1,f);
				if(h==2)
				{
					GRAPH.back().layer.back().xAxisBreak.minor_ticks_before = GRAPH.back().layer.back().xAxis.minorTicks;
					GRAPH.back().layer.back().xAxisBreak.scale_increment_before = GRAPH.back().layer.back().xAxis.step;
					readGraphAxisBreakInfo(GRAPH.back().layer.back().xAxisBreak, f, LAYER);
				}
				else if(h==4)
				{
					GRAPH.back().layer.back().yAxisBreak.minor_ticks_before = GRAPH.back().layer.back().yAxis.minorTicks;
					GRAPH.back().layer.back().yAxisBreak.scale_increment_before = GRAPH.back().layer.back().yAxis.step;
					readGraphAxisBreakInfo(GRAPH.back().layer.back().yAxisBreak, f, LAYER);
				}
				LAYER+=0x2D + 0x1;
			}
			else
				break;
		}
		LAYER+=0x5;
		

		LAYER+=0x5;
		readGraphGridInfo(GRAPH.back().layer.back().xAxis.minorGrid, f, LAYER);
		LAYER+=0x1E7+1;

		LAYER+=0x5;
		readGraphGridInfo(GRAPH.back().layer.back().xAxis.majorGrid, f, LAYER);
		LAYER+=0x1E7+1;

		LAYER+=0x5;
		readGraphAxisTickLabelsInfo(GRAPH.back().layer.back().xAxis.tickAxis[0], f, LAYER);
		LAYER+=0x1E7+1;

		LAYER+=0x5;
		readGraphAxisFormatInfo(GRAPH.back().layer.back().xAxis.formatAxis[0], f, LAYER);
		LAYER+=0x1E7+1;

		LAYER+=0x5;
		readGraphAxisTickLabelsInfo(GRAPH.back().layer.back().xAxis.tickAxis[1], f, LAYER);
		LAYER+=0x1E7+1;

		LAYER+=0x5;
		readGraphAxisFormatInfo(GRAPH.back().layer.back().xAxis.formatAxis[1], f, LAYER);
		LAYER+=0x1E7+1;

		LAYER+=0x5;


		LAYER+=0x5;
		readGraphGridInfo(GRAPH.back().layer.back().yAxis.minorGrid, f, LAYER);
		LAYER+=0x1E7+1;

		LAYER+=0x5;
		readGraphGridInfo(GRAPH.back().layer.back().yAxis.majorGrid, f, LAYER);
		LAYER+=0x1E7+1;

		LAYER+=0x5;
		readGraphAxisTickLabelsInfo(GRAPH.back().layer.back().yAxis.tickAxis[0], f, LAYER);
		LAYER+=0x1E7+1;

		LAYER+=0x5;
		readGraphAxisFormatInfo(GRAPH.back().layer.back().yAxis.formatAxis[0], f, LAYER);
		LAYER+=0x1E7+1;

		LAYER+=0x5;
		readGraphAxisTickLabelsInfo(GRAPH.back().layer.back().yAxis.tickAxis[1], f, LAYER);
		LAYER+=0x1E7+1;

		LAYER+=0x5;
		readGraphAxisFormatInfo(GRAPH.back().layer.back().yAxis.formatAxis[1], f, LAYER);
		LAYER+=0x1E7+1;

		LAYER+=0x2*0x5+0x1ED*0x6;

		fseek(f,LAYER,SEEK_SET);
		fread(&sec_size,4,1,f);
		if(IsBigEndian()) SwapBytes(sec_size);
		if(sec_size==0)
			break;
	}
	POS = LAYER+0x5;

	fseek(f,POS,SEEK_SET);
}

void OPJFile::skipObjectInfo(FILE *f, FILE *)
{
	int POS=ftell(f);

	int headersize;
	fread(&headersize,4,1,f);
	if(IsBigEndian()) SwapBytes(headersize);
	POS+=5;

	int LAYER = POS;
	LAYER += headersize + 0x1;
	int sec_size;
	while(1)// multilayer loop
	{
		// LAYER section
		LAYER +=0x5/* length of block = 0x12D + '\n'*/ + 0x12D + 0x1;
		//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
		//possible sections: column formulas, __WIPR, __WIOTN, __LayerInfoStorage
		//section name(column name in formula case) starts with 0x46 position
		while(1)
		{
		//section_header_size=0x6F(4 bytes) + '\n'
			LAYER+=0x5;

		//section_header
			fseek(f,LAYER+0x46,SEEK_SET);
			char sec_name[42];
			sec_name[41]='\0';
			fread(&sec_name,41,1,f);

		//section_body_1_size
			LAYER+=0x6F+0x1;
			fseek(f,LAYER,SEEK_SET);
			fread(&sec_size,4,1,f);
			if(IsBigEndian()) SwapBytes(sec_size);

		//section_body_1
			LAYER+=0x5;

		//section_body_2_size
			LAYER+=sec_size+0x1;
			fseek(f,LAYER,SEEK_SET);
			fread(&sec_size,4,1,f);
			if(IsBigEndian()) SwapBytes(sec_size);

		//section_body_2
			LAYER+=0x5;

		//close section 00 00 00 00 0A
			LAYER+=sec_size+(sec_size>0?0x1:0);

		//section_body_3_size
			fseek(f,LAYER,SEEK_SET);
			fread(&sec_size,4,1,f);
			if(IsBigEndian()) SwapBytes(sec_size);

		//section_body_3
			LAYER+=0x5;

		//close section 00 00 00 00 0A
			LAYER+=sec_size+(sec_size>0?0x1:0);

			if(0==strcmp(sec_name,"__LayerInfoStorage"))
				break;

		}
		LAYER+=0x5;

		while(1)
		{
			LAYER+=0x5;

			LAYER+=0x1E7+0x1;
			fseek(f,LAYER,SEEK_SET);
			int comm_size=0;
			fread(&comm_size,4,1,f);
			if(IsBigEndian()) SwapBytes(comm_size);
			LAYER+=0x5;
			if(comm_size>0)
			{
				LAYER+=comm_size+0x1;
			}
			fseek(f,LAYER,SEEK_SET);
			int ntmp;
			fread(&ntmp,4,1,f);
			if(IsBigEndian()) SwapBytes(ntmp);
			if(ntmp!=0x1E7)
				break;
		}

		LAYER+=0x5*0x5+0x1ED*0x12;
		fseek(f,LAYER,SEEK_SET);
		fread(&sec_size,4,1,f);
		if(IsBigEndian()) SwapBytes(sec_size);
		if(sec_size==0)
			break;
	}
	POS = LAYER+0x5;

	fseek(f,POS,SEEK_SET);
}

void OPJFile::readGraphGridInfo(graphGrid &grid, FILE *f, int pos)
{
	unsigned char h;
	short w;
	fseek(f,pos+0x26,SEEK_SET);
	fread(&h,1,1,f);
	grid.hidden=(h==0);

	fseek(f,pos+0xF,SEEK_SET);
	fread(&h,1,1,f);
	grid.color=h;


	fseek(f,pos+0x12,SEEK_SET);
	fread(&h,1,1,f);
	grid.style=h;

	fseek(f,pos+0x15,SEEK_SET);
	fread(&w,2,1,f);
	if(IsBigEndian()) SwapBytes(w);
	grid.width=(double)w/500.0;
}

void OPJFile::readGraphAxisBreakInfo(graphAxisBreak &axis_break, FILE *f, int pos)
{
	axis_break.show=true;

	fseek(f,pos+0x0B,SEEK_SET);
	fread(&axis_break.from,8,1,f);
	if(IsBigEndian()) SwapBytes(axis_break.from);
	
	fread(&axis_break.to,8,1,f);
	if(IsBigEndian()) SwapBytes(axis_break.to);

	fread(&axis_break.scale_increment_after,8,1,f);
	if(IsBigEndian()) SwapBytes(axis_break.scale_increment_after);

	double position=0.0;
	fread(&position,8,1,f);
	if(IsBigEndian()) SwapBytes(position);
	axis_break.position=(int)position;

	unsigned char h;
	fread(&h,1,1,f);
	axis_break.log10=(h==1);

	fread(&axis_break.minor_ticks_after,1,1,f);
}

void OPJFile::readGraphAxisFormatInfo(graphAxisFormat &format, FILE *f, int pos)
{
	unsigned char h;
	short w;
	double p;
	fseek(f,pos+0x26,SEEK_SET);
	fread(&h,1,1,f);
	format.hidden=(h==0);

	fseek(f,pos+0xF,SEEK_SET);
	fread(&h,1,1,f);
	format.color=h;

	fseek(f,pos+0x4A,SEEK_SET);
	fread(&w,2,1,f);
	if(IsBigEndian()) SwapBytes(w);
	format.majorTickLength=(double)w/10.0;

	fseek(f,pos+0x15,SEEK_SET);
	fread(&w,2,1,f);
	if(IsBigEndian()) SwapBytes(w);
	format.thickness=(double)w/500.0;

	fseek(f,pos+0x25,SEEK_SET);
	fread(&h,1,1,f);
	format.minorTicksType=(h>>6);
	format.majorTicksType=((h>>4)&3);
	format.axisPosition=(h&0xF);
	switch(format.axisPosition)
	{
		case 1:
			fseek(f,pos+0x37,SEEK_SET);
			fread(&h,1,1,f);
			format.axisPositionValue=(double)h;
			break;
		case 2:
			fseek(f,pos+0x2F,SEEK_SET);
			fread(&p,8,1,f);
			if(IsBigEndian()) SwapBytes(p);
			format.axisPositionValue=p;
			break;
	}
}

void OPJFile::readGraphAxisTickLabelsInfo(graphAxisTick &tick, FILE *f, int pos) {
	unsigned char h;
	unsigned char h1;
	short w;
	fseek(f,pos+0x26,SEEK_SET);
	fread(&h,1,1,f);
	tick.hidden=(h==0);

	fseek(f,pos+0xF,SEEK_SET);
	fread(&h,1,1,f);
	tick.color=h;

	fseek(f,pos+0x13,SEEK_SET);
	fread(&w,2,1,f);
	if(IsBigEndian()) SwapBytes(w);
	tick.rotation=w/10;

	fseek(f,pos+0x15,SEEK_SET);
	fread(&w,2,1,f);
	if(IsBigEndian()) SwapBytes(w);
	tick.fontsize=w;

	fseek(f,pos+0x1A,SEEK_SET);
	fread(&h,1,1,f);
	tick.fontbold=(h&0x8);

	fseek(f,pos+0x23,SEEK_SET);
	fread(&w,2,1,f);
	if(IsBigEndian()) SwapBytes(w);

	fseek(f,pos+0x25,SEEK_SET);
	fread(&h,1,1,f);
	fread(&h1,1,1,f);
	tick.value_type=(h&0xF);

	vector<string> col;
	switch(tick.value_type)
	{
	case 0: //Numeric

		/*switch((h>>4))
		{
			case 0x9:
				tick.value_type_specification=1;
				break;
			case 0xA:
				tick.value_type_specification=2;
				break;
			case 0xB:
				tick.value_type_specification=3;
				break;
			default:
				tick.value_type_specification=0;
		}*/
		if((h>>4)>7)
		{
			tick.value_type_specification=(h>>4)-8;
			tick.decimal_places=h1-0x40;
		}
		else
		{
			tick.value_type_specification=(h>>4);
			tick.decimal_places=-1;
		}

		break;
	case 2: // Time
	case 3: // Date
	case 4: // Month
	case 5: // Day
	case 6: // Column heading
		tick.value_type_specification=h1-0x40;
		break;
	case 1: // Text
	case 7: // Tick-indexed dataset
	case 10: // Categorical
		col=findDataByIndex(w-1);
		if(col.size()>0)
		{
			tick.colName=col[0];
			tick.dataName=col[1];
		}
		break;
	default: // Numeric Decimal 1.000
		tick.value_type=Numeric;
		tick.value_type_specification=0;
		break;
	}
}

void OPJFile::readProjectTree(FILE *f, FILE *debug)
{
	readProjectTreeFolder(f, debug, projectTree.begin());

	fprintf(debug,"Origin project Tree\n");
	tree<projectNode>::iterator sib2=projectTree.begin(projectTree.begin());
    tree<projectNode>::iterator end2=projectTree.end(projectTree.begin());
    while(sib2!=end2)
	{
        for(int i=0; i<projectTree.depth(sib2)-1; ++i)
			fprintf(debug," ");
		fprintf(debug,"%s\n",(*sib2).name.c_str());
        ++sib2;
    }
	fflush(debug);
}

void OPJFile::readProjectTreeFolder(FILE *f, FILE *debug, tree<projectNode>::iterator parent)
{
	int POS=ftell(f);

	double creation_date, modification_date;

	POS+=5;
	fseek(f,POS+0x10,SEEK_SET);
	fread(&creation_date,8,1,f);
	if(IsBigEndian()) SwapBytes(creation_date);

	fread(&modification_date,8,1,f);
	if(IsBigEndian()) SwapBytes(modification_date);

	POS+=0x20+1+5;
	fseek(f,POS,SEEK_SET);

	int namesize;
	fread(&namesize,4,1,f);
	if(IsBigEndian()) SwapBytes(namesize);

	POS+=5;

	// read folder name
	char* name=new char[namesize+1];
	name[namesize]='\0';
	fseek(f,POS,SEEK_SET);
	fread(name,namesize,1,f);
	tree<projectNode>::iterator current_folder=projectTree.append_child(parent, projectNode(name, 1, creation_date, modification_date));
	POS+=namesize+1+5+5;

	int objectcount;
	fseek(f,POS,SEEK_SET);
	fread(&objectcount,4,1,f);
	if(IsBigEndian()) SwapBytes(objectcount);
	POS+=5+5;

	for(int i=0; i<objectcount; ++i)
	{
		POS+=5;
		char c;
		fseek(f,POS+0x2,SEEK_SET);
		fread(&c,1,1,f);
		int objectID;
		fseek(f,POS+0x4,SEEK_SET);
		fread(&objectID,4,1,f);
		if(IsBigEndian()) SwapBytes(objectID);
		if(c==0x10)
		{
			projectTree.append_child(current_folder, projectNode(NOTE[objectID].name, 0));
		}
		else
			projectTree.append_child(current_folder, projectNode(findObjectByIndex(objectID), 0));
		POS+=8+1+5+5;
	}
	fseek(f,POS,SEEK_SET);
	fread(&objectcount,4,1,f);
	if(IsBigEndian()) SwapBytes(objectcount);
	fseek(f,1,SEEK_CUR);
	for(int i=0; i<objectcount; ++i)
		readProjectTreeFolder(f, debug, current_folder);
}

void OPJFile::readWindowProperties(originWindow& window, FILE *f, FILE *debug, int POS, int headersize)
{
	window.objectID=objectIndex;
	objectIndex++;

	fseek(f,POS + 0x1B,SEEK_SET);
	fread(&window.clientRect,8,1,f);
	if(IsBigEndian()) SwapBytes(window.clientRect);

	char c;
	fseek(f,POS + 0x32,SEEK_SET);
	fread(&c,1,1,f);

	if(c&0x01)
		window.state = originWindow::Minimized;
	else if(c&0x02)
		window.state = originWindow::Maximized;

	fseek(f,POS + 0x69,SEEK_SET);
	fread(&c,1,1,f);

	if(c&0x01)
		window.title = originWindow::Label;
	else if(c&0x02)
		window.title = originWindow::Name;
	else
		window.title = originWindow::Both;

	window.bHidden = (c&0x08);
	if(window.bHidden)
	{
		fprintf(debug,"			WINDOW %d NAME : %s	is hidden\n", objectIndex, window.name.c_str());
		fflush(debug);
	}

	fseek(f,POS + 0x73,SEEK_SET);
	fread(&window.creation_date,8,1,f);
	if(IsBigEndian()) SwapBytes(window.creation_date);

	fread(&window.modification_date,8,1,f);
	if(IsBigEndian()) SwapBytes(window.modification_date);
	
	if(headersize > 0xC3)
	{
		int labellen = 0;
		fseek(f,POS + 0xC3,SEEK_SET);
		fread(&c,1,1,f);
		while (c != '@')
		{
			fread(&c,1,1,f);
			labellen++;
		}
		if(labellen > 0)
		{
			char *label=new char[labellen+1];
			label[labellen]='\0';
			fseek(f,POS + 0xC3,SEEK_SET);
			fread(label,labellen,1,f);
			window.label=label;
			delete label;
		}
		else
			window.label="";
		fprintf(debug,"			WINDOW %d LABEL: %s\n", objectIndex, window.label.c_str());
		fflush(debug);
	}
}
bool OPJFile::IsBigEndian()
{
   short word = 0x4321;
   if((*(char *)& word) != 0x21 )
     return true;
   else
     return false;
}

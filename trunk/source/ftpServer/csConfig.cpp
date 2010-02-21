/****************************************************************************
 * Copyright (C) 2009
 * by LilouMaster 
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 *
 * for WiiXplorer 2009
 ***************************************************************************/


#include "csString.h"
#include "csConfig.h"


bool cfgReadIni(CsString *FilePath, CsString *Section, CsString *Entries, CsString * Value)
{
	CsString Line;
	int LongEntries;
	FILE *file;
	char n[50];

	CsString temp("[");

	if (Section->Left(1) != "[" ) {
		*Section = temp + *Section  ;
	}
	if (Section->Right(1) != "]" ) {
		*Section += "]" ;
	}
	*Entries += "=";
	LongEntries = Entries->GetLength();
	if ((file = fopen(*FilePath,"r")) != NULL ){
		while (file && fgets(n, 50, file) ) {
			Line = n; 
			Line.Remove('\n');Line.Remove('\r');
			if (Line == *Section) { // Section trouvée, recherche de l'entrée
				while (file && fgets(n, 50, file) ) {
					Line = n;	
					Line.Remove('\n');Line.Remove('\r');
					if (Line.Left(1) == "[") {
						// fin de la recherche si atteint le debut de la section suivante
						fclose(file);
						return false;
					}
					if (Line.Left(LongEntries) == *Entries) {
						// Lecture valeur 
						*Value = Line.Right( Line.GetLength() - LongEntries);
						fclose(file);
						return true;
					}
				}
			}
		}
		fclose(file);
	} 

	return false;
}


bool cfgWriteIni(CsString *FilePath, CsString *Section, CsString *Entries, CsString *Value)
{
	CsString Line(50);
	int LongEntries;
	FILE *sfile,*dfile;
	char n[50];
	CsString dFilePath;
	bool ret = false;

	sfile= NULL;
	dfile = NULL;

	bool SectionFind = false;
	CsString temp("[");

	if (Section->Left(1) != "[" ) {
		*Section = temp + *Section  ;
	}
	if (Section->Right(1) != "]" ) {
		*Section += "]" ;
	}
	*Entries += "=";
	LongEntries = Entries->GetLength();

	dFilePath = *FilePath + "z"; 
	if ((dfile = fopen(dFilePath,"w")) == NULL) { 
		return false;
	}

	if ((sfile = fopen(*FilePath,"r")) != NULL) { 
		while (sfile && fgets(n, 50, sfile) ) { // Recherche de la section
			Line = n;
			Line.Remove('\n');Line.Remove('\r');
			if (Line == *Section) { // Section trouvée
				SectionFind = true;
				fwrite(Line, Line.GetLength(),1,  dfile);
				while (sfile && fgets(n, 50, sfile) ) { // // Section trouvée, recherche de l'entrée
					Line = n;	
					Line.Remove('\n');Line.Remove('\r');
					if (Line.GetLength() != 0) {
						if (Line.Left(1) == "[" ) { // Fin de section trouvée
							fwrite("\n", 1, 1, dfile);
							fwrite(*Entries, Entries->GetLength(), 1, dfile);
							fwrite(*Value, Value->GetLength(), 1, dfile);
							fwrite("\n", 1, 1, dfile);
							fwrite(Line, Line.GetLength(), 1, dfile);
							fwrite("\n", 1, 1, dfile);
							while (sfile && fgets(n, 50, sfile) ) {
								Line = n;	
								fwrite(Line, Line.GetLength(), 1, dfile);
							}
							fclose (sfile);
							fclose (dfile);
							ret= true;
							goto end_prj;
						}
						if (Line.Left(LongEntries) == *Entries) { // Entrée trouvée
							fwrite("\n", 1, 1, dfile);
							fwrite(*Entries, Entries->GetLength(), 1, dfile);
							fwrite(*Value, Value->GetLength(), 1, dfile);
							fwrite("\n", 1, 1, dfile);
							while (sfile && fgets(n, 50, sfile) ) {
								Line = n;	
								fwrite(Line, Line.GetLength(), 1, dfile);
							}
							fclose (sfile);
							fclose (dfile);
							ret= true;
							goto end_prj;
						}
						fwrite("\n", 1, 1, dfile);
						fwrite(Line, Line.GetLength(), 1, dfile);
					}
				} // Section trouvée, fin de la recherche de l'entrée
				fwrite("\n", 1, 1, dfile);
				fwrite(*Entries, Entries->GetLength(), 1, dfile);
				fwrite(*Value, Value->GetLength(), 1, dfile);
				fclose(sfile);
				fclose(dfile);
				ret= true;
				goto end_prj;
			}
			else { 	// Section non trouvée
				fwrite(Line, Line.GetLength(), 1, dfile);
				fwrite("\n", 1, 1, dfile);
			}
		} // Fin de la recherche de la section
		// on n'a rien trouvé dans le fichier source
	} 
	fwrite("\n", 1, 1, dfile);
	fwrite(*Section, Section->GetLength(), 1, dfile);
	fwrite("\n", 1, 1, dfile);
	fwrite(*Entries, Entries->GetLength(), 1, dfile);
	fwrite(*Value, Value->GetLength(), 1, dfile);

	ret= true;
	goto end_prj;

end_prj: 
	if (sfile!=NULL) fclose (sfile);
	if (dfile!=NULL) fclose (dfile);



	if (ret == true) 
	{
		remove(*FilePath);

		//if (!(tt=rename(dFilePath, *FilePath))) 
		{
			int len;
			sfile=fopen(dFilePath,"r");
			dfile=fopen(*FilePath,"w");
			do {
				len=fread(n, 1, 50, sfile);
				fwrite(n, 1, len, dfile);
			} while(!feof(sfile));
			fclose(sfile);
			fclose(dfile);
			remove(dFilePath);
		}
	}


	return ret;
}




bool blocConfig:: cfgOpen(CsString *FilePath, CsString *Section) 
{ 
	CsString Line(50); 
	FILE *sfile; 
	char n[50]; 
	bool ret = false; 

	sfile= NULL; 
	dfile = NULL; 
	dSection = *Section; 

	CsString temp("["); 
	if (dSection.Left(1) != "[" ) 
	{ 
		dSection = temp + dSection  ; 
	} 
	if (dSection.Right(1) != "]" ) 
	{ 
		dSection += "]" ; 
	} 

	sFilePath = *FilePath; 
	dFilePath = *FilePath + "z"; 
	if ((dfile = fopen(dFilePath,"w")) == NULL) 
	{ 
		return false; 
	} 

	if ((sfile = fopen(sFilePath,"r")) != NULL) 
	{ 
		while (sfile && fgets(n, 50, sfile) )  // Recherche de la section 
		{ 
			Line = n; 
			Line.Remove('\n');Line.Remove('\r'); 
			if (Line == dSection) 
			{ // Section trouvée 
				ret= true; 
				goto end_prj; 
			} 
			else 
			{ // Section non trouvée 
				fwrite(Line, Line.GetLength(), 1, dfile); 
				fwrite("\n", 1, 1, dfile); 
			} 
		} 
	} 

end_prj: 

	fclose(sfile); 

	fwrite("\n", 1, 1,  dfile); 
	fwrite(dSection, dSection.GetLength(), 1, dfile); 
	fwrite("\n", 1, 1,  dfile); 
	return true; 
} 



bool blocConfig:: cfgWriteEntries( CsString *Entries,  CsString *Value) 
{ 
	if (dfile==NULL) return false; 

	fwrite("\n", 1, 1,  dfile); 
	fwrite(*Entries, Entries->GetLength(), 1, dfile); 
	fwrite("=", 1,  1, dfile); 
	fwrite(*Value, Value->GetLength(), 1, dfile); 

	return true; 
} 


bool blocConfig:: cfgClose(void) 
{ 
	CsString Line(50); 
	FILE *sfile; 
	char n[50]; 
	bool ret = false; 

	fwrite("\n", 1, 1,  dfile); 
	if (dfile==NULL) return false; 

	if ((sfile = fopen(sFilePath,"r")) != NULL) 
	{ 
		while (sfile && fgets(n, 50, sfile) ) // Recherche de la section 
		{ 
			Line = n; 
			Line.Remove('\n');Line.Remove('\r'); 
			if (Line == dSection)  // Section trouvée 
			{ 
				while (sfile && fgets(n, 50, sfile) )  // // Section trouvée, recherche prochaine section

				{ 
					Line = n;       
					Line.Remove('\n');Line.Remove('\r'); 
					if (Line.GetLength() != 0) 
					{ 
						if (Line.Left(1) == "[" )  // Fin de section trouvée 
						{ 
							fwrite(Line, Line.GetLength(),1,  dfile); 
							fwrite("\n", 1, 1, dfile); 
							while (sfile && fgets(n, 50, sfile) ) 
							{ 
								Line = n;       
								fwrite(Line, Line.GetLength(), 1, dfile); 
							} 
							goto end_prj; 
						} 
					} 
				} 
			} 
		} 
	} 



end_prj: 

	if (sfile!=NULL) fclose (sfile); 
	if (dfile!=NULL) { ret = true; fclose (dfile); } 

	//int tt; 

	if (ret == true) 
	{ 
		remove(sFilePath); 

		//if (!(tt=rename(dFilePath, *FilePath))) 
		{ 
			int len; 
			sfile=fopen(dFilePath,"r"); 
			dfile=fopen(sFilePath,"w"); 
			do 
			{ 
				len=fread(n, 1, 50, sfile); 
				fwrite(n, 1, len, dfile); 
			} while(!feof(sfile)); 
			fclose(sfile); 
			fclose(dfile); 
			remove(dFilePath); 
		} 
	} 

	dfile=NULL; 

	return true; 
} 


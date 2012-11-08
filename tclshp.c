/*
    Tclshp, a Tcl API for shapelib
    Copyright (C) 2006  Devin J. Eyre

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/
#include <math.h>
#include <string.h>
#include "shapefil.h"
#include <tcl.h>

/* -------------------------------------------------------------------- */
/*     Start of SHP section                                             */
/* -------------------------------------------------------------------- */

int Shpcreate (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    SHPHandle	hSHP;
    int		nShapeType;
    /*char	*shpfilename;*/
    if (objc != 3) {
	Tcl_WrongNumArgs(interp, 1, objv, "filename, shapeType");
		return TCL_ERROR;
    }
/* -------------------------------------------------------------------- */
/*	Figure out the shape type.					*/
/* -------------------------------------------------------------------- */
    
    if (Tcl_GetIntFromObj(interp, objv[2], &nShapeType) != TCL_OK)
        return TCL_ERROR;
    if (nShapeType != SHPT_POINT && nShapeType != SHPT_ARC && nShapeType != SHPT_POLYGON && nShapeType == !SHPT_MULTIPOINT) 
        return TCL_ERROR;
   /* if (Tcl_GetStringFromObj(objv[1], NULL) != TCL_OK)
        return TCL_ERROR;*/
/* -------------------------------------------------------------------- */
/*	Create the requested layer.					*/
/* -------------------------------------------------------------------- */
    hSHP = SHPCreate( Tcl_GetStringFromObj(objv[1], NULL), nShapeType );

    if( hSHP == NULL )
    {
	printf( "Unable to create:%s\n", Tcl_GetStringFromObj(objv[1],NULL) );
	return TCL_ERROR;
    }
    /*printf( "Created: %s\n", shpfilename );*/

    SHPClose( hSHP );
    return TCL_OK;
}

int Shpadd (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv []) {
   SHPHandle  hSHP;
   int	       nShapeType, nVertices, nParts, nPartIndicesMax, *partIndices, i, nVMax;
   double     *padfX, *padfY;
   SHPObject  *psObject;

/* -------------------------------------------------------------------- */
/*      Display a usage message.                                        */
/* -------------------------------------------------------------------- */
   if (objc < 2) {
      Tcl_WrongNumArgs (interp, 1, objv, "shp_file [[x y] [+]]*\n");
      return TCL_ERROR;
   }
      
/* -------------------------------------------------------------------- */
/*      Open the passed shapefile.                                      */
/* -------------------------------------------------------------------- */
   hSHP = SHPOpen( Tcl_GetStringFromObj(objv[1], NULL), "r+b" );

   if( hSHP == NULL ) {
      printf( "Unable to open:%s\n", Tcl_GetStringFromObj(objv[1], NULL));
      return TCL_ERROR;
   }

   SHPGetInfo( hSHP, NULL, &nShapeType, NULL, NULL );
     
   if( objc == 2 )
      nShapeType = SHPT_NULL;
	
/* -------------------------------------------------------------------- */
/*	Build a vertex/part list from the command line arguments.	*/
/* -------------------------------------------------------------------- */

   nVMax = 1000;
   padfX = (double *) ckalloc(sizeof(double) * nVMax);
   padfY = (double *) ckalloc(sizeof(double) * nVMax);
   nVertices = 0;

   nPartIndicesMax = 100;
   partIndices = (int *) ckalloc(sizeof(int) * nPartIndicesMax);
   nParts = 1;
   partIndices[0] = 0;

   for( i = 2; i < objc;  ) {
       if( strcmp(Tcl_GetStringFromObj(objv[i], NULL),"+") == 0 ) {
		if( nParts == nPartIndicesMax ) {
			nPartIndicesMax *= 2;
			partIndices = (int *) ckrealloc((char *)partIndices, sizeof(int) * nPartIndicesMax);
		}
	    partIndices[nParts++] = nVertices;
	  	i++;
      } else if( i < objc-1 ) {
         if( nVertices == nVMax ) {
            nVMax = nVMax * 2;
            padfX = (double *) ckrealloc((char *)padfX,sizeof(double)*nVMax);
            padfY = (double *) ckrealloc((char *)padfY,sizeof(double)*nVMax);
         }

	 if (Tcl_GetDoubleFromObj(interp, objv[i], padfX+nVertices) != TCL_OK) {
             SHPClose( hSHP );
             return TCL_ERROR;
         }
	 if (Tcl_GetDoubleFromObj(interp, objv[i+1], padfY+nVertices) != TCL_OK) {
             SHPClose( hSHP );
             return TCL_ERROR;
         }
	 nVertices += 1;
         i += 2;
      }
   }

/* -------------------------------------------------------------------- */
/*      Write the new entity to the shape file.                         */
/* -------------------------------------------------------------------- */
    psObject = SHPCreateObject( nShapeType, -1, nParts, partIndices, NULL,
                                nVertices, padfX, padfY, NULL, NULL );
    SHPWriteObject( hSHP, -1, psObject );
    SHPDestroyObject( psObject );
    
    SHPClose( hSHP );

    ckfree( (char *)partIndices );
    ckfree( (char *)padfX );
    ckfree( (char *)padfY );

    return TCL_OK;
}

int Shpinfo (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv []) {

   SHPHandle	hSHP;
   int	   	nShapeType, nEntities, i, recNum;
   double 	adfMinBound[4], adfMaxBound[4];
   Tcl_Obj 	*resultPtr;
   SHPObject	*psShape;
 
/* -------------------------------------------------------------------- */
/*      Display a usage message.                                        */
/* -------------------------------------------------------------------- */
   if( objc < 2 ) {
      Tcl_WrongNumArgs (interp, 1, objv, "shp_file" );
      return TCL_ERROR;	
   }
   
    
/* -------------------------------------------------------------------- */
/*      Open the passed shapefile.                                      */
/* -------------------------------------------------------------------- */
   hSHP = SHPOpen( Tcl_GetStringFromObj(objv[1], NULL), "rb" );

   if( hSHP == NULL ) {
      printf( "Unable to open:%s\n", Tcl_GetStringFromObj(objv[1], NULL) );
      return TCL_ERROR;
   }

/* -------------------------------------------------------------------- */
/*      Print out the file bounds.                                      */
/* -------------------------------------------------------------------- */
   SHPGetInfo( hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound ); 
   
   resultPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
     
   if (objc == 2) {
      Tcl_ListObjAppendElement(interp, resultPtr, Tcl_NewIntObj(nShapeType));
      Tcl_ListObjAppendElement(interp, resultPtr, Tcl_NewIntObj(nEntities));
      for (i = 0; i < 2; i++) {
         Tcl_ListObjAppendElement(interp, resultPtr, Tcl_NewDoubleObj(adfMinBound[i]));
      }
      for (i = 0; i < 2; i++) {
         Tcl_ListObjAppendElement(interp, resultPtr, Tcl_NewDoubleObj(adfMaxBound[i]));
      }
      Tcl_SetObjResult(interp, resultPtr);
   }
/* -------------------------------------------------------------------- */
/*	Skim over the list of shapes, printing all the vertices.	*/
/* -------------------------------------------------------------------- */
   if (objc >= 3) {
   
      if (Tcl_GetIntFromObj(interp, objv[2], &recNum) != TCL_OK) {
         SHPClose( hSHP );
         return TCL_ERROR;
      }
      i = recNum - 1;
      if (recNum > nEntities) {
         SHPClose( hSHP );
         return TCL_ERROR;
      }

      psShape = SHPReadObject( hSHP, i );
      Tcl_ListObjAppendElement(interp, resultPtr, Tcl_NewIntObj( psShape->nVertices));
      Tcl_ListObjAppendElement(interp, resultPtr, Tcl_NewIntObj( psShape->nParts));
      Tcl_ListObjAppendElement(interp, resultPtr, Tcl_NewDoubleObj( psShape->dfXMin));
      Tcl_ListObjAppendElement(interp, resultPtr, Tcl_NewDoubleObj( psShape->dfYMin));
      Tcl_ListObjAppendElement(interp, resultPtr, Tcl_NewDoubleObj( psShape->dfXMax));
      Tcl_ListObjAppendElement(interp, resultPtr, Tcl_NewDoubleObj( psShape->dfYMax));
      SHPDestroyObject( psShape );
   }
   Tcl_SetObjResult(interp, resultPtr);
   SHPClose( hSHP );

   return TCL_OK;
}
  
int Shpget (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv []) {
   SHPHandle	hSHP;
   int	   nShapeType, nEntities, i, iPart, recNum;
   double 	adfMinBound[4], adfMaxBound[4];
   Tcl_Obj 	*resultPtr;
   Tcl_Obj 	*recListPtr;
   Tcl_Obj 	*partListPtr;
   SHPObject	*psShape;

/* -------------------------------------------------------------------- */
/*      Display a usage message.                                        */
/* -------------------------------------------------------------------- */
   if( objc < 2 || objc > 3) {
      Tcl_WrongNumArgs (interp, 1, objv, "shp_file [recordNumber]" );
      return TCL_ERROR;	
    }
   
    
/* -------------------------------------------------------------------- */
/*      Open the passed shapefile.                                      */
/* -------------------------------------------------------------------- */
   hSHP = SHPOpen( Tcl_GetStringFromObj(objv[1], NULL), "rb" );

   if (hSHP == NULL ) {
      printf( "Unable to open:%s\n", Tcl_GetStringFromObj(objv[1], NULL) );
      return TCL_ERROR;
   }

/* -------------------------------------------------------------------- */
/*      Print out the file bounds.                                      */
/* -------------------------------------------------------------------- */
   SHPGetInfo( hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound ); 
   resultPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
     
   if (objc == 2) {
/* -------------------------------------------------------------------- */
/*      Return ALL shapes in shpfile                                    */
/* -------------------------------------------------------------------- */
      for (i = 0; i < nEntities; i++) {
         int		j;
         /*SHPObject	*psShape;*/

         psShape = SHPReadObject( hSHP, i );
         
   	 recListPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);

         partListPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
         for (j = 0, iPart= 1; j < psShape->nVertices; j++) {
            if (iPart < psShape->nParts && psShape->panPartStart[iPart] == j) {
               Tcl_ListObjAppendElement(interp, recListPtr, partListPtr);
   	       partListPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
            }
            Tcl_ListObjAppendElement(interp, partListPtr, Tcl_NewDoubleObj(psShape->padfX[j]));
            Tcl_ListObjAppendElement(interp, partListPtr, Tcl_NewDoubleObj(psShape->padfY[j]));
	 }
         Tcl_ListObjAppendElement(interp, recListPtr, partListPtr);
         Tcl_ListObjAppendElement(interp, resultPtr, recListPtr);
         SHPDestroyObject( psShape );
      }
   } else {
/* -------------------------------------------------------------------- */
/*	Return shape #recNum.                                          	*/
/* -------------------------------------------------------------------- */
      if (Tcl_GetIntFromObj(interp, objv[2], &recNum) != TCL_OK) {
         SHPClose( hSHP );           
         return TCL_ERROR;
      }
      if ((recNum >= nEntities) || (recNum < 0)) {
         SHPClose( hSHP );
         return TCL_ERROR;
      }

      psShape = SHPReadObject( hSHP, recNum );

      partListPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
      for (i = 0, iPart= 1; i < psShape->nVertices; i++) {
         if (iPart < psShape->nParts && psShape->panPartStart[iPart] == i) {
            Tcl_ListObjAppendElement(interp, resultPtr, partListPtr);
            partListPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
         }
         Tcl_ListObjAppendElement(interp, partListPtr, Tcl_NewDoubleObj(psShape->padfX[i]));
         Tcl_ListObjAppendElement(interp, partListPtr, Tcl_NewDoubleObj(psShape->padfY[i]));
      }
      Tcl_ListObjAppendElement(interp, resultPtr, partListPtr);
      SHPDestroyObject( psShape );
   }
   Tcl_SetObjResult(interp, resultPtr);
   SHPClose( hSHP );

   return TCL_OK;
}

/* -------------------------------------------------------------------- */
/*     Start of DBF section                                             */
/* -------------------------------------------------------------------- */
int Dbfcreate (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
   DBFHandle	hDBF;
   int		i, field_length, decimal_scale;
   if (objc < 2) {
      printf( "dbfcreate xbase_file [[-s field_name width].[-n field_name width decimals]]...\n");
      return TCL_ERROR;
   }
/* -------------------------------------------------------------------- */
/*      Create the database.                                            */
/* -------------------------------------------------------------------- */
   hDBF = DBFCreate(Tcl_GetStringFromObj(objv[1],NULL));
   if (hDBF == NULL) {
      printf( "DBFCreate(%s) failed.\n", Tcl_GetStringFromObj(objv[1],NULL));
      return TCL_ERROR;
   }
/* -------------------------------------------------------------------- */
/*      Loop over the field definitions adding new fields.              */
/* -------------------------------------------------------------------- */
   for (i = 2; i < objc; i++) {
      if (strcmp(Tcl_GetStringFromObj(objv[i],NULL),"-s") == 0 && i < objc-2) {
         if (Tcl_GetIntFromObj(interp, objv[i+2], &field_length) != TCL_OK) {
            printf( "field_length must be an integer %s!\n", Tcl_GetStringFromObj(objv[i+1], NULL));
            return TCL_ERROR;
         }
         if (DBFAddField( hDBF, Tcl_GetStringFromObj(objv[i+1],NULL), FTString, field_length, 0 ) == -1 ) {
                printf( "DBFAddField(%s,FTString,%d,0) failed.\n", Tcl_GetStringFromObj(objv[i+1], NULL), field_length );
                return TCL_ERROR;
         }
         i = i + 2;
      } else if( strcmp(Tcl_GetStringFromObj(objv[i], NULL),"-n") == 0 && i < objc-3 ) {
         if (Tcl_GetIntFromObj(interp, objv[i+2], &field_length) != TCL_OK) {
            printf( "field_length must be an integer %s!\n", Tcl_GetStringFromObj(objv[i+2], NULL));
            return TCL_ERROR;
         }
         if (Tcl_GetIntFromObj(interp, objv[i+3], &decimal_scale) != TCL_OK) {
            printf( "decimal_scale must be an integer %s!\n", Tcl_GetStringFromObj(objv[i+3], NULL));
            return TCL_ERROR;
         }
         if ( DBFAddField( hDBF, Tcl_GetStringFromObj(objv[i+1],NULL), FTDouble, field_length, decimal_scale) == -1 ) {
            printf ("DBFAddField(%s,FTDouble,%d,%d) failed.\n", Tcl_GetStringFromObj(objv[i+1],NULL), field_length, decimal_scale);
            return TCL_ERROR;
         }
         i = i + 3;
      } else {
         printf( "Argument incomplete, or unrecognised:%s\n", Tcl_GetStringFromObj(objv[i],NULL) );
         return TCL_ERROR;
      }
   }
   DBFClose( hDBF );
   return TCL_OK;
}

int Dbfadd (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
   DBFHandle	hDBF;
   int		i, iRecord;
   double	dblvalue;
   char	fieldName[12];
   
/* -------------------------------------------------------------------- */
/*      Display a usage message.                                        */
/* -------------------------------------------------------------------- */
    if( objc < 3 ) {
	Tcl_WrongNumArgs(interp, 1, objv, "xbase_file field_values" );
        return TCL_ERROR;
    }

/* -------------------------------------------------------------------- */
/*      Create the database.                                            */
/* -------------------------------------------------------------------- */
    hDBF = DBFOpen( Tcl_GetStringFromObj(objv[1], NULL), "r+b" );
    if (hDBF == NULL) {
        printf( "DBFOpen(%s,\"rb+\") failed.\n", Tcl_GetStringFromObj(objv[1],NULL) );
        return TCL_ERROR;
    }

/* -------------------------------------------------------------------- */
/*      Do we have the correct number of arguments?                     */
/* -------------------------------------------------------------------- */
    if( DBFGetFieldCount( hDBF ) != objc - 2 ) {
        printf( "Got %d fields, but require %d\n", objc - 2, DBFGetFieldCount( hDBF ) );
        return TCL_ERROR;
    }

    iRecord = DBFGetRecordCount( hDBF );

/* -------------------------------------------------------------------- */
/*      Loop assigning the new field values.                            */
/* -------------------------------------------------------------------- */
   for( i = 0; i < DBFGetFieldCount(hDBF); i++ ) {
      if( strcmp( Tcl_GetStringFromObj(objv[i+2], NULL), "" ) == 0 ) {
         DBFWriteNULLAttribute(hDBF, iRecord, i );
      } else if( DBFGetFieldInfo( hDBF, i, fieldName, NULL, NULL ) == FTString ) {
         DBFWriteStringAttribute(hDBF, iRecord, i, Tcl_GetStringFromObj(objv[i+2],NULL));
      } else {
         if (Tcl_GetDoubleFromObj(interp, objv[i+2], &dblvalue) != TCL_OK) {
            printf("Expected a number for field %s, but got %s.\n", fieldName, Tcl_GetStringFromObj(objv[i+2],NULL));
            return TCL_ERROR;
         }
         DBFWriteDoubleAttribute(hDBF, iRecord, i, dblvalue);
      }
   }

/* -------------------------------------------------------------------- */
/*      Close and cleanup.                                              */
/* -------------------------------------------------------------------- */
    DBFClose( hDBF );

    return TCL_OK;
}

int Dbfinfo (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
   DBFHandle	hDBF;
   int		i;
   char		*pszFilename = NULL;
   int		nWidth, nDecimals;
   char		szTitle[12];
   Tcl_Obj	*resultPtr;
   Tcl_Obj	*sublistPtr;
   if (objc) {
      pszFilename = Tcl_GetStringFromObj(objv[1],NULL);
      hDBF = DBFOpen( pszFilename, "rb" );
      if (hDBF == NULL) {
         printf( "DBFOpen(%s,\"r\") failed.\n", pszFilename);
         return TCL_ERROR;
      }
      resultPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
      if (DBFGetFieldCount(hDBF) == 0) {
         DBFClose( hDBF );
         return TCL_OK;
      }
      Tcl_ListObjAppendElement(interp, resultPtr, Tcl_NewIntObj(DBFGetRecordCount(hDBF)));
      for (i = 0; i < DBFGetFieldCount(hDBF); i++ ) {
         DBFFieldType        eType;
         const char          *pszTypeName;
         
         eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );
         if (eType == FTString)
             pszTypeName = "String";
         else if (eType == FTInteger)
             pszTypeName = "Integer";
         else if (eType == FTDouble)
             pszTypeName = "Double";
         else if (eType == FTInvalid)
             pszTypeName = "Invalid";

         sublistPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
         Tcl_ListObjAppendElement(interp, sublistPtr, Tcl_NewStringObj(szTitle, strlen(szTitle)));
         Tcl_ListObjAppendElement(interp, sublistPtr, Tcl_NewStringObj(pszTypeName, strlen(pszTypeName)));
         Tcl_ListObjAppendElement(interp, sublistPtr, Tcl_NewIntObj(nWidth));
         Tcl_ListObjAppendElement(interp, sublistPtr, Tcl_NewIntObj(nDecimals));
         Tcl_ListObjAppendElement(interp, resultPtr, sublistPtr);
      }
      Tcl_SetObjResult(interp, resultPtr);
      DBFClose( hDBF );
      return TCL_OK;
   } else {
      return TCL_ERROR;
   }
}

int Dbfget (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
   DBFHandle	hDBF;
   int		i, iRecord;
   char		*pszFilename = NULL;
   int		nWidth, nDecimals;
   char		szTitle[12];
   int          intvalue;
   double       dblvalue;
   Tcl_Obj	*resultPtr;
   Tcl_Obj	*sublistPtr;
/* -------------------------------------------------------------------- */
/*   char         *strvalue = NULL;*/
/*      Display a usage message.                                        */
/* -------------------------------------------------------------------- */
   if( objc < 2 || objc > 3) {
      Tcl_WrongNumArgs(interp, 1, objv, "xbase_file [record_number]" );
      return TCL_ERROR;
   }
   pszFilename = Tcl_GetStringFromObj(objv[1],NULL);
   hDBF = DBFOpen( pszFilename, "rb" );
   if (hDBF == NULL) {
      printf( "DBFOpen(%s,\"r\") failed.\n", pszFilename);
      return TCL_ERROR;
   }
   if (DBFGetFieldCount(hDBF) == 0 || DBFGetFieldCount(hDBF) == 0) {
      DBFClose( hDBF );
      return TCL_OK;
   }
   resultPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
   if (objc == 2) {
      for (iRecord = 0; iRecord < DBFGetRecordCount(hDBF); iRecord++) {
         sublistPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
         for (i = 0; i < DBFGetFieldCount(hDBF); i++ ) {
            DBFFieldType        eType;
            const char          *strvalue;
      
            eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );
            if (eType == FTString) {
               strvalue = DBFReadStringAttribute(hDBF, iRecord, i);
               Tcl_ListObjAppendElement(interp, sublistPtr, Tcl_NewStringObj(strvalue, strlen(strvalue)));
            } else if (eType == FTInteger) {
               intvalue = DBFReadIntegerAttribute(hDBF, iRecord, i);
               Tcl_ListObjAppendElement(interp, sublistPtr, Tcl_NewIntObj(intvalue));
            } else if (eType == FTDouble) {
               dblvalue = DBFReadDoubleAttribute(hDBF, iRecord, i);
               Tcl_ListObjAppendElement(interp, sublistPtr, Tcl_NewDoubleObj(dblvalue));
            } else {
               Tcl_ListObjAppendElement(interp, sublistPtr, Tcl_NewStringObj("", 0));
            }
         }
         Tcl_ListObjAppendElement(interp, resultPtr, sublistPtr);
      }
   } else {
      if (Tcl_GetIntFromObj(interp, objv[2], &iRecord) != TCL_OK) {
         DBFClose( hDBF );
         return TCL_ERROR;
      }
      if ((iRecord >= DBFGetRecordCount(hDBF)) || (iRecord < 0)) {
         DBFClose( hDBF );
         return TCL_ERROR;
      }
      for (i = 0; i < DBFGetFieldCount(hDBF); i++ ) {
         DBFFieldType        eType;
         const char          *strvalue;
      
         eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );
         if (eType == FTString) {
            strvalue = DBFReadStringAttribute(hDBF, iRecord, i);
            Tcl_ListObjAppendElement(interp, resultPtr, Tcl_NewStringObj(strvalue, strlen(strvalue)));
         } else if (eType == FTInteger) {
            intvalue = DBFReadIntegerAttribute(hDBF, iRecord, i);
            Tcl_ListObjAppendElement(interp, resultPtr, Tcl_NewIntObj(intvalue));
         } else if (eType == FTDouble) {
            dblvalue = DBFReadDoubleAttribute(hDBF, iRecord, i);
            Tcl_ListObjAppendElement(interp, resultPtr, Tcl_NewDoubleObj(dblvalue));
         } else {
            Tcl_ListObjAppendElement(interp, resultPtr, Tcl_NewStringObj("", 0));
         }
      }
   }
   Tcl_SetObjResult(interp, resultPtr);
   DBFClose( hDBF );
   return TCL_OK;
}

/* -------------------------------------------------------------------- */
/*     End of DBF section                                               */
/* -------------------------------------------------------------------- */
int Tclshp_Init (Tcl_Interp *interp) {
   Tcl_CreateObjCommand(interp, "shpcreate", Shpcreate, NULL, NULL);
   Tcl_CreateObjCommand(interp, "shpadd", Shpadd, NULL, NULL);
   Tcl_CreateObjCommand(interp, "shpinfo", Shpinfo, NULL, NULL);
   Tcl_CreateObjCommand(interp, "shpget", Shpget, NULL, NULL);
   Tcl_CreateObjCommand(interp, "dbfcreate", Dbfcreate, NULL, NULL);
   Tcl_CreateObjCommand(interp, "dbfadd", Dbfadd, NULL, NULL);
   Tcl_CreateObjCommand(interp, "dbfinfo", Dbfinfo, NULL, NULL);
   Tcl_CreateObjCommand(interp, "dbfget", Dbfget, NULL, NULL);
   return TCL_OK;
}

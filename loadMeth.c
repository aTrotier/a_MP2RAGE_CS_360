/****************************************************************
 *
 * Copyright (c) 2002-2021
 * Bruker BioSpin MRI GmbH
 * D-76275 Ettlingen, Germany
 *
 * All Rights Reserved
 *
 *
 ****************************************************************/


#define DEBUG		0


#include "method.h"

/*:=MPB=:=======================================================*
 *
 * Global Function: loadMeth
 *
 * Description: This procedure is automatically called in
 *	response to a method file for this method being read.
 *
 * Error History: 
 *
 * Interface:							*/

void loadMeth(const char *	className)
/*:=MPE=:=======================================================*/
{
  DB_MSG(( "-->FLASH:loadMeth( %s )\n", className ));



  if (0 != className)
  {
      if (0 == strcmp( className, "MethodClass"))
      {
        MRT_CheckOperationMode();
        initMeth();
      }
      else if (0 == strcmp(className, "MethodRecoGroup"))
      {
          DB_MSG(("...responding to loadMeth call for MethodRecoGroup."));
          // Adapt parent dataset path.
          if (ParxRelsParHasValue("ParentDset"))
              PvAdjManCorrectProcnoResultPath(ParentDset, study_result);
          SetRecoParam();
      }
  }
  else
  {
    DB_MSG(( "...ignoring loadMeth call - I don't know this class" ));
  }

  DB_MSG(( "<--FLASH:loadMeth( %s )\n", className ));

}

/****************************************************************/
/*		E N D   O F   F I L E				*/
/****************************************************************/



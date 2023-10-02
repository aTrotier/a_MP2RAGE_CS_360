/* ***************************************************************
 *
 * Copyright (c) 2022
 * Bruker BioSpin MRI GmbH
 * D-76275 Ettlingen, Germany
 *
 * All Rights Reserved
 *
 * ***************************************************************/
void InitCustom();

#define DEBUG	 0

#include "method.h"

/*===============================================================
 * InitCustom
 * 
 *===============================================================*/
void InitCustom()
{
    strncpy(ReleaseVersion, "v0.0.1", 9);
}

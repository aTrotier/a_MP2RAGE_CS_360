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
void ExcPulse2EnumRelation();
void ExcPulse2AmplRel();
void ExcPulse2Relation();
void ExcPulse2Range();
void HandleRFPulse2Amplitude();

#define DEBUG	 0

#include "method.h"

/*===============================================================
 * InitCustom
 * 
 *===============================================================*/
void InitCustom()
{
  strncpy(ReleaseVersion, "v0.0.1", 9);
  ParxRelsParMakeNonEditable({"ReleaseVersion"});
  //ParxRelsShowInEditor("ReleaseVersion"); //ParxRelsHideInEditor

  MP2_NTI = 2;
  ParxRelsParMakeNonEditable({"MP2_NTI"}); 

  if(ParxRelsParHasValue("MP2_EchoTrainLength") == No)
  {
    MP2_EchoTrainLength = PVM_Matrix[1];
  }

  STB_InitRFPulse("ExcPulse2",      // name of pulse structure
                "ExcPulse2Enum",  // name of pulse list parameter
                "ExcPulse2Ampl",  // name of pulse amplitude parameter
                "ExcPulse2Shape", // name of pulse shape (for calc. pulses)
                0,                // used for excitation
                "Calculated",     // default shape
                3000.0,           // default bandwidth
                7.0);            // default pulse angle

  // check valid range for this specific pulse see parsRelations.c
  ExcPulse2Range();
}

/*===============================================================
 * RF functions
 * copy from parsRelations.c
 *===============================================================*/
void ExcPulse2EnumRelation(void)
{
  DB_MSG(("-->ExcPulse2EnumRelation"));

  UT_SetRequest("ExcPulse2Enum");
  backbone();
  
  DB_MSG(("<--ExcPulse2EnumRelation"));                                       
}


void ExcPulse2AmplRel(void)
{
  DB_MSG(("-->ExcPulse2AmplRel"));
  UT_SetRequest("ExcPulse2Ampl");
  HandleRFPulse2Amplitude();
  DB_MSG(("-->ExcPulse2AmplRel"));
}

void HandleRFPulse2Amplitude(void)
{
  DB_MSG(("-->HandleRFPulse2Amplitude"));

  STB_UpdateRFShapeAmplitude("ExcPulse2Ampl",No);
  ATB_SetRFPulse("ExcPulse2","ACQ_RfShapes[1]");

  DB_MSG(("<--HandleRFPulse2Amplitude"));
}

void ExcPulse2Relation(void)
{
  DB_MSG(("-->ExcPulse2Relation"));
  
  /*
   * Tell the request handling system that the parameter
   * ExcPulse has been edited 
   */
  
  UT_SetRequest("ExcPulse2");
  
  /* Check the values of ExcPulse */
  
  ExcPulse2Range();
  
  /* 
   * call the backbone; further handling will take place there
   * (by means of STB_UpdateRFPulse)  
   */
  
  backbone();
  
  DB_MSG(("<--ExcPulse2Relation"));
}


void ExcPulse2Range(void)
{
  DB_MSG(("-->ExcPulse2Range"));
  
  // range checker fields to be controlled may be
  // .Length  
  // .Bandwidth
  // .Flipangle
  // .Calculated
  // .Sharpness
  // .Flatness   
 double dval=ExcPulse2.Flipangle;
  
 ExcPulse2.Flipangle = MIN_OF(90.0,MAX_OF(dval,1.0));

 DB_MSG(("<--ExcPulse2Range")); 
}
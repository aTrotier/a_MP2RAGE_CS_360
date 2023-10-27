/* ***************************************************************
 *
 * Copyright (c) 2022
 * Bruker BioSpin MRI GmbH
 * D-76275 Ettlingen, Germany
 *
 * All Rights Reserved
 *
 * ***************************************************************/



#define DEBUG	 1

#include "method.h"

/*===============================================================
 * InitCustom
 * 
 *===============================================================*/
void InitCustom()
{
  strncpy(ReleaseVersion, "v0.0.2", 9);
  ParxRelsParMakeNonEditable({"ReleaseVersion"});
  //ParxRelsShowInEditor("ReleaseVersion"); //ParxRelsHideInEditor

  MP2_NTI = 2;
  ParxRelsParMakeNonEditable({"MP2_NTI"}); 

  ParxRelsParChangeDims("EffectiveTI",{MP2_NTI});

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
 * MP2 Delay functions
 * 
 *===============================================================*/
void MP2_Delay()
{

  double IrInitDelay; // time of the IR module between half of Inv Pulse and end of module
    //see : /opt/PV6.0.1/exp/stan/nmr/lists/pp/SliceSelIr.mod
      IrInitDelay =
              (
                PVM_SelIrP0/1000.0/1000.0/2 + // IrPulse /2 
                PVM_SelIrD4                 + //grad_ramp
                PVM_SelIrD2                 + //IR spoiler : not used (except for repetitions)
                PVM_SelIrD4                 + //grad_ramp
                PVM_SelIrD1                   
              )*1000; //en ms
      
        DB_MSG(("-->IrInitDelay = %f ",IrInitDelay));

double delayBeforePulse;
delayBeforePulse = (
                    0.03 + //ms
                    0.01 +
                    SliceSpoiler.dur +
                    CFG_GradientRiseTime() +
                    ExcPulse1.Length - EffPulseDur
                   );

        DB_MSG(("-->delayBeforePulse = %f ",delayBeforePulse));

  double minTI[MP2_NTI];
  minTI[0] =  IrInitDelay + 
              delayBeforePulse +
              PVM_RepetitionTime * (int)((MP2_EchoTrainLength-1)/2);
  
        DB_MSG(("-->minTI[0] = %f ",minTI[0]));
        
  EffectiveTI[0]=MAX_OF(minTI[0],EffectiveTI[0]);
  MP2_VarIRDelay[0] = (EffectiveTI[0]-minTI[0])/1000.0;

  minTI[1]=EffectiveTI[0] + PVM_RepetitionTime*(int)(MP2_EchoTrainLength);
  EffectiveTI[1]=MAX_OF(minTI[1],EffectiveTI[1]);
  MP2_VarIRDelay[1] = (EffectiveTI[1]-minTI[1])/1000.0;

  double minRecovTime = EffectiveTI[1] +
                        PVM_RepetitionTime*(int)((MP2_EchoTrainLength)/2+1) +
                        + 0.02 //delay module inversion
                        + (PVM_SelIrD4)*1000.0
                        + PVM_SelIrP0/1000.0/2 ;

  MP2_RecoveryTime = MAX_OF(MP2_RecoveryTime,minRecovTime);
  MP2_VarIRDelay[2] = (MP2_RecoveryTime - minRecovTime) / 1000.0;
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
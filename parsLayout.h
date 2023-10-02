/****************************************************************
 *
 * Copyright (c) 2022
 * Bruker BioSpin MRI GmbH
 * D-76275 Ettlingen, Germany
 *
 * All Rights Reserved
 *
 ****************************************************************/

/****************************************************************/
/*	PARAMETER CLASSES				       	*/
/****************************************************************/



/*--------------------------------------------------------------*
 * Definition of the PV class...
 *--------------------------------------------------------------*/

pargroup
{
  CustomParameters;
  PVM_EffSWh;
  PVM_EchoPosition;
  EncGradDur;
  PVM_AcquisitionTime;
  ReadSpoiler;
  SliceSpoiler;
}
attributes
{
  display_name "Sequence Details";
} Sequence_Details;

pargroup
{
  ExcPulse1Enum;
  ExcPulse1;
  ExcPulse1Ampl;
}
attributes
{
  display_name "RF Pulses";
} RF_Pulses;

pargroup
{
  DummyScans_Parameters;

  PVM_NMovieFrames;
  TimeForMovieFrames;

  PVM_EvolutionOnOff;
  Evolution_Parameters;

  PVM_TriggerModule;
  Trigger_Parameters;

  PVM_TaggingOnOff;
  Tagging_Parameters;

  PVM_SelIrOnOff;
  Selective_IR_Parameters;

  PVM_BlBloodOnOff;
  BlackBlood_Parameters;

  PVM_FatSupOnOff;
  Fat_Sup_Parameters;

  PVM_SatTransOnOff;
  Sat_Transfer_Parameters;

  PVM_FovSatOnOff;
  Fov_Sat_Parameters;

  PVM_InFlowSatOnOff;
  Flow_Sat_Parameters;

  PVM_MotionSupOnOff;

  DriftComp_Parameters;

  RFSpoiling;
  AngioMode;

} Preparation;


extend pargroup
{
  PVM_EchoTime;
  PVM_RepetitionTime;
  PVM_NEchoImages;
  PVM_NAverages;
  PVM_NRepetitions;
  PVM_ScanTimeStr;
  PVM_ScanTime;
  PVM_DeriveGains;
  RF_Pulses;
  Nuclei;
  Encoding;
  Sequence_Details;
  ImageGeometry;
  Preparation;
  MapShim;
  MethodAtsGroup;
  StartupShims;
  GopAdj;
  IsGrappaAdj;  
} MethodClass;

// parameters that should be tested after any editing
conflicts
{
  PVM_EchoTime;
  PVM_RepetitionTime;
  PVM_Fov;
  PVM_SliceThick;
  ExcPulse1.Flipangle;
};

// parameters for reconstruction
extend pargroup
{
  RecoMethMode;
  WeightingMode;
  MaskWeighting;
  GaussBroadening;
  MethodAtsRecoGroup;
  ParentDset;
}attributes
{
  display_name "Reconstruction Options";
}MethodRecoGroup;

/****************************************************************/
/*	E N D   O F   F I L E					*/
/****************************************************************/

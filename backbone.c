/* ***************************************************************
 *
 * Copyright (c) 2022
 * Bruker BioSpin MRI GmbH
 * D-76275 Ettlingen, Germany
 *
 * All Rights Reserved
 *
 * ***************************************************************/

#define DEBUG	 0

#include "method.h"


void backbone(void)
{
  double minFov[3];
  double minSliceThick;

  DB_MSG(("-->backbone"));

  /* update nuclei parameter group                            */
  STB_UpdateNuclei(No);

  //constrain geometry
  STB_ConstrainSliceGeoForMb(PVM_MbEncNBands);
  
  /* update encoding parameter group                          */
  STB_UpdateEncoding(
    NULL,
    SEG_SEQUENTIAL,
    Yes,
    Yes,
    Yes,
    &PVM_EchoPosition,
    4,
    PVM_NMovieFrames); 

  /* update parameters controlling the data sampling period   */
  MRT_UpdateDigPars( &PVM_EffSWh,
                      PVM_EncMatrix[0],
                     &PVM_AntiAlias[0],
                     &PVM_AcquisitionTime );

  /* update excitation pulse                                  */
  UpdateRFPulses();

  /* general features of the method */
  PVM_NEchoImages = MP2_NTI;

  /* the inversion pulses can't handle multi-band */
  if(PVM_MbEncNBands > 1)
      PVM_SelIrOnOff = Off;
  
  /* coexistance of modules: */
  if(PVM_SelIrOnOff == On)
  {
    PVM_BlBloodOnOff = Off;
    PVM_NMovieFrames = 1;
  }

  /* Slice Selective Inversion does not work in AngioMode     */
  if (AngioMode==Yes)
  {
    ParxRelsParMakeNonEditable({"PVM_SelIrOnOff"});

    if(PVM_SelIrOnOff == On)
      PVM_SelIrOnOff = Off;
  }
  else
  {
    ParxRelsParMakeEditable({"PVM_SelIrOnOff"});
  }

  /* set limits for read, phase and slice gradients            */
  ControlGradientLimits(PVM_MajSliceOri);

  /* calculate minima for FOV and slice thickness              */
  UpdateGeometryMinima(minFov, &minSliceThick);

  /* update geometry parameters                                */
  int dim=PTB_GetSpatDim();

  if(dim < 2)
    AngioMode = No;

  /* only one package if black-blood module on                 */
  int maxPackages = PVM_BlBloodOnOff == On? 1:0;

  /* only one slice per package if 3D                          */
  int maxPerPackage = dim>2? 1:0;

  STB_UpdateImageGeometry(dim,
                          PVM_Matrix,
                          minFov,
                          0, // total slices (no restr)
                          maxPackages,
                          maxPerPackage,
                          minSliceThick,
                          1.0); // sliceFovRatio in 3D

  STB_UpdateMultiBandGeoPars("PVM_SliceGeoObj", 
        PVM_ObjOrderScheme,
        PVM_MbEncNBands);
  
  //Update RF Pulses, mb-parameters might have changed
  UpdateRFPulses();
  
  std::vector<double> spatres=STB_GetSpatResol();
  STB_AtsUpdate(&spatres);

  /* update slice spoiler                                      */
  double mindurSlice = 1.5*CFG_GradientRiseTime();
  double spoilerThick = dim>2 ? PVM_SpatResol[2]*PVM_EncZf[2] : PVM_SliceThick;
  MRT_UpdateSpoiler("SliceSpoiler",2.0,ExcSliceGradLim,mindurSlice,PVM_GradCalConst,spoilerThick);

  /* handling of modules                                       */
  STB_UpdateDriftCompModule(spoilerThick, PVM_DeriveGains);
  STB_UpdateFatSupModule(PVM_Nucleus1, PVM_DeriveGains, spoilerThick);
  STB_UpdateSatTransModule(PVM_DeriveGains);
  STB_UpdateFovSatModule(PVM_Nucleus1, PVM_DeriveGains);
  STB_UpdateFlowSaturationModule(PVM_Nucleus1,PVM_DeriveGains);
  STB_UpdateTriggerModule();
  STB_UpdateTaggingModule(dim,PVM_Fov,PVM_Matrix,PVM_SpatResol[0]*PVM_EncZf[0],PVM_DeriveGains);
  STB_UpdateEvolutionModule();

  /* Black Blood Module   */
  {
    GObject slicegeo("PVM_SliceGeo");

    double extents[3],pos[3];
    slicegeo.getAcqFov(extents,0);
    slicegeo.getPosToAcq(pos,0);

    /* Inversion time TI extends up to the centre of the excitation pulse!  *
     * Some delays of the imaging block and module durations have to be     * 
     * considered. Their sum is handed to the updater. Some modules may not *
     * make sense to be combined w inversion or even harm, but are taken    *
     * into acount anyway:                                                  */   
    double fixedTime = PVM_TaggingModuleTime
                       + 0.03                        /* UpdateDynPars */
                       + 0.01                        /* freqRx */
                       + PVM_InFlowSatModuleTime
                       + PVM_FovSatModuleTime
                       + PVM_SatTransModuleTime
                       + PVM_FatSupModuleTime
                       + SliceSpoiler.dur            /* d6   */
                       + CFG_GradientRiseTime()      /* d3   */
                       + ExcPulse1.Length/2.0;       /* p0/2 */

    /* for trigger active there's either per_Slice or per_PhaseStep: consider only one */
    fixedTime += PVM_TriggerModuleTime;

    STB_UpdateBlBloodModule( &extents[2], &pos[2], 1, fixedTime,
                             PVM_SliceThick, PVM_DeriveGains);
  }

  UpdateMovie();
  UpdateRFSpoiling();

  /* Calculate read and slice gradients                       */
  UpdateReadSliceGradients();

  /* Sequence elements, TE, TR, total time:                   */
  UpdateSequenceTiming();
  
  PTB_ClearAdjustments();

  PTB_AppendOrderAdjustment(per_scan, per_scan, RG_ADJNAME);
  
  if(PVM_EncPpiRefScan)
  {
    PTB_AppendAdjustment("Grappa",
                         "Adjust GRAPPA Coeff.",
                         "Adjustment of GRAPPA Coefficients",
                         per_scan);
    if(PVM_MbEncAccelFactor > 1){
        PTB_AppendAdjustment("MbRg",
                        "Receiver Gain MB", 
                        "Re-Adjustment of the Receiver Gain for Multi-Band",
                        per_scan);
    }
  }  

  /* Set correct DriftCompUpdateTime after TR-upate           */
  if (AngioMode)
    STB_UpdateDriftComp( PVM_RepetitionTime * (PVM_EncGenTotalSteps/PVM_NMovieFrames/PVM_EncGenLoopOuter + PVM_DummyScans) +
                         0.01                               +
                         PVM_DriftCompModuleTime            +
                         PVM_BlBloodModuleTime              +
                         PVM_TriggerModuleTime              +
                         PVM_SelIrModuleTime                +
                         PVM_TaggingModuleTime              );
  else
    STB_UpdateDriftComp( PVM_RepetitionTime );

  /* Dummy Scans                                              */
  STB_UpdateDummyScans(PVM_RepetitionTime);

  /* calculate dephasing and phase-encoding gradients         */
  UpdatePhaseGradients();

  /* calculate frequency offsets                              */
  UpdateFrequencyOffsets();

  /* update mapshim parameter group */
  STB_UpdateMapShim(PVM_Nucleus1,"PVM_SliceGeoObj");

  /* update startup shim parameter group                      */
  STB_UpdateStartupShims();

  /* Set ACQP parameters                                      */
  SetBaseLevelParam();

  /* Set RECO parameters                                      */
  SetRecoParam();

  DB_MSG(("<--backbone"));
  return;
}

/*-------------------------------------------------------
 * local utility routines to simplify the backbone
 *------------------------------------------------------*/


void UpdateSequenceTiming()
/* -------------------------------------------------------
   Adapt sequence elements to the current geometry
   (in this method, only EncGradDur is concerned),
   update TE and TR.
   ReadGrad and ExcSliceGrad must be already set.
   ------------------------------------------------------*/
{
  double minEnc2d, minEnc3d, minRephSlice, minDephRead,
         extensionPossible, extensionAllowed, extension;
  int nSlices;

  /* Part of the exctiation pulse to be refocused: */
  EffPulseDur = ExcPulse1.Length * (ExcPulse1.Rpfac/100);

  /* Part of the echo to be refocused */
  EchoDelay = PVM_AcquisitionTime * PVM_EchoPosition / 100;

  /* Minimum durations of all phase-gradient pulses */

  minEnc2d     = PTB_GetSpatDim()>1?
                 MRT_EncodingTime(PVM_SpatResol[1],
                                  PVM_GradCalConst*Phase2DGradLim/100)
                 : 0;
  
  if(PTB_GetSpatDim()>2){
    minEnc3d = MRT_EncodingTime(PVM_SpatResol[2],
                    PVM_GradCalConst*Phase3DGradLim/100);
  }else if (PTB_GetSpatDim() == 2 && PVM_MbEncNBands > 1){
    minEnc3d = MRT_EncodingTime(PVM_MbSliceDist*(PVM_MbEncNBands-1)/(4.0*PVM_MbEncBlipFactor),
                    PVM_GradCalConst*Phase3DGradLim/100);
  }else
    minEnc3d = 0;

  minRephSlice = MRT_DephaseTime(EffPulseDur,
                                 CFG_GradientRiseTime(),
                                 ExcSliceGrad,
                                 ExcSliceRephGradLim);

  minDephRead  = MRT_DephaseTime(EchoDelay,
                                 CFG_GradientRiseTime(),
                                 ReadGrad,
                                 ReadDephGradLim);


  /* In this sequence all phase-gradient pulses are
     simultaneous with duration EncGradDur. We set it first
     to the common (longest) minimum: */

  EncGradDur = MAX_OF( MAX_OF(minEnc2d ,  minEnc3d),
                       MAX_OF(minRephSlice ,minDephRead ) );

  /* EncGradDur should also contain one ramp, thus: */

  EncGradDur = RewGradDur = MAX_OF(EncGradDur, CFG_GradientRiseTime());

  DB_MSG(("minimum EncGradDur = %f", EncGradDur));

  /* Update TE with the mimimum EncGradDur */

  UpdateEchoTime();


  /* If there is some freedom, make EncGradDur longer
     (to avoid unnecessarily strong phase gradients),
     but not longer than allowed by gradient resolution */

  extensionPossible = PVM_EchoTime - PVM_MinEchoTime;
  extensionAllowed  = PTB_GetSpatDim()>1 ? MRT_MaxEncodingTime(PVM_Fov[1], PVM_GradCalConst)-EncGradDur :20;

  DB_MSG(("ext possible = %f, allowed = %f", extensionPossible, extensionAllowed));

  extension = MIN_OF(extensionPossible, extensionAllowed);
  extension = MAX_OF(extension, 0);

  DB_MSG(("extension = %f",extension));

  EncGradDur += extension;
  TeFillDelay = PVM_EchoTime - PVM_MinEchoTime - extension;

  /* Other sequence elements, not involved in TE, e.g. spoilers  */
  double mindurRead =  RewGradDur+CFG_GradientRiseTime();
  MRT_UpdateSpoiler( "ReadSpoiler", 2.0, Phase2DGradLim, mindurRead,
                     PVM_GradCalConst, PVM_SpatResol[0]*PVM_EncZf[0] );

  /* Update module SelIr */
  MinSliceSegDur = minLoopRepetitionTime();
  /* SliceSegDur may be changed by the UpdateSelIrModule! */
  SliceSegDur = MinSliceSegDur;
  nSlices = GTB_NumberOfSlices( PVM_NSPacks, PVM_SPackArrNSlices);

  /* Inversion time TI extends up to the centre of the excitation pulse!         *
   * some delays of the imaging block and module durations have to be considered.*
   * Their sum is handed to the updater. Some modules may not make sense to be   *
   * combined w inversion or even harm but are taken into acount anyway:         */
  double fixedTime = PVM_TaggingModuleTime
                     + 0.03                       /*UpdateDynPars*/
                     + 0.01                       /* freqRx */
                     + PVM_InFlowSatModuleTime
                     + PVM_FovSatModuleTime
                     + PVM_SatTransModuleTime
                     + PVM_FatSupModuleTime
                     + SliceSpoiler.dur           /* d6   */
                     + CFG_GradientRiseTime()     /* d3   */
                     + ExcPulse1.Length/2.0;      /* p0/2 */

  /* consider trigger event within slice-loop only if mode is per_Slice: *
     (no SelIr in angio-mode!)                                           */
  if (PVM_TriggerMode == per_Slice)
    fixedTime += PVM_TriggerModuleTime;

  STB_UpdateSelIrModule( PVM_SliceThick, PVM_EffSliceOffset, nSlices, &SliceSegDur,
                         0, fixedTime, PVM_DeriveGains );

  if(PVM_SelIrOnOff==On)
    ParxRelsParCopy("PVM_InversionTime","PVM_SelIrInvTime");

  if(PVM_BlBloodOnOff==On)
    ParxRelsParCopy("PVM_InversionTime","PVM_BlBloodInvTime");

  if(PVM_BlBloodOnOff==Off&&PVM_SelIrOnOff==Off)
  {
    PVM_InversionTime = 0.0;
    ParxRelsParHideInEditor({"PVM_InversionTime"});
  }

  /* Find min TR and update TR */
  UpdateRepetitionTime();

  /* Calculate total experiment time */
  UpdateTotalTime();
}


void UpdateRepetitionTime(void)
{
  int nSlices;

  DB_MSG(("-->UpdateRepetitionTime"));

  nSlices = GTB_NumberOfSlices( PVM_NSPacks, PVM_SPackArrNSlices )/PVM_MbEncNBands;

  if(AngioMode == No)
  {
    if(PVM_SelIrOnOff == On)
    {
      PVM_MinRepetitionTime = PVM_DriftCompModuleTime
                              + PVM_SelIrModuleTime
                              + PVM_TaggingModuleTime
                              + nSlices * SliceSegDur ;
    }
    else
    {
      /* min TR in a movie: */
      PVM_MinRepetitionTime = nSlices * minLoopRepetitionTime();

      /* if there is no movie, TR also includes some modules: */
      if(PVM_NMovieFrames == 1)
      {
        PVM_MinRepetitionTime +=  PVM_DriftCompModuleTime
                                + PVM_BlBloodModuleTime
                                + PVM_SelIrModuleTime
                                + PVM_TaggingModuleTime ;
      }
    }

    if (PVM_NMovieFrames == 1 && PVM_TriggerMode == per_PhaseStep)
    {
      PVM_MinRepetitionTime += PVM_TriggerModuleTime;

      //Additional trigger_per_phase event if BlBlood is active
      if (PVM_BlBloodOnOff)
        PVM_MinRepetitionTime += PVM_TriggerModuleTime;
    }
  }
  else
    PVM_MinRepetitionTime = minLoopRepetitionTime();

  PVM_RepetitionTime=MAX_OF( PVM_MinRepetitionTime, PVM_RepetitionTime );

  /* Adapt RewGradDur if TR allows, but only if Slice Sel Inversion is not in use, */
  /* because otherwise SliceSegDur is changed & delay betw inv pulses become wrong  */
  if (PVM_SelIrOnOff == Off)
  {
    double riseT = CFG_GradientRiseTime();
    double extensionAllowed = EncGradDur - RewGradDur;
    double extensionPossible = ReadSpoiler.dur-riseT - RewGradDur;
    double extension = MAX_OF(MIN_OF(extensionPossible, extensionAllowed),0);

    /* 1.) if possible, extend RewGradDur to duration of read spoiler */
    if(extension > 0)
    {
      RewGradDur += extension;
      extensionAllowed = EncGradDur - RewGradDur;
    }

    int slices = AngioMode==Yes? 1:nSlices;

    extensionPossible = (PVM_RepetitionTime - PVM_MinRepetitionTime)/slices;
    extension = MAX_OF(MIN_OF(extensionPossible, extensionAllowed),0);

    /* 2.) if possible, extend RewGradDur to EncGradDur  */
    if(extension > 0)
    {
      RewGradDur += extension;
      MRT_UpdateSpoiler( "ReadSpoiler", 2.0, Phase2DGradLim, RewGradDur+riseT,
                          PVM_GradCalConst, PVM_SpatResol[0] * PVM_EncZf[0] );

      PVM_MinRepetitionTime += extension*slices;
    }
  }

  DB_MSG(("<--UpdateRepetitionTime"));
  return;
}


/* calculates PVM_ScanTimeStr and TimeForMovieFrames */

void UpdateTotalTime(void)
{
  DB_MSG(("-->UpdateTotalTime"));

  int nSlices      = GTB_NumberOfSlices( PVM_NSPacks, PVM_SPackArrNSlices );
  int dim          = PTB_GetSpatDim();
  double TotalTime = 0;

  if( dim >1 )
  {
    if(PVM_NMovieFrames > 1)
    {
      /* TR is one movie frame, without prep modules: */
      TimeForMovieFrames = PVM_RepetitionTime * PVM_NMovieFrames + PVM_TaggingModuleTime;
      TotalTime = ( PVM_BlBloodModuleTime + TimeForMovieFrames ) *
                    PVM_EncGenTotalSteps/PVM_NMovieFrames * PVM_NAverages;
    }
    else
    {
      if(AngioMode==No)
      {
        /* TR includes prep modules */
        TimeForMovieFrames = PVM_RepetitionTime - PVM_BlBloodModuleTime - PVM_TaggingModuleTime;
        TotalTime = PVM_RepetitionTime * PVM_EncGenTotalSteps/PVM_NMovieFrames * PVM_NAverages;
      }
      else
      {
        TotalTime = PVM_RepetitionTime * PVM_EncGenTotalSteps/PVM_NMovieFrames/PVM_EncGenLoopOuter
                    + PVM_BlBloodModuleTime
                    + PVM_TaggingModuleTime
                    + PVM_DriftCompModuleTime;

       if (PVM_TriggerMode == per_Slice)
          TotalTime += (PVM_BlBloodOnOff == On) ? 2*PVM_TriggerModuleTime : PVM_TriggerModuleTime;

        TotalTime*= nSlices*PVM_NAverages;
        if( dim >2 )
          TotalTime *= PVM_EncGenLoopOuter;        
      }
    }
  }

  /* time for one repetition */
  OneRepTime = TotalTime/1000.0;

  TotalTime = TotalTime * PVM_EvolutionCycles + PVM_EvolutionModuleTime;
  TotalTime = TotalTime * PVM_SatTransRepetitions * PVM_NRepetitions;

  PVM_ScanTime = TotalTime;
  UT_ScanTimeStr(PVM_ScanTimeStr,TotalTime);
  ParxRelsParShowInEditor({"PVM_ScanTimeStr"});
  ParxRelsParMakeNonEditable({"PVM_ScanTimeStr"});

  DB_MSG(("<--UpdateTotalTime"));
}


void UpdateGeometryMinima(double *minFov, double *minSliceThick)
{
  int dim;

  DB_MSG(("-->UpdateGeometryMinima"));

  dim=PTB_GetSpatDim();

  minFov[0]      = PVM_EffSWh /
                   (1e-2*ReadGradLim * PVM_GradCalConst);
 
  *minSliceThick = ExcPulse1.Bandwidth /
                   (1e-2*ExcSliceGradLim * PVM_GradCalConst);

  if(dim >= 2)
    minFov[1] = minFov[0]/8;

  if(dim >= 3)
    minFov[2] = *minSliceThick;

  DB_MSG(("<--UpdateGeometryMinima"));
}


void UpdateReadSliceGradients(void)
{
  DB_MSG(("-->UpdateReadSliceGradients"));

  ReadGrad = MRT_ReadGrad(PVM_EffSWh,
                          PVM_Fov[0],
                          PVM_GradCalConst);

  ExcSliceGrad = MRT_SliceGrad(ExcPulse1.Bandwidth,
                               PVM_SliceThick,
                               PVM_GradCalConst);

  DB_MSG(("<--UpdateReadSliceGradients"));
}

void UpdatePhaseGradients()
{
  DB_MSG(("-->UpdatePhaseGradients"));

  /* Calculation of phase-encoding, dephasing and rephasing gradients.

     (ReadGrad, ExcSliceGrad, EchoDelay, EffPulseDur, and EncGradDur 
      must be calculated before)       */

  double rise  = CFG_GradientRiseTime();

  ReadDephGrad =     MRT_DephaseGrad(EncGradDur, EchoDelay,   rise, ReadGrad);

  ExcSliceRephGrad = MRT_DephaseGrad(EncGradDur, EffPulseDur, rise, ExcSliceGrad);

  Phase2DGrad = PTB_GetSpatDim() > 1 ?
                MRT_PhaseGrad(EncGradDur, PVM_Matrix[1], PVM_Fov[1], PVM_GradCalConst) : 0.0;

  if(PTB_GetSpatDim() == 3){
    Phase3DGrad = MRT_PhaseGrad(EncGradDur, PVM_Matrix[2], PVM_Fov[2], PVM_GradCalConst);
  }else if(PTB_GetSpatDim() == 2 && PVM_MbEncNBands > 1){
    Phase3DGrad = MRT_PhaseGrad(EncGradDur, 1, PVM_MbSliceDist, PVM_GradCalConst);
  }else
    Phase3DGrad = 0.0;

  Rew2DGrad = PTB_GetSpatDim() > 1 ?
                MRT_PhaseGrad(RewGradDur, PVM_Matrix[1], PVM_Fov[1], PVM_GradCalConst) : 0.0;

  if(PTB_GetSpatDim() == 3){
    Rew3DGrad = MRT_PhaseGrad(RewGradDur, PVM_Matrix[2], PVM_Fov[2], PVM_GradCalConst);
  }else if(PTB_GetSpatDim() == 2 && PVM_MbEncNBands > 1){
    Rew3DGrad = MRT_PhaseGrad(RewGradDur, 1, PVM_MbSliceDist, PVM_GradCalConst);  
  }else
    Rew3DGrad = 0.0;

  DB_MSG(("<--UpdatePhaseGradients"));
  return;
}

void UpdateFrequencyOffsets( void )
{
 DB_MSG(("-->UpdateFrequencyOffsets"));
  int spatDim;
  int i,nslices;

  spatDim = PTB_GetSpatDim();
  nslices = GTB_NumberOfSlices(PVM_NSPacks,PVM_SPackArrNSlices);

  MRT_FrequencyOffsetList(nslices,
                          PVM_EffReadOffset,
                          ReadGrad,
                          PVM_GradCalConst,
                          PVM_ReadOffsetHz );

  MRT_FrequencyOffsetList(nslices,
                          PVM_EffSliceOffset,
                          ExcSliceGrad,
                          PVM_GradCalConst,
                          PVM_SliceOffsetHz );
  
  if(PVM_MbEncNBands > 1){
    nslices /= PVM_MbEncNBands;
    MRT_FrequencyOffsetList(nslices,
                            PVM_MbEffReadOffset,
                            ReadGrad,
                            PVM_GradCalConst,
                            PVM_MbReadOffsetHz );

    MRT_FrequencyOffsetList(nslices,
                            PVM_MbEffSliceOffset,
                            ExcSliceGrad,
                            PVM_GradCalConst,
                            PVM_MbSliceOffsetHz );
  } 
  
  if(spatDim == 3)
  {
    for(i=0;i<nslices;i++)
      PVM_EffPhase2Offset[i] = -PVM_EffSliceOffset[i];
  }

 DB_MSG(("<--UpdateFrequencyOffsets"));
}


/*--------------------------------------------------------
 * Routine to update RF pulse parameters
 *-------------------------------------------------------*/

void UpdateRFPulses(void)
{

  /* Updates all parameters that belong to ExcPulse1 pulse structure
     (as initialized by STB_InitRFPulse see initMeth.c)
  */
  DB_MSG(("-->UpdateRFPulses"));

  STB_UpdateRFPulse("ExcPulse1",1,PVM_DeriveGains,Conventional,"PVM_MbEncNBands,PVM_MbRelSliceDist");

  if(PVM_DeriveGains==Yes)
  {
    ParxRelsParHideInEditor({"ExcPulse1Ampl"});
  }
  else
  {
    ParxRelsParShowInEditor({"ExcPulse1Ampl"});
  }

  ParxRelsParShowInFile({"ExcPulse1Ampl"});

  DB_MSG(("<--UpdateRFPulses"));

  return;
}

/*--------------------------------------------------------
 * Routine to control the visibility of parameters
 *-------------------------------------------------------*/


void ControlGradientLimits(YesNo NotOblique)
{
  DB_MSG(("-->ControlGradientLimits: Obliqueness forbidden: %s",NotOblique==Yes ? "Yes":"No"));


  if(NotOblique==Yes)
  {
    ReadDephGradLim     =
    Phase2DGradLim      =
    Phase3DGradLim      =
    ExcSliceRephGradLim = 100.0;
  }
  else
  {
    /* Gradient limits in % of max. Value 57 (1/sqrt(3))
       is needed for arbitrary oblique slices. */
    ReadDephGradLim     =
    Phase2DGradLim      =
    Phase3DGradLim      =
    ExcSliceRephGradLim = 57.0;
  }

  ReadGradLim        = 100.0;
  ExcSliceGradLim    = 100.0;

  DB_MSG(("-->ControlGradientLimits"));
}

void UpdateMovie(void)
{
  DB_MSG(("-->UpdateMovie"));

  if(AngioMode==Yes)
    PVM_NMovieFrames=1;

  if(PVM_NMovieFrames == 1)
  {
    ParxRelsParHideInEditor({"TimeForMovieFrames"});
  }
  else
  {
    ParxRelsParShowInEditor({"TimeForMovieFrames"});
  }

  DB_MSG(("<--UpdateMovie"));
}




/* Calculates PVM_MinEchoTime and restrict PVM_EchoTIme.
   EffPulseDur, EncGradDur must be set before */
void UpdateEchoTime( void )
{

  double riseTime = CFG_GradientRiseTime(),
    rampTime = CFG_GradientRampTime()+CFG_InterGradientWaitTime();

  DB_MSG(("-->UpdateEchoTime\n"));

  PVM_MinEchoTime =
    EffPulseDur    +
    rampTime       + //falling ramp of slice grad
    EncGradDur     + //enc. time (ramp+plateau
    rampTime       + //min te-filling (end ramp of encoding)
    riseTime       + //read-on ramp          +
    EchoDelay;


  PVM_EchoTime = MAX_OF(PVM_EchoTime, PVM_MinEchoTime);

  DB_MSG(("<--echoTimeRels\n"));
}



double minLoopRepetitionTime(void)
/* ---------------------------------------------------------
this function returns the minimum duration of the innermost
pulse program loop
----------------------------------------------------------*/
{
  double minTr, minD0,
    riseTime = CFG_GradientRiseTime(),
    igwt = CFG_InterGradientWaitTime();

  minD0 = 0.01  /* ADC_END */ + igwt;

  minTr = 0.030                          + /* UpdateDynPars */
          0.010                          + /* freqRx */
          PVM_InFlowSatModuleTime        +
          PVM_FovSatModuleTime           +
          PVM_SatTransModuleTime         +
          PVM_FatSupModuleTime           +
          SliceSpoiler.dur               + /* d6 */
          riseTime                       + /* d3 */
          ExcPulse1.Length - EffPulseDur +
          PVM_EchoTime - EchoDelay       +
          PVM_AcquisitionTime            + /* AQ_job(0) */
          ReadSpoiler.dur                + /* d11 + d12 */
          riseTime                       + /* d3  */
          minD0;                           /* d0  */

  if ( !AngioMode && PVM_TriggerMode == per_Slice )
    minTr += PVM_TriggerModuleTime;

  /* angio-mode swaps slice & phase-encoding order */
  if ( AngioMode && PVM_TriggerMode == per_PhaseStep )
    minTr += PVM_TriggerModuleTime;

  return minTr;
}

void UpdateRFSpoiling(void)
{
  DB_MSG(("-->UpdateRFSpoiling"));

  if(RFSpoiling==No)
  {
    ParxRelsParChangeDims("RFPhaseList",{1});
    RFPhaseList[0] = 0;
  }
  else
  {
    int max = PVM_EncGenTotalSteps/PVM_NMovieFrames+PVM_DummyScans;
    int size = MAX_OF(256,max);
    ParxRelsParChangeDims("RFPhaseList",{size});
    MRT_RFSpoilPhaseList(117,size,RFPhaseList,Yes);
  }

  DB_MSG(("<--UpdateRFSpoiling"));
}

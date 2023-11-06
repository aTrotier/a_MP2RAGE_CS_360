/****************************************************************
 *
 * Copyright (c) 2018 - 2021
 * Bruker BioSpin MRI GmbH
 * D-76275 Ettlingen, Germany
 *
 * All Rights Reserved
 *
 ****************************************************************/

#define DEBUG       0

#include "method.h"


void SetBaseLevelParam( void )
{

  DB_MSG(("-->SetBaseLevelParam\n"));

  SetBasicParameters();

  SetFrequencyParameters();

  SetPpgParameters();

  SetAcquisitionParameters();

  SetGradientParameters();

  SetInfoParameters();

  ATB_SetReceiverGains();

  /* setting baselevel parameters used by modules */
  ATB_SetFatSupBaselevel();
  ATB_SetSatTransBaseLevel();
  ATB_SetFovSatBaseLevel();
  ATB_SetFlowSaturationBaseLevel();
  ATB_SetTaggingBaseLevel();
  ATB_SetSelIrBaseLevel(GTB_NumberOfSlices( PVM_NSPacks, PVM_SPackArrNSlices ));
  ATB_SetBlBloodBaseLevel();

    printTiming();

  DB_MSG(("<--SetBaseLevelParam\n"));
}

void SetBasicParameters( void )
{
  int nSlices;

  DB_MSG(("-->SetBasicParameters\n"));

  /* NSLICES */
  nSlices = GTB_NumberOfSlices( PVM_NSPacks, PVM_SPackArrNSlices );
  ATB_SetNSlices( nSlices );

  /* NR */
  ATB_SetNR( PVM_SatTransRepetitions * PVM_EvolutionCycles * PVM_NRepetitions );

  /* NI */
  ATB_SetNI( nSlices/PVM_MbEncNBands * PVM_NEchoImages *  PVM_NMovieFrames);

  /* AVERAGING */
  if (!PVM_MotionSupOnOff)
  {
    ATB_SetNA( PVM_NAverages );
    ATB_SetNAE( 1 );
  }
  else
  {
    ATB_SetNAE( PVM_NAverages );
    ATB_SetNA( 1 );
  }

   /* ACQ_ns */

  ACQ_ns_list_size = PVM_NEchoImages;
  ParxRelsParRelations("ACQ_ns_list_size",Yes);

  for(int echo=0; echo<PVM_NEchoImages; echo++)
    ACQ_ns_list[echo] = 1;

  NS = ACQ_ns_list[0];
  
  /* NECHOES */
  NECHOES = PVM_NEchoImages;

  /* ACQ_obj_order */
  ParxRelsParChangeDims("ACQ_obj_order",{NI});

  if( PVM_NMovieFrames > 1)
    SetACQ_obj_orderForMovie();
  else if(PVM_MbEncNBands > 1){
    ATB_SetAcqObjOrder( nSlices/PVM_MbEncNBands, PVM_MbObjOrderList, PVM_NEchoImages, 1 );
  }else{
    ATB_SetAcqObjOrder( nSlices, PVM_ObjOrderList, PVM_NEchoImages, 1 );
  }
  DB_MSG(("<--SetBasicParameters\n"));
}

void SetFrequencyParameters( void )
{
  int nslices;

  DB_MSG(("-->SetFrequencyParameters\n"));

  ATB_SetNuc1(PVM_Nucleus1);

  sprintf(NUC2,"off");
  sprintf(NUC3,"off");
  sprintf(NUC4,"off");
  sprintf(NUC5,"off");
  sprintf(NUC6,"off");
  sprintf(NUC7,"off");
  sprintf(NUC8,"off");

  ATB_SetNucleus(PVM_Nucleus1);

  ACQ_O1_mode = BF_plus_Offset_list;
  ParxRelsParRelations("ACQ_O1_mode",Yes);

  O1 = 0.0;
  O2 = 0.0;
  O3 = 0.0;
  O4 = 0.0;
  O5 = 0.0;
  O6 = 0.0;
  O7 = 0.0;
  O8 = 0.0;

  /* Set BF's to working freuncies on used channels */
  ACQ_BF_enable = No;
  BF1 = PVM_FrqWork[0];
  BF2 = PVM_FrqWork[1];
  /* call relations of BF1 (no need for other BF's) */
  ParxRelsParRelations("BF1", Yes);


  nslices = GTB_NumberOfSlices( PVM_NSPacks, PVM_SPackArrNSlices );

  if(PVM_MbEncNBands > 1){
    nslices /= PVM_MbEncNBands;
    ATB_SetAcqO1List( nslices,
                      PVM_MbObjOrderList,
                      PVM_MbSliceOffsetHz );

    ATB_SetAcqO1BList( nslices,
                       PVM_MbObjOrderList,
                       PVM_MbReadOffsetHz);
  }else{    
    ATB_SetAcqO1List( nslices,
                    PVM_ObjOrderList,
                    PVM_SliceOffsetHz );


    ATB_SetAcqO1BList( nslices,
                     PVM_ObjOrderList,
                     PVM_ReadOffsetHz);
  }
  
  DB_MSG(("<--SetFrequencyParameters\n"));
}

void SetGradientParameters( void )
{
  DB_MSG(("-->SetGradientParameters\n"));

  int spatDim = PTB_GetSpatDim();

  ParxRelsParChangeDims("ACQ_phase_encoding_mode", {spatDim} );
  ParxRelsParChangeDims("ACQ_phase_enc_start", {spatDim} );
  switch(spatDim)
  {
    case 3:
      ACQ_phase_encoding_mode[2] = User_Defined_Encoding;
      ACQ_phase_enc_start[2] = -1; /* set, but no used */
      [[fallthrough]];
      /* no break */
    case 2:
      ACQ_phase_encoding_mode[1] = User_Defined_Encoding;;
      ACQ_phase_enc_start[1] = -1.0; /* set, but no used */
      ACQ_spatial_size_1 = PVM_EncGenTotalSteps;
      ACQ_spatial_size_2 = PVM_EncGenTotalSteps;
      ParxRelsParCopy("ACQ_spatial_phase_1","PVM_EncGenValues1");
      ParxRelsParCopy("ACQ_spatial_phase_2","PVM_EncGenValues2");
      [[fallthrough]];
      /* no break */
    default:
      ACQ_phase_encoding_mode[0] = Read;
      ACQ_phase_enc_start[0] = -1;
  }
  ParxRelsParRelations("ACQ_phase_encoding_mode",Yes);

  // new function to set up gradient matrix
  if(PVM_MbEncNBands > 1)
    ATB_SetAcqGradMatrix("PVM_MbSliceGeoObj");
  else 
    ATB_SetAcqGradMatrix("PVM_SliceGeoObj");

  ACQ_scaling_read  = 1.0;
  ACQ_scaling_phase = 1.0;
  ACQ_scaling_slice = 1.0;

  /* gradient amplitudes */
  ACQ_gradient_amplitude[0] =  ExcSliceGrad;
  ACQ_gradient_amplitude[1] = -ExcSliceRephGrad;
  ACQ_gradient_amplitude[2] = -ReadDephGrad;
  ACQ_gradient_amplitude[3] =  Phase2DGrad;
  ACQ_gradient_amplitude[4] = -Phase3DGrad;
  ACQ_gradient_amplitude[5] =  ReadGrad;
  ACQ_gradient_amplitude[6] =  ReadSpoiler.ampl;
  ACQ_gradient_amplitude[7] = -Rew2DGrad;
  ACQ_gradient_amplitude[8] =  Rew3DGrad;
  ACQ_gradient_amplitude[9] =  SliceSpoiler.ampl;


  DB_MSG(("<--SetGradientParameters\n"));
}

void SetInfoParameters( void )
{
  int slices, i, spatDim;

  DB_MSG(("-->SetInfoParameters\n"));

  // initialize ACQ_n_echo_images ACQ_echo_descr
  //            ACQ_n_movie_frames ACQ_movie_descr
  ATB_ResetEchoDescr();
  ATB_ResetMovieDescr();

  spatDim = PTB_GetSpatDim();

  /* ACQ_dim */
  ACQ_dim = spatDim;
  ParxRelsParRelations("ACQ_dim",Yes);

  /* ACQ_dim_desc */
  ATB_SetAcqDimDesc( 0, spatDim, NULL );

  ATB_SetAcqFov( Spatial, spatDim, PVM_Fov, PVM_AntiAlias );

  ACQ_flip_angle = ExcPulse1.Flipangle;

 ParxRelsParChangeDims( "ACQ_echo_time", {PVM_NEchoImages} );
  for(i=0; i<PVM_NEchoImages; i++)
    ACQ_echo_time[i] = PVM_EchoTime;

  ParxRelsParChangeDims("ACQ_inter_echo_time",{1});
  ACQ_inter_echo_time[0] = PVM_EchoTime;

  ParxRelsParChangeDims("ACQ_repetition_time",{1});
  ACQ_repetition_time[0] = PVM_RepetitionTime;

  ParxRelsParChangeDims("ACQ_recov_time",{1});
  ACQ_recov_time[0] =  PVM_RepetitionTime - ExcPulse1.Length;

  /* calculation of ACQ_time_points */
  ATB_EvolutionSetTimePoints(PVM_NRepetitions, OneRepTime);

  ParxRelsParChangeDims("ACQ_inversion_time",{1});
  ACQ_inversion_time[0] = PVM_InversionTime;

  ATB_SetAcqSliceAngle( PtrType3x3 PVM_SPackArrGradOrient[0],
                        PVM_NSPacks );

  ACQ_slice_orient = Arbitrary_Oblique;

  ACQ_slice_thick = PVM_SliceThick;

  slices = GTB_NumberOfSlices( PVM_NSPacks, PVM_SPackArrNSlices );

  ParxRelsParChangeDims("ACQ_slice_offset",{slices});
  ParxRelsParChangeDims("ACQ_read_offset",{slices});
  ParxRelsParChangeDims("ACQ_phase1_offset",{slices});
  ParxRelsParChangeDims("ACQ_phase2_offset",{slices});

  for(i=0;i<slices;i++)
  {
    ACQ_slice_offset[i]  = PVM_SliceOffset[i];
    ACQ_read_offset[i]   = PVM_ReadOffset[i];
    ACQ_phase1_offset[i] = PVM_Phase1Offset[i];
    ACQ_phase2_offset[i] = PVM_Phase2Offset[i];
  }


  ACQ_read_ext = (int)PVM_AntiAlias[0];

  ParxRelsParChangeDims("ACQ_slice_sepn", {slices==1 ? 1 : slices-1});

  if( slices == 1 )
  {
    ACQ_slice_sepn[0] = 0.0;
  }
  else
  {
    for( i=1; i<slices;i++ )
    {
      ACQ_slice_sepn[i-1]=PVM_SliceOffset[i]-PVM_SliceOffset[i-1];
    }
  }

  ATB_SetAcqSliceSepn( PVM_SPackArrSliceDistance,
                       PVM_NSPacks );

  ACQ_n_t1_points = 1;

  if( ParxRelsParHasValue("ACQ_transmitter_coil") == No )
  {
    ACQ_transmitter_coil[0] = '\0';
  }

  if( ParxRelsParHasValue("ACQ_contrast_agent") == No )
  {
    ACQ_contrast_agent[0] = '\0';
  }

  if( ParxRelsParHasValue("ACQ_contrast") == No )
  {
    ACQ_contrast.volume = 0.0;
    ACQ_contrast.dose = 0.0;
    ACQ_contrast.route[0] = '\0';
    ACQ_contrast.start_time[0] = '\0';
    ACQ_contrast.stop_time[0] = '\0';
  }

  ParxRelsParRelations("ACQ_contrast_agent",Yes);

  ACQ_position_X = 0.0;
  ACQ_position_Y = 0.0;
  ACQ_position_Z = 0.0;

  ParxRelsParChangeDims("ACQ_temporal_delay",{1});
  ACQ_temporal_delay[0] = 0.0;

  ACQ_RF_power = 0;

  if(PVM_NMovieFrames > 1)
  {
    ACQ_n_echo_images = PVM_NMovieFrames;
    ParxRelsParChangeDims("ACQ_echo_descr",{PVM_NMovieFrames,20});
    for(i=0; i<PVM_NMovieFrames; i++)
      sprintf(ACQ_echo_descr[i], "Movie frame %d", i+1);
  }

  DB_MSG(("<--SetInfoParameters\n"));

}

void SetPpgParameters( void )
{
  DB_MSG(("-->SetPpgParameters\n"));

  if(AngioMode==Yes)
    ATB_SetPulprog("a_MP2RAGE_CS_360Angio.ppg");
  else
    ATB_SetPulprog("a_MP2RAGE_CS_360.ppg");

  double igwt = CFG_InterGradientWaitTime();
  double riseTime = CFG_GradientRiseTime();
  double rampTime = CFG_GradientRampTime() + igwt;

  int slices = AngioMode==Yes? 1:NSLICES/PVM_MbEncNBands;

  if((PVM_SelIrOnOff==On) && (AngioMode==No))
  {
    D[0]  = (SliceSegDur - MinSliceSegDur + igwt + 0.01) / 1000.0;
    D[20] = (PVM_RepetitionTime - PVM_MinRepetitionTime)/1000.0;
  }
  else
  {
    D[0]  = ((PVM_RepetitionTime - PVM_MinRepetitionTime)/slices + igwt + 0.01) / 1000.0;
    D[20] = 0.0;
  }
  D[2]  = (TeFillDelay + rampTime) / 1000.0;
  D[3]  = riseTime / 1000.0;
  D[4]  = rampTime / 1000.0;
  D[6]  = SliceSpoiler.dur/1000.0;
  D[10] = EncGradDur / 1000.0;
  D[11] = RewGradDur / 1000.0;
  D[12] = (ReadSpoiler.dur - RewGradDur)/1000.0;

  /* set shaped pulses, in this method ACQ_RfShapes[0] is used
     the pulse duration is stored in baselevel parameter P[0]
  */
  ATB_SetRFPulse("ExcPulse1","ACQ_RfShapes[0]","P[0]");

  if(AngioMode==Yes){
    L[1]  = PVM_EncGenTotalSteps/PVM_EncGenLoopOuter;
    L[2]  = PVM_EncGenLoopOuter;
  }else{
    L[1]  = PVM_EncGenTotalSteps/PVM_NMovieFrames;
  }
  L[3]  = PVM_DummyScans;
  L[10] = PVM_NMovieFrames;

  ParxRelsParRelations("L",Yes);

  DB_MSG(("<--SetPpgParameters\n"));
}


/*-------------------------------------------------------*/
/*            Image sorting for movie mode               */
/*-------------------------------------------------------*/
void SetACQ_obj_orderForMovie(void)
{
  int k,j,i;
  int nSlices;
  DB_MSG(("-->SetACQ_obj_orderForMovie\n"));

  j=0;
  nSlices = GTB_NumberOfSlices( PVM_NSPacks, PVM_SPackArrNSlices )/PVM_MbEncNBands;
  while(j< PVM_NMovieFrames)
  {
    for(i=0;i<nSlices;i++)
    {
      k=j*nSlices+i;
      if(PVM_MbEncNBands > 1)
        ACQ_obj_order[k]= PVM_MbObjOrderList[i]*PVM_NMovieFrames +j;
      else
        ACQ_obj_order[k]= PVM_ObjOrderList[i]*PVM_NMovieFrames +j;
    }
    j=j+1;
  }

  DB_MSG(("<--SetACQ_obj_orderForMovie\n"));
}


void SetAcquisitionParameters(void)
{
  DB_MSG(("-->SetAcquisitionParameters\n"));

  /* ------------------------------------------------ */
  /* -------               Job 0              ------- */
  /* ------------------------------------------------ */
  JOBPARAMETERS(jobParameters);
  JOBDESCRIPTION job0=jobParameters->getOrCreateJob("job0");

  job0->initPars(1, PVM_EncMatrix[0], PVM_EffSWh * PVM_AntiAlias[0]);

  if (AngioMode == No)
  {
    
      job0->appendLoop(PVM_RareFactor);
    job0->appendLoop(PVM_NEchoImages);
    job0->appendLoop(NSLICES);
    job0->appendLoop(PVM_DummyScans, LOOP_DUMMIES);
    job0->appendLoop(NA, LOOP_AVERAGE, LOOP_SETUP);
    job0->appendLoop(PVM_EncGenTotalSteps/PVM_RareFactor);
    job0->appendLoop(NAE, LOOP_AVERAGE);
    job0->appendLoop(NR);
    /*
    job0->appendLoop(NI);
    job0->appendLoop(PVM_DummyScans, LOOP_DUMMIES);
    job0->appendLoop(NA, LOOP_AVERAGE, LOOP_SETUP);
    job0->appendLoop(PVM_EncGenTotalSteps/PVM_NMovieFrames);
    */
  }
  else
  {
    job0->appendLoop(PVM_EncGenTotalSteps/PVM_EncGenLoopOuter);
    job0->appendLoop(NI, LOOP_SETUP);
    job0->appendLoop(NA, LOOP_AVERAGE);
    job0->appendLoop(PVM_EncGenLoopOuter);
  }

  job0->appendLoop(NAE, LOOP_AVERAGE);
  job0->appendLoop(NR);

  /* ------------------------------------------------ */
  /* -------         DriftCompensation        ------- */
  /* ------------------------------------------------ */
  int nTotalScansJob1 = NR * NAE * NA;

  if(AngioMode==No)
  {
    nTotalScansJob1 *= PVM_EncGenTotalSteps;
  }
  else
    nTotalScansJob1 *= NI;

  if(AngioMode==No)
    ATB_SetDriftCompBaseLevel(jobParameters, NA, nTotalScansJob1, PVM_DummyScans);
  else
    ATB_SetDriftCompBaseLevel(jobParameters, NA, nTotalScansJob1, 0);

  DB_MSG(("<--SetAcquisitionParameters\n"));
}


void printTiming(void)
{
#if DEBUG
#define d(n) (D[n]*1e3)
#define p(n) (P[n]*1e-3)

  DB_MSG(("-->printTiming"));

  double te, tr, aqq = PVM_AcquisitionTime;

  /* TE */
  te = p(0)/2 + d(4) + d(10) + d(2) + d(3) + aqq/2;

  DB_MSG(("   TE: %f, should be %f, diff %f", te, PVM_EchoTime,
          te-PVM_EchoTime));

  /* TR :
     For non angio-mode this is from start label to 'loop to start time NA'
     For angio mode this is from aquisition label to 'loop to acquisitin time l1' 
  */

  if(AngioMode==No)
  {
    tr = 0.03 /* UPDATE_DYN_PARS */ + 0.01 /* freqRx */ + d(6) + d(3) + p(0)/2 
        + te + aqq/2 + d(11) + d(12) + d(3) + d(0);

    if (!PVM_BlBloodOnOff || !PVM_SelIrOnOff)
    {
      tr += PVM_InFlowSatModuleTime
           + PVM_FovSatModuleTime
           + PVM_SatTransModuleTime
           + PVM_FatSupModuleTime;

      if ( !AngioMode && PVM_TriggerMode == per_Slice )
        tr += PVM_TriggerModuleTime;
    }

    tr *= NSLICES/PVM_MbEncNBands;

    if (!PVM_BlBloodOnOff || !PVM_SelIrOnOff)
      tr += PVM_BlBloodModuleTime 
           + PVM_SelIrModuleTime 
           + PVM_TaggingModuleTime;

    tr += PVM_DriftCompModuleTime + d(20);

    if (PVM_NMovieFrames==1 && PVM_TriggerMode == per_PhaseStep)
      tr += (PVM_BlBloodOnOff==On)? 2*PVM_TriggerModuleTime : PVM_TriggerModuleTime;
  }
  else
  {
    tr = 0.03 /* UPDATE_DYN_PARS */ + 0.01 /* acquisition-label*/ + d(6) + d(3) 
        + d(8) + p(0)/2  + te + aqq/2 + d(11) + d(12) + d(3) + d(0);

    if (!PVM_BlBloodOnOff) //no sel ir in angio  
            //those modules are part of blackblood's fixed time
      tr += PVM_InFlowSatModuleTime 
        + PVM_FovSatModuleTime
        + PVM_SatTransModuleTime
        + PVM_FatSupModuleTime;

    if (PVM_TriggerMode == per_PhaseStep)
      tr += PVM_TriggerModuleTime;
  }

  DB_MSG(("   TR: %f, should be %f, diff %f", tr, PVM_RepetitionTime, 
          tr-PVM_RepetitionTime));

  DB_MSG(("<--printTiming"));

#undef d
#undef p
#endif
}

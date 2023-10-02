/* ***************************************************************
 *
 * Copyright (c) 2006 - 2021
 * Bruker BioSpin MRI GmbH
 * D-76275 Ettlingen, Germany
 *
 * All Rights Reserved
 *
 * ***************************************************************/

#define DEBUG		0

#include "method.h"
#include "common/ovl_toolbox/AdjTools.h"


/* -------------------------------------------------------------------
  Relations of PVM_AdjHandler, called when an adjustment is starting.
  Parameter changes made her will be used for the adjustment and then
  disarded (the current scan will not be affected).
  -------------------------------------------------------------------*/
void HandleAdjustmentRequests(void)
{
  YesNo adjPossible=No;


  DB_MSG(("-->HandleAdjustmentRequests"));

  if (Yes==STB_AtsAdj()) {
    return;
  }

  if(No == PTB_AdjustmentStarting())
  {
    DB_MSG(("<--HandleAdjustmentRequests: method should not be rebuilt in adjustment platform"));
    return;
  }

 

  DB_MSG(("RG sub-adjustment sequence: %s", adjSequence));

  std::string adjname(PTB_GetAdjName());
  std::string subadjname(PTB_GetAdjSubProcess());

  PVM_MbEncNBands = 1;
  
  if(Yes==PTB_AdjMethSpec())
  {
    if(adjname==RG_ADJNAME || subadjname== RG_ADJNAME)
    {
      DB_MSG(("setting up RG adjustment"));
      
      PVM_MbEncAccelFactor = 1;
      backbone();
      ParxRelsParRelations("PVM_AdjHandler",Yes);
      adjPossible=Yes;
    }
    else if(adjname =="Grappa"|| subadjname=="Grappa")
    {
      /* Set up a non-accelerated scan to adjust
         the GRAPPA parameters */
      
      DB_MSG(("setting up Grappa adjustment"));
      
      AdjEnableGlobalResultSave=No;
      AdjEnableUserResultSave=No;
      PvAdjManRequestNewExpno(GopAdj);    
      IsGrappaAdj = GopAdj;
      ParentDset = PvAdjGetOriginalDatasetPath();
      ACQ_ScanOnlineReco = Yes; 
      
      static const std::vector<AdjResultPar> grappaResultPar =
        { 
          PvAdjManCreateResultPar(adj_type, "PVM_GrappaAdjScan"),
          PvAdjManCreateResultPar(adj_type, "RecoGrappaCoefficients"),
          PvAdjManCreateResultPar(adj_type, "RecoSliceGrappaCoefficients")
        };
      PvAdjManSetAdjParameterList(grappaResultPar);   
      
      double storedZf[3] = {1.,1.,1.};
      int virtualAccel[3] = {1,1,1};
      YesNo storedEncGenPpi = PVM_EncGenPpi;
      YesNo storedEncGenCaipi = PVM_EncGenCaipirinha;
      
      PVM_AntiAlias[1] = 1;
      PVM_EncPft[1] = 1;
      storedZf[1] = PVM_EncZf[1];
      PVM_EncZf[1] = std::max(1.0,PVM_Matrix[1]/double(PVM_EncPpiGopRefLines[1]));
      virtualAccel[1] = PVM_EncPpi[1];
      PVM_EncPpi[1] = 1;      
      if(PTB_GetSpatDim()>2){
        PVM_AntiAlias[2] = 1;
        PVM_EncPft[2] = 1;
        storedZf[2] = PVM_EncZf[2];
        PVM_EncZf[2] = std::max(1.0,PVM_Matrix[2]/double(PVM_EncPpiGopRefLines[2]));
        virtualAccel[2] = PVM_EncPpi[2];
        PVM_EncPpi[2] = 1;
      }
      
      PVM_NRepetitions = 1;
      PVM_NMovieFrames = 1;
      PVM_NAverages = 1;
      
      adjPossible = Yes;
      backbone();
      
      /* copy back acceleration for reconstruction */
      PVM_EncPpi[1] = virtualAccel[1];
      if(PTB_GetSpatDim()>2){
        PVM_EncPpi[2] = virtualAccel[2];
      }
      PVM_EncGenPpi = storedEncGenPpi;
      PVM_EncGenCaipirinha = storedEncGenCaipi;
      
      /* derive reco, with acceleration */
      SetRecoParam();
      
      /* copy back zero-filling for later reconstructions */
      PVM_EncZf[1] = storedZf[1];   
      if(PTB_GetSpatDim()>2){
        PVM_EncZf[2] = storedZf[2];
      }
      
    }
    else if(adjname=="MbRg"|| subadjname=="MbRg")
    {
      PVM_MbEncNBands = PVM_MbEncAccelFactor;
      PVM_MbEncAccelFactor = 1;
      strcpy(ACQ_SetupAutoName,"PVM_AutoRgCounter");
      ParxRelsParRelations("PVM_AdjHandler",Yes);
      adjPossible=Yes;    
    } 
  }// no method specific adjustment
  else
  {
    ParxRelsParRelations("PVM_AdjHandler",Yes);
    return;
  } 
   

  if(adjPossible == No)
  {
    PARX_sprintf("Unknown adjustment required");
    /* make illegal adjustment request fail by setting ACQ_SetupAutoName to empty string */
    ACQ_SetupAutoName[0]='\0';
  }

  DB_MSG(("<--HandleAdjustmentRequests"));
  return;
}


/*
 *  This function is called each time an adjustment is finished. Changes made here
 *  will stay for the active scan. This routine is performed in the parameter space
 *  of the active scan and NOT in the parameter space of the adjustment.
 */

void HandleAdjustmentResults(void)
{
  DB_MSG(("-->HandleAdjustmentResults"));

  const char * adjName = PTB_GetAdjResultName();
  const char * subAdjName = PTB_GetAdjResultSubProcess();

  if(!strcmp(adjName, "Grappa") || !strcmp(subAdjName, "Grappa"))
  {
    DB_MSG(("-->HandleAdjustmentResults::Grappa"));
    std::vector<std::string> grouplist = MRT_RecoGetPIParameterList();
    ParentDset = PvAdjGetOriginalDatasetPath();
    char currentScanpath[PATH_MAX];
    memset(currentScanpath,0,PATH_MAX*sizeof(char));
    FormatDataPathname(currentScanpath, 4095, IsExpnoPath, ParentDset, 
        "Grappadset");
    /** Write a list of parameters or groups to a file. */
    ParxRelsWriteParList(grouplist,
                         currentScanpath,
                         Parx::Utils::WriteMode::NormalMode);
    DB_MSG(("<--HandleAdjustmentResults::Grappa"));
    
  }

  DB_MSG(("Calling backbone"));
  backbone();
  DB_MSG(("<--HandleAdjustmentResults"));

}

void HandleGopAdjResults(void)
{

  std::string adjname(PTB_GetAdjName());
  std::string subadjname(PTB_GetAdjSubProcess());
  std::ostringstream oerrst;

  ParxRelsParRelations("PVM_GopAdjResultHandler", OVERLAY_ONLY);
  
  if(adjname == "Grappa" || subadjname == "Grappa" )
  {
    DB_MSG(("-->HandleGopAdjResults"));
    bool overflowDetected = false;
    YesNo *overflow=ACQ_adc_overflow;
    int dim=(int)ParxRelsParGetDim("ACQ_adc_overflow",1);
    for(int i=0;i<dim;i++)
    {
      if(overflow[i]==Yes)
      {
        overflowDetected=true;
        oerrst << "overflow on channel " 
               << i+1 << std::endl;
      }
    }
    if(overflowDetected)
      throw PvStdException("Grappa Adjustment failed:\n%s",
                           oerrst.str().c_str());
    PTB_RegisterGopAdj();
    AdjExecutionState adjstate = adj_successful;
    ParxRelsParSetValue("AdjPerformState",&adjstate);

    DB_MSG(("<--HandleGopAdjResults"));

  }


 
}


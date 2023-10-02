/****************************************************************
 *
 * $Source$
 *
 * Copyright (c) 1999-2002
 * Bruker BioSpin MRI GmbH
 * D-76275 Ettlingen, Germany
 *
 * All Rights Reserved
 *
 *
 * $Id$
 *
 ****************************************************************/

/* react on changes of system configuration                  */
relations PVM_SysConfigHandler backbone;

/* Encoding */
relations PVM_EncodingHandler backbone;

/* geometry */
relations PVM_ImageGeometryHandler  backbone;

/* bandwidth */
relations PVM_EffSWh           EffSWhRel;

/* modules */
relations PVM_FatSupHandler     backbone;
relations PVM_SatTransHandler   backbone;
relations PVM_FovSatHandler     backbone;
relations PVM_FlowSatHandler    backbone;
relations PVM_TriggerHandler    backbone;
relations PVM_TaggingHandler    backbone;
relations PVM_EvolutionHandler  backbone;
relations PVM_SelIrHandler      backbone;
relations PVM_BlBloodHandler    backbone;
relations PVM_DummyScansHandler backbone;
relations PVM_DriftCompHandler  backbone;

/* other parameters */
relations PVM_NucleiHandler     backbone;
relations PVM_DeriveGains       backbone;
relations PVM_RepetitionTime    backbone;
relations PVM_EchoTime          backbone;
relations PVM_NAverages         NAveragesRels;
relations PVM_MotionSupOnOff    backbone;
relations PVM_NMovieFrames      backbone;
relations PVM_NRepetitions      backbone;
relations PVM_InversionTime     InversionTimeRels;
relations PVM_AutoRgInitHandler MyRgInitRel;


/* Redirect relation for visu creation */
relations VisuDerivePars        deriveVisu;

/* redirection of method specific reconstruction method */
relations RecoUserUpdate        RecoDerive;

/* relations for mapshim parameter group*/
relations PVM_MapShimHandler backbone;

/* relations for ATS handling */
relations PVM_AtsHandler backbone;
relations PVM_AtsRefPosHandler backbone;

/* Adjustment related redirections */
relations PVM_AdjResultHandler   HandleAdjustmentResults;
relations PVM_AdjHandler         HandleAdjustmentRequests;
relations PVM_GopAdjResultHandler HandleGopAdjResults;

/****************************************************************/
/*      E N D   O F   F I L E                                   */
/****************************************************************/

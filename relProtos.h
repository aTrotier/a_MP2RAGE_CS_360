/****************************************************************
 *
 * Copyright (c) 2013 - 2020
 * Bruker BioSpin MRI GmbH
 * D-76275 Ettlingen, Germany
 *
 * All Rights Reserved
 *
 ****************************************************************/

#ifndef METHRELS_H
#define METHRELS_H

/* gen/src/prg/methods/FLASH/initMeth.c */
void initMeth(void);
/* gen/src/prg/methods/FLASH/loadMeth.c */
void loadMeth(const char *);
/* gen/src/prg/methods/FLASH/parsRelations.c */
void ExcPulse1EnumRelation(void);
void ExcPulse1AmplRel(void);
void HandleRFPulseAmplitude(void);
void ExcPulse1Relation(void);
void ExcPulse1Range(void);
void NAveragesRange(void);
void NAveragesRels(void);
void EffSWhRange(void);
void EffSWhRel(void);
void InversionTimeRels(void);
void InversionTimeRange(void);
void ReadSpoilerRel(void);
void SliceSpoilerRel(void);
void RecoMethModeVisPar(void);
void MaskModeRel(void);
void GaussBroadRange(void);
void MaskWeightRange(void);
void MyRgInitRel(void);
/* gen/src/prg/methods/FLASH/BaseLevelRelations.c */
void SetBaseLevelParam(void);
void SetBasicParameters(void);
void SetFrequencyParameters(void);
void SetGradientParameters(void);
void SetInfoParameters(void);
void SetPpgParameters(void);
void SetACQ_obj_orderForMovie(void);
void SetAcquisitionParameters(void);
void printTiming(void);
/* gen/src/prg/methods/FLASH/RecoRelations.c */
void SetRecoParam(void);
void RecoDerive(void);
void SWIRecoRel(void);
/* gen/src/prg/methods/FLASH/backbone.c */
void backbone(void);
void UpdateSequenceTiming(void);
void UpdateRepetitionTime(void);
void UpdateTotalTime(void);
void UpdateGeometryMinima(double *, double *);
void UpdateReadSliceGradients(void);
void UpdatePhaseGradients(void);
void UpdateFrequencyOffsets(void);
void UpdateRFPulses(void);
void ControlGradientLimits(YesNo);
void UpdateMovie(void);
void UpdateEchoTime(void);
double minLoopRepetitionTime(void);
void UpdateRFSpoiling(void);
/* gen/src/prg/methods/FLASH/deriveVisu.c */
void deriveVisu(void);
/* gen/src/prg/methods/FLASH/adjust.c */
void HandleAdjustmentRequests(void);
void HandleAdjustmentResults(void);
void HandleGopAdjResults(void);
#endif

/****************************************************************/
/*      E N D   O F   F I L E                                   */
/****************************************************************/


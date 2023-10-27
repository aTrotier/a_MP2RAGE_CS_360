char parameter ReleaseVersion[9];

/****** MP2RAGE ********/
double parameter MP2_VarIRDelay[3]; // 3 delay between IR/ETL1/ETL2/MP2RAGE_Recovery

int parameter MP2_NTI; // Number of INVERSION TIME POINTS

double parameter
{
  display_name "Recovery time";
  format "%.2f";
  units "ms";
  relations backbone;
}MP2_RecoveryTime;

int parameter
{
  display_name "Echo Train Length";
  relations backbone;
}MP2_EchoTrainLength;

double parameter
{
  display_name "Effective Inversion Time";
  format "%.2f";
  units "ms";
  relations backbone;
} EffectiveTI[];

/**** Pulse ****/
PV_PULSE_LIST parameter
{
  display_name "Excitation Pulse Shape";
  relations    ExcPulse2EnumRelation;
}ExcPulse2Enum;


PVM_RF_PULSE parameter
{
  display_name "Excitation Pulse 2";
  relations    ExcPulse2Relation;
}ExcPulse2;

PVM_RF_PULSE_AMP_TYPE parameter
{
  display_name "RF Pulse Amplitude";
  relations ExcPulse2AmplRel;
}ExcPulse2Ampl;

double parameter
{
  relations backbone;
}ExcPulse2Shape[];


/****************************************************************/
pargroup
{
    ReleaseVersion;
    MP2_RecoveryTime;
    MP2_EchoTrainLength;
    EffectiveTI;
    ExcPulse2Enum;
    ExcPulse2;
    ExcPulse2Ampl;
}
attributes
{
  display_name "Custom Parameters";

} CustomParameters;

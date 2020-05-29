#ifndef ORIGA_NVM_H
#define ORIGA_NVM_H

//#include "../Origa_type.h"
#include "Origa_type.h"
#include "Origa_Swi.h"



/* ****************************************************************************
   *** v0.90; 2008-11-10: 1st version 
   *** v0.93; 2009-03-31: Nvm_GetAdcOffset removed.
   ************************************************************************* */

BOOL Nvm_ProgramData( BOOL b_WaitForFinish, BOOL b_VerifyData, UWORD uw_Address, UBYTE ub_BytesToProgram, UBYTE * ubp_Data );
BOOL Nvm_ReadData( UWORD uw_Address, UBYTE ub_BytesToRead, UBYTE * ubp_Data );

BOOL Nvm_DecreaseLifeSpanCounter( void );
BOOL Nvm_VerifyLifeSpanCounter( BOOL * bp_IsValid );
BOOL Nvm_ReadLifeSpanCounter( ULONG * ulp_LifeSpanCounter);



#endif

#include "F2806x_Device.h"
#include "Stepper-Settings.h"

#if (Origa_Ena)


#include "Origa_Nvm.h"

extern ULONG g_culNvmTimeout;


/* ****************************************************************************
   name:      Nvm_ProgramData()
  
   function:  program desired data values into ORIGA NVM address space.
              the option b_VerifyData can be used to re-read the programmed
              data to be sure that the data was written correct.
  
   input:     IN: b_WaitForFinish
                if 'true', then the host waits for the NVM state-mashine to
                finish before returning from that function.
              IN: b_VerifyData
                if 'true', then the written data is cross-checked with the
                content of the write buffer.
              IN: uw_Address
                start address within NVM to program data beginning from.
              IN: ub_BytesToProgram
                number of bytes to program.
              IN: * ubp_Data
                pointer to buffer holding values to program into NVM.
   output:    bool
  
   return:    'true', if write access was executed without any problems.
              'false', if failures happened
  
   date:			2010-03-04: time out added.
   ************************************************************************* */
BOOL Nvm_ProgramData( BOOL b_WaitForFinish, BOOL b_VerifyData, UWORD uw_Address, UBYTE ub_BytesToProgram, UBYTE * ubp_Data )
{

  BOOL bResult;
  UBYTE ubData;
  UWORD uwDataCount;
  
  UWORD uwAddress;
  UBYTE ubAddressH;
  UBYTE ubAddressL;

	ULONG ulNvmTimeOut;


  /* check not allowed settings */
  if( (b_WaitForFinish == FALSE) && (b_VerifyData == TRUE) )
  {
    return FALSE;
  }     

  /* remove NVM offset */
  uw_Address &= 0x00FFu;

  for( uwDataCount = 0u; uwDataCount < ub_BytesToProgram; uwDataCount++ )
  {

    uwAddress = uw_Address + uwDataCount;
    ubAddressH = (UBYTE)((uwAddress >> 3u) & 0x1Fu);
    ubAddressH |= 0xC0u;
    ubAddressL = (UBYTE)(uwAddress & 0x07u);

   /* wait for NVM state-machine to be ready for a new command */
   ulNvmTimeOut = g_culNvmTimeout;
   do
    {
      if( Swi_ReadRegisterSpace( SWI_ORIGA_CTRL2_NVM, &ubData ) == FALSE )
      {
        return FALSE;
      }
      /* check for timeout */
      if( ulNvmTimeOut == 0u )
      {							  
       return FALSE; 
	    }
      ulNvmTimeOut--;
    }while( (ubData & 0x80u) != 0u );

    /* functions return always true, if mask is set to 0xFF */
    (void)Swi_WriteRegisterSpaceNoIrq( SWI_NVM_WIP0 | ubAddressL, ubp_Data[uwDataCount], 0xFFu );
    (void)Swi_WriteRegisterSpaceNoIrq( SWI_ORIGA_NVM_ADDR, ubAddressL, 0xFFu );
    (void)Swi_WriteRegisterSpaceNoIrq( SWI_ORIGA_CTRL2_NVM, ubAddressH, 0xFFu );
  }

  /* if user wants to wait until write is done, then poll as long as programming requires */
  if( b_WaitForFinish == TRUE )
  {
		ulNvmTimeOut = g_culNvmTimeout;
    do
    {
      if( Swi_ReadRegisterSpace( SWI_ORIGA_CTRL2_NVM, &ubData ) == FALSE )
      {
        return FALSE;
      }
			/* check for timeout */
			if( ulNvmTimeOut == 0u )
			{
        return FALSE;
			}
			ulNvmTimeOut--;
    }while( (ubData & 0x80u) != 0u );
  }


  if( b_VerifyData == TRUE )
  {
    for( uwDataCount = 0u; uwDataCount < ub_BytesToProgram; uwDataCount++ )
    {
     bResult = Nvm_ReadData( uw_Address + uwDataCount, 1u, &ubData );
     if( (bResult == FALSE) || (ubData != ubp_Data[uwDataCount]) )
     {
       return FALSE;
     }
    }
  }

  /* all ok */
  return TRUE;

}



/* ****************************************************************************
   name:      Nvm_ReadData()
  
   function:  read data from requested NVM address and store data into
              provided buffer.
  
   input:     IN: uw_Address
                start address to read data from NVM.
              IN: ub_BytesToRead
                number of bytes to read from NVM.
              OUT: * ubp_Data 
                pointer to buffer to store read data into.
   output:    bool
  
   return:    'true', if reading was ok  .
              'false', if reading failed.
  
   date:			2010-03-04: time out added.
   ************************************************************************* */
BOOL Nvm_ReadData( UWORD uw_Address, UBYTE ub_BytesToRead, UBYTE * ubp_Data )
{

  UWORD uwDataCount;
  UBYTE ubData;

  UWORD uwAddress;
  UBYTE ubAddressH;
  UBYTE ubAddressL;

	ULONG ulNvmTimeOut;


  /* remove NVM offset */
  uw_Address &= 0x00FFu;

  for( uwDataCount = 0u; uwDataCount < ub_BytesToRead; uwDataCount++ )
  {
    uwAddress = uw_Address + uwDataCount;
    ubAddressH = (UBYTE)((uwAddress >> 3u) & 0x1Fu);
    ubAddressH |= 0x80u;
    ubAddressL = (UBYTE)(uwAddress & 0x07u);

    ulNvmTimeOut = g_culNvmTimeout;
    do
    {
      if( Swi_ReadRegisterSpace( SWI_ORIGA_CTRL2_NVM, &ubData ) == FALSE )
      {
        return FALSE;
      }
			/* check for timeout */
			if( ulNvmTimeOut == 0u )
			{
        return FALSE;
			}
			ulNvmTimeOut--;
    }while( (ubData & 0x80u) != 0u );


	  /* functions return always true, if mask is set to 0xFF */
    (void)Swi_WriteRegisterSpaceNoIrq( SWI_ORIGA_NVM_ADDR, ubAddressL, 0xFFu );
    (void)Swi_WriteRegisterSpaceNoIrq( SWI_ORIGA_CTRL2_NVM, ubAddressH, 0xFFu );

    ulNvmTimeOut = g_culNvmTimeout;
    do
    {
      if( Swi_ReadRegisterSpace(SWI_ORIGA_CTRL2_NVM, &ubData) == FALSE )
      {
        return FALSE;
      }
			/* check for timeout */
			if( ulNvmTimeOut == 0u )
			{
        return FALSE;
			}
			ulNvmTimeOut--;
    }while( (ubData & 0x80u) != 0u );

    if( Swi_ReadRegisterSpace( SWI_NVM_WIP2 | ubAddressL, &ubData ) == FALSE )
    {
      return FALSE;
    }
    ubp_Data[uwDataCount] = ubData;
  }

  /* all ok */
  return TRUE;
  
}



/* ****************************************************************************
   name:      Nvm_DecreaseLifeSpanCounter()
  
   function:  try to decrease LifeSpanCounter by one.
  
   input:     -
   output:    bool
  
   return:    'true', if decrease was triggered.
              'false', if decrease failed.
  
   date:			2010-03-04: time out added.
   ************************************************************************* */
BOOL Nvm_DecreaseLifeSpanCounter( void )
{

  UBYTE ubData;
 
	ULONG ulNvmTimeOut;

  
  /* wait for NVM state-machine to be ready for a new command */
  ulNvmTimeOut = g_culNvmTimeout;
  do
  {
    if( Swi_ReadRegisterSpace( SWI_ORIGA_CTRL2_NVM, &ubData ) == FALSE )
    {
      return FALSE;
    }
    /* check for timeout */
    if( ulNvmTimeOut == 0u )
    {
      return FALSE;
	  }
    ulNvmTimeOut--;
  }while( (ubData & 0x80u) != 0u );
  
  /* function returns always true, if mask is set to 0xFF */
  (void)Swi_WriteRegisterSpaceNoIrq( SWI_ORIGA_NVM_ADDR, 0x20u, 0xFFu );

  
  return TRUE;

}



/* ****************************************************************************
   name:      Nvm_VerifyLifeSpanCounter()
  
   function:  check that the LifeSpanCounter is within a valid nuber range
  
   input:     OUT: * bp_IsValid
                pointer to bool to store verification state of the 
                LifeSpanCounter into.
   output:    bool
  
   return:    'true', if read access was ok.
              'false', if read access failed.
  
   date:      .
   ************************************************************************* */
BOOL Nvm_VerifyLifeSpanCounter( BOOL * bp_IsValid )
{

  ULONG ulLifeSpanCount;
  
  
  /* for case of fail, the LSP also will set to false */
  *bp_IsValid = FALSE;

  if( Nvm_ReadLifeSpanCounter( &ulLifeSpanCount ) == TRUE )
  {
    if( ulLifeSpanCount <= 100000u )
    {
      *bp_IsValid = TRUE;
    }
    return TRUE;
  }
  else
  {
    return FALSE;
  }
  
}



/* ****************************************************************************
   name:      Nvm_ReadLifeSpanCounter()
  
   function:  read current setting of the LifeSpanCounter.
  
   input:     OUT: * ulp_LifeSpanCounter
                pointer to ULONG to store current LifeSpanCounter state
                into.
   output:    bool
  
   return:    'true', if LifeSpanCounter reading was ok.
              'false', if LifeSpanCounter reading failed.
  
   date:      v0.93; 2009-05-25: size optimized
   ************************************************************************* */
BOOL Nvm_ReadLifeSpanCounter( ULONG * ulp_LifeSpanCounter )
{

  UBYTE ubCount;
  UBYTE ubData[ 4 ];
  ULONG ulResult = 0UL;  
  

  /* read data from NVM */
  if( Nvm_ReadData( 0x148u, 4u, ubData ) == FALSE )
  {
    return FALSE;
  }

  /* create result */
  ubCount = 4;
  do
  {
  	ubCount--;	/* NOTE: prevent MISRA-C rule 12.13 violation. */
    ulResult = (ulResult << 8u) | ubData[ubCount];
  }while( ubCount != 0u );
  *ulp_LifeSpanCounter =  ulResult;
  

  /* all done well */
  return TRUE;

}


#endif

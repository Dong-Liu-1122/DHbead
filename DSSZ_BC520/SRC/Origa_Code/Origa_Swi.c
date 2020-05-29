#include "F2806x_Device.h"
#include "Stepper-Settings.h"

#if (Origa_Ena)

#include "Origa_Swi.h"
#include "board.h"

/* *** global definitions *** */
extern ULONG g_ulBaudLow;				//1 * BIF_DEFAULT_TIMEBASE
extern ULONG g_ulBaudHigh;				//3 * BIF_DEFAULT_TIMEBASE
extern ULONG g_ulBaudStop;				//5 * BIF_DEFAULT_TIMEBASE
extern ULONG g_ulResponseTimeOut;		//20 * BIF_DEFAULT_TIMEBASE
extern ULONG g_ulBaudPowerUpPulse;
extern ULONG g_ulBaudPowerDownTime;
extern ULONG g_ulBaudPowerUpTime;
extern ULONG g_ulBaudResetTime;
extern ULONG g_ulIntTimeoutDefault;
extern ULONG g_ulImmediateIntTimeOut;

// Used in multiple slave uid search
UBYTE ub_Stack[96];
UBYTE ub_StackPointer = 0; 

 
/* ****************************************************************************
   name:      Swi_ReadActualSpace()
  
   function:  read one byte from actual addressed space.
  
   input:     OUT: * ubp_Data
                pointer to UBYTE to store read value into.
   output:    bool
  
   return:    true, if read was ok.
              false, if read errors detected.
  
   date:      2011-07-22; v1.03: function added.
   ************************************************************************* */
BOOL Swi_ReadActualSpace( UBYTE * ubp_Data )
{

  BOOL bResult;
  U_SWI_WORD uReceivedWord;

  
  /* read out data */
  bResult = Swi_ReceiveRawWord( &uReceivedWord );
  if( bResult == TRUE )
  {
    *ubp_Data = (UBYTE)(uReceivedWord.sSwiAbstact.SWI_DATA);
  }
  
  /* show result */
  return bResult;

}



/* ****************************************************************************
   name:      Swi_ReadRegisterSpace()
  
   function:  read one byte from the ORIGA register space.
  
   input:     IN: uw_Address
                address within register space to read byte from.
              OUT: * ubp_Data
                pointer to UBYTE to store read value into.
   output:    bool
  
   return:    true, if read was ok.
              false, if read errors detected.
  
   date:      .
   ************************************************************************* */
BOOL Swi_ReadRegisterSpace( UWORD uw_Address, UBYTE * ubp_Data )
{

  BOOL bResult;
  U_SWI_WORD uReceivedWord;


  /* set burst length to one byte */
  Swi_SendRawWordNoIrq(SWI_BC, SWI_RBL0 );
  /* select register set */
  Swi_SendRawWordNoIrq(SWI_BC, SWI_EREG );

  /* send address to read from */
  Swi_SendRawWordNoIrq(SWI_ERA, ((uw_Address >> 8u) & 0xFFu) );
  Swi_SendRawWordNoIrq(SWI_RRA, ( uw_Address        & 0xFFu) );

  /* read out data */
  bResult = Swi_ReceiveRawWord( &uReceivedWord );

  if( bResult == TRUE )
  {
    *ubp_Data = (UBYTE)(uReceivedWord.sSwiAbstact.SWI_DATA);
  }
  
  /* show result */
  return bResult;

}



/* ****************************************************************************
   name:      Swi_WriteRegisterSpace()
  
   function:  write new content to ORIGA register address space.
              the function can also handle IRQ events that may be triggered
              after doing a register write access.
  
   input:     IN: uw_Address
                address within ORIGA register space to change the content.
              IN: ub_Data
                new data to store at the address pointed to by uw_Address
              IN: ub_BitSelect
                AND-Mask for doing selective change of register content.
                only bits set to '1' at ub_BitSelect are changed.
                NOTE: ub_BitSelect == 0xFF will alter all bits
              IN: b_WaitForInterrupt
                wait until SWI IRQ is detected.
              IN: b_ImmediateInterrupt
                wait until SWI IRQ is detected directly after command is sent.
              OUT: * bp_IrqDetected
                pointer to bool to save detection of SWI IRQ.
                NOTE: id b_WaitForInterrupt == FALSE and
                      b_ImmediateInterrupt == FALSE, then the pointer can
                      be set to NULL.
   output:    bool
  
   return:    true, if write was ok.
              false, if errors detected.
  
   date:      v0.93; 2009-03-31: fix of bit select mechanism.
                     2009-05-25: Swi_SendRawWord return treated as void.
   ************************************************************************* */
BOOL Swi_WriteRegisterSpace(UWORD uw_Address, UBYTE ub_Data, UBYTE ub_BitSelect, BOOL b_WaitForInterrupt, BOOL b_ImmediateInterrupt, BOOL * bp_IrqDetected )
{

  BOOL bIntOccured;
  UBYTE ubReadData = 0x00u;
  

  /* ub_BitSelect == 0x00 makes no sense! */
  if( ub_BitSelect == 0x00u )
  {
    return FALSE;
  }

  /* read out current data, if bit masking used */
  if( ub_BitSelect != 0xFFu )
  {
    if( Swi_ReadRegisterSpace( uw_Address, &ubReadData ) == FALSE)
    {
      return FALSE;
    }     
    ubReadData &= ~ub_BitSelect;
    ub_Data = ubReadData | (ub_Data & ub_BitSelect);
  }
  
  /* set burst length is 1 byte */
  Swi_SendRawWordNoIrq(SWI_BC, SWI_RBL0 );
  /*select device register set */
  Swi_SendRawWordNoIrq(SWI_BC, SWI_EREG );
  
  Swi_SendRawWordNoIrq(SWI_ERA, ((uw_Address >> 8u) & 0xFFu) );
  Swi_SendRawWordNoIrq(SWI_WRA, ( uw_Address        & 0xFFu) );
  
  Swi_SendRawWord( SWI_WD, ub_Data, b_WaitForInterrupt, b_ImmediateInterrupt, &bIntOccured );
  if( (b_WaitForInterrupt == TRUE) || (b_ImmediateInterrupt == TRUE) )
  {
    *bp_IrqDetected = bIntOccured;
  }
        
  /* all done */
  return TRUE;

}



/* ****************************************************************************
   name:      Swi_WriteRegisterSpaceNoIrq()
  
   function:  write new content to ORIGA register address space.
  
   input:     IN: uw_Address
                address within ORIGA register space to change the content.
              IN: ub_Data
                new data to store at the address pointed to by uw_Address
              IN: ub_BitSelect
                AND-Mask for doing selective change of register content.
                only bits set to '1' at ub_BitSelect are changed.
                NOTE: ub_BitSelect == 0xFF will alter all bits
   output:    bool
  
   return:    true, if write was ok.
              false, if errors detected.
  
   date:      .
   ************************************************************************* */
BOOL Swi_WriteRegisterSpaceNoIrq( UWORD uw_Address, UBYTE ub_Data, UBYTE ub_BitSelect )
{
  return Swi_WriteRegisterSpace( uw_Address, ub_Data, ub_BitSelect, FALSE, FALSE, NULL );
}



/* ****************************************************************************
   name:      Swi_ReadConfigSpace()
  
   function:  read one byte from the ORIGA config space.
  
   input:     IN: uw_Address
                address within config space to read one byte from.
              OUT: * ubp_Data
                pointer to UBYTE buffer to store read data into.
   output:    bool
  
   return:    true, if read was ok.
              false, if read errors detected.
  
   date:      .
   ************************************************************************* */
BOOL Swi_ReadConfigSpace( UWORD uw_Address, UBYTE * ubp_Data )
{

  BOOL bResult;
  U_SWI_WORD u_ReceivedWord;


  /* burst lenght is 1 byte */
  Swi_SendRawWordNoIrq(SWI_BC, SWI_RBL0 ); 
  /* select device config set */
  Swi_SendRawWordNoIrq(SWI_BC, SWI_ECFG );          
  
  Swi_SendRawWordNoIrq(SWI_ERA, ((uw_Address >> 8u ) & 0xFFu) );
  Swi_SendRawWordNoIrq(SWI_RRA, ( uw_Address         & 0xFFu) );
  
  bResult = Swi_ReceiveRawWord( &u_ReceivedWord );
  if( bResult == TRUE )
  {
    *ubp_Data = (UBYTE)(u_ReceivedWord.sSwiAbstact.SWI_DATA);
  }
  
  /* show result */
  return bResult;

}



/* ****************************************************************************
   name:      Swi_WriteConfigSpace()
  
   function:  write new content to ORIGA config address space.
  
   input:     IN: uw_Address
                address within ORIGA config space to change the content.
              IN: ub_Data
                new data to store at the address pointed to by uw_Address.
              IN: ub_BitSelect
                AND-Mask for doing selective change of register content.
                only bits set to '1' at ub_BitSelect are changed.
                NOTE: ub_BitSelect == 0xFF will alter all bits.
   output:    bool
  
   return:    true, if write was ok.
              false, if errors detected.
  
   date:      .
   ************************************************************************* */
BOOL Swi_WriteConfigSpace( UWORD uw_Address, UBYTE ub_Data, UBYTE ub_BitSelect )
{

  UBYTE ubReadData = 0u;
  
  
  /* ub_BitSelect == 0x00 makes no sense! */
  if( ub_BitSelect == 0x00u )
  {
    return FALSE;
  }

  /* read out current data, if bit masking used */
  if(ub_BitSelect != 0xFFu)
  {
    if( Swi_ReadConfigSpace(uw_Address, &ubReadData) == FALSE )
    {
      return FALSE;
    }
    ubReadData &= ~ub_BitSelect;
    ub_Data = ubReadData | (ub_Data & ub_BitSelect);
  }
  
  /* burst lenght is 1 byte */
  Swi_SendRawWordNoIrq(SWI_BC, SWI_RBL0 ); 
  /* select device config set */
  Swi_SendRawWordNoIrq(SWI_BC, SWI_ECFG );          
  
  Swi_SendRawWordNoIrq(SWI_ERA, ((uw_Address >> 8u) & 0xFFu) );
  Swi_SendRawWordNoIrq(SWI_WRA, ( uw_Address        & 0xFFu) );
  
  Swi_SendRawWordNoIrq(SWI_WD, ub_Data );

  /* all done */
  return TRUE;

}



/* ****************************************************************************
   name:      Swi_SearchPuid()
  
   function:  find unique ID of attached system.
              NOTE: this function only supports one attached system. for
                    ORGA multi UID handling please ask your AE.
  
   input:     IN: ub_BitsToSearch
                length of the UID in bits.
              OUT: * stp_DetectedPuid
                pointer to structure that will hold the detected UID.
   output:    BOOL
  
   return:    true, if read was ok.
              false, if read errors detected.
  
   date:      v0.93; 2009-05-25: Swi_SendRawWord return treated as void.
   ************************************************************************* */
BOOL Swi_SearchPuid( UBYTE ub_BitsToSearch, S_ORIGA_PUID * stp_DetectedPuid )
{

  BOOL bFound_0;
  BOOL bFound_1;
  UBYTE ubBitIndex = 0u;
  UBYTE ubByteIndex;
  UBYTE * ubpBytes = (UBYTE*)stp_DetectedPuid;

  
  /* do parameter checks */
  if( ( ub_BitsToSearch == 0u ) || ( ub_BitsToSearch > 96u ) )
  {
    return FALSE;
  }

  /* do bus reset first */
  Swi_Reset();
  /* start ID search */
  Swi_SendRawWordNoIrq( SWI_BC, SWI_DISS );


  /* search loop */
  for( ; ub_BitsToSearch != 0u; ub_BitsToSearch-- )
  {
    ubByteIndex = ubBitIndex >> 3u;
    ubpBytes[ubByteIndex] = ubpBytes[ubByteIndex] << 1u;
    
    /* take care that all two probes did perform correct */
    Swi_SendRawWord( SWI_BC, SWI_DIP0, TRUE, TRUE, &bFound_0 );
    Swi_SendRawWord( SWI_BC, SWI_DIP1, TRUE, TRUE, &bFound_1 );
				  
    /* check result and exit on error! */
    if( (bFound_1 == FALSE) && (bFound_0 == TRUE) )
    {
      ubpBytes[ubByteIndex] &= 0xFEu;
      Swi_SendRawWordNoIrq( SWI_BC, SWI_DIE0 );
    }
    else if( (bFound_1 == TRUE) && (bFound_0 == FALSE) )
    {
      ubpBytes[ubByteIndex] |= 0x01u;
      Swi_SendRawWordNoIrq( SWI_BC, SWI_DIE1 );
    }
    else
    {
      /* if nothing detected then break! */
      Swi_AbortIrq();
      return FALSE;
    }
    
    /* check next bit */
    ubBitIndex++;
  }
  
  /* all done */
  return TRUE;

}

/********************************************************************
*
* Function Name: b_UidSearch_GetDipDoneBit
* Description: This function is to be used in multiple UID search
*
* \param[in]  ub_BitInfo
*			  bit 0: 0: DIP needed	; 1: DIP not needed
*             bit 1: 0: DIE1 not needed;  1: DIE1 needed
*             bit 2: 0: DIE0 not needed;  1: DIE0 needed 
*
* \return TRUE if no processing error
********************************************************************/

BOOL b_UidSearch_GetDipDoneBit( UBYTE ub_BitInfo )
{
    if (ub_BitInfo & 0x01) 
		return TRUE;
	else 
		return FALSE;
}

/********************************************************************
*
* Function Name: b_UidSearch_SetDipDoneBit
* Description: This function is to be used in multiple UID search
*
* \param[out]  ub_BitInfo
*			  bit 0: 0: DIP needed	; 1: DIP not needed
*             bit 1: 0: DIE1 not needed;  1: DIE1 needed
*             bit 2: 0: DIE0 not needed;  1: DIE0 needed 
* \param[in]  b_Bit : TRUE: set
*                     FALSE: clear

* \return  -
********************************************************************/
void b_UidSearch_SetDipDoneBit( UBYTE * ub_BitInfo , BOOL b_Bit)
{
    if (b_Bit) 
		*ub_BitInfo = (*ub_BitInfo) | 0x01; // set
	else  
		*ub_BitInfo = (*ub_BitInfo) & 0xfe; // clear
}

/********************************************************************
*
* Function Name: b_UidSearch_GetDIE0Info
* Description: This function is to be used in multiple UID search
*
* \param[in]  ub_BitInfo
*			  bit 0: 0: DIP needed	; 1: DIP not needed
*             bit 1: 0: DIE1 not needed;  1: DIE1 needed
*             bit 2: 0: DIE0 not needed;  1: DIE0 needed 
* \param[in]  b_Bit : TRUE: set
*                     FALSE: clear

* \return  TRUE if DIE0(bit2) is set
********************************************************************/
BOOL b_UidSearch_GetDIE0Info( UBYTE ub_BitInfo )
{
    if (ub_BitInfo & 0x04) return TRUE;
	else return FALSE;
}

/********************************************************************
*
* Function Name: b_UidSearch_SetDIE0Info
* Description: This function is to be used in multiple UID search
*
* \param[out]  ub_BitInfo
*			  bit 0: 0: DIP needed	; 1: DIP not needed
*             bit 1: 0: DIE1 not needed;  1: DIE1 needed
*             bit 2: 0: DIE0 not needed;  1: DIE0 needed 
* \param[in]  b_Bit : TRUE: set
*                     FALSE: clear

* \return  -
********************************************************************/
void b_UidSearch_SetDIE0Info( UBYTE * ub_BitInfo , BOOL b_Data)
{
    if (b_Data) 
		*ub_BitInfo = (*ub_BitInfo) | 0x04;	 // set
	else  
		*ub_BitInfo  = (*ub_BitInfo) & 0xfb; // clear
}

/********************************************************************
*
* Function Name: b_UidSearch_GetDIE1Info
* Description: This function is to be used in multiple UID search
*
* \param[in]  ub_BitInfo
*			  bit 0: 0: DIP needed	; 1: DIP not needed
*             bit 1: 0: DIE1 not needed;  1: DIE1 needed
*             bit 2: 0: DIE0 not needed;  1: DIE0 needed 
* \param[in]  b_Bit : TRUE: set
*                     FALSE: clear

* \return  TRUE if DIE1(bit1) is set
********************************************************************/
BOOL b_UidSearch_GetDIE1Info( UBYTE ub_BitInfo )
{
    if (ub_BitInfo & 0x02) 
		return TRUE;
	else 
		return FALSE;
}

/********************************************************************
*
* Function Name: b_UidSearch_SetDIE1Info
* Description: This function is to be used in multiple UID search
*
* \param[out]  ub_BitInfo
*			  bit 0: 0: DIP needed	; 1: DIP not needed
*             bit 1: 0: DIE1 not needed;  1: DIE1 needed
*             bit 2: 0: DIE0 not needed;  1: DIE0 needed 

* \return  -
********************************************************************/
void b_UidSearch_SetDIE1Info( UBYTE * ub_BitInfo )
{
    *ub_BitInfo = (*ub_BitInfo) | 0x02;
}

/********************************************************************
*
* Function Name: b_pop
* Description: This function is to be used in multiple UID search
*
* \param[out]  ub_Data: read out last data in ub_Stack

* \return  TRUE if no error
********************************************************************/
BOOL b_pop(UBYTE * ub_Data)
{
    if(ub_StackPointer == 0 ) 
		return FALSE;	// stack empty
	*ub_Data = ub_Stack[--ub_StackPointer];
	return TRUE;
}

/********************************************************************
*
* Function Name: b_push
* Description: This function is to be used in multiple UID search
*
* \param[out]  ub_Data: push data at the end of  ub_Stack

* \return  TRUE if no error
********************************************************************/
BOOL b_push( UBYTE ub_Data )
{
    if (ub_StackPointer == 96 ) return FALSE; // stack full
	ub_Stack[ub_StackPointer++] = ub_Data;
	return TRUE;
}

/********************************************************************
*
* Function Name: ub_SizeOfStack
* Description: This function is to be used in multiple UID search
*
* \return  Size of ub_Stack
********************************************************************/
UBYTE ub_SizeOfStack()
{
    return ub_StackPointer;
}

/********************************************************************
*
* Function Name: bif_MultiUidSearch
* Description: This function is to be used in multiple UID search
* devAddr is assigned sequentially from 1 to N for each searched device
* \param[out]  stp_DetectedPuid
* \param[out]  ubp_DevCnt: slave amount in system

* \return  TRUE if no error
********************************************************************/
BOOL Swi_SearchMultiplePuid(UBYTE ub_BitsToSearch,  S_ORIGA_PUID *stp_DetectedPuid, UBYTE * ubp_DevCnt )
{
    UBYTE ubBitInfo[96];
	UBYTE ubCurrentIdPtr, ubLastIdPtr =0;
	UBYTE ubSlaveCnt = 0;
	BOOL bFound_0;
  	BOOL bFound_1;
	UBYTE ubBitCnt = ub_BitsToSearch;
  	UBYTE ubByteIndex;
	BOOL bSearchDone = FALSE;
	UBYTE i;
  	UBYTE ubpBytes[12];
	UBYTE *ptr=(UBYTE*)stp_DetectedPuid;

	ub_StackPointer = 0;  // clear stack first
	for (i = 0;i < ubBitCnt;i ++){
		ubBitInfo[i] = 0; // clear all info for all 80 bits
	}

	do
	{
		/* Restore device state */
		if(ubSlaveCnt > 0){
			 Swi_SendRawWordNoIrq(SWI_BC, SWI_DIE0);
		}

		/* start ID search */
		Swi_SendRawWordNoIrq(SWI_BC, SWI_DISS);

		ubCurrentIdPtr = ub_BitsToSearch;
		ubBitCnt = ub_BitsToSearch;
		for ( i = 0;i < 12;i ++){
			ubpBytes[i] = 0;
		}

		for ( ; ubBitCnt > 0; ubBitCnt --)
		{
		    ubCurrentIdPtr --; 
			ubByteIndex = 11 - (ubCurrentIdPtr >> 3u);
		    ubpBytes[ubByteIndex] = ubpBytes[ubByteIndex] << 1u;

		    if (b_UidSearch_GetDipDoneBit(ubBitInfo[ubCurrentIdPtr]) == FALSE )	  // need to do DIP
			{
				// DIP0
		 		Swi_SendRawWordNoIrq(SWI_BC, SWI_DIP0);
				Swi_WaitForIrq(&bFound_0, TRUE);
				udelay(g_ulResponseTimeOut);

				// DIP1
		 		Swi_SendRawWordNoIrq(SWI_BC, SWI_DIP1);
				Swi_WaitForIrq(&bFound_1, TRUE); 
				udelay(g_ulResponseTimeOut);

				if( (bFound_0 == TRUE) && (bFound_1 == TRUE)) // both DIP0 and DIP1 has positive response
				{
					ubLastIdPtr = ubCurrentIdPtr;
					if( b_push( ubLastIdPtr ) == FALSE){
						return FALSE;
					}

		      		ubpBytes[ubByteIndex] &= 0xFEu;	// DIE0
		  			Swi_SendRawWordNoIrq(SWI_BC, SWI_DIE0);
					b_UidSearch_SetDIE0Info( &ubBitInfo[ubLastIdPtr] , TRUE);
					b_UidSearch_SetDIE1Info( &ubBitInfo[ubCurrentIdPtr] );
				}
				else if ((bFound_0 == TRUE) && (bFound_1 == FALSE))	  // DIE0
				{
		      		ubpBytes[ubByteIndex] &= 0xFEu;
		  			Swi_SendRawWordNoIrq(SWI_BC, SWI_DIE0);
					b_UidSearch_SetDIE0Info( &ubBitInfo[ubCurrentIdPtr], TRUE );
				}
				else if ((bFound_0 == FALSE) && (bFound_1 == TRUE)) // DIE1
				{
		      		ubpBytes[ubByteIndex] |= 0x01u;
		  			Swi_SendRawWordNoIrq(SWI_BC, SWI_DIE1);
					b_UidSearch_SetDIE1Info( &ubBitInfo[ubCurrentIdPtr] );
				}
				else 
					return FALSE;
				b_UidSearch_SetDipDoneBit( &ubBitInfo[ubCurrentIdPtr] , TRUE);
	
	
			} 
			else  // DIPDone == TRUE
			{
			    if (ubCurrentIdPtr == ubLastIdPtr){
					b_pop( &ubCurrentIdPtr );
				}

				if ( b_UidSearch_GetDIE0Info( ubBitInfo[ubCurrentIdPtr] )){
		      		ubpBytes[ubByteIndex] &= 0xFEu;
		  			Swi_SendRawWordNoIrq(SWI_BC, SWI_DIE0);
				}
				else if (b_UidSearch_GetDIE1Info( ubBitInfo[ubCurrentIdPtr])){
		      		ubpBytes[ubByteIndex] |= 0x01u;
		  			Swi_SendRawWordNoIrq(SWI_BC, SWI_DIE1);
				}
			}

			if (ubCurrentIdPtr == 0) // clear dip done from last bit index that has two positive response
			{
			    if ( ub_SizeOfStack() == 0)  
					bSearchDone = TRUE;
				else 
				{
					// refresh ubLastIdPtr
				    b_pop( &ubLastIdPtr );
					b_push( ubLastIdPtr );
				}
				// clear DIP done bit since last id pointer
			    for  (i = 0;i < ubLastIdPtr;i ++){
					b_UidSearch_SetDipDoneBit( &ubBitInfo[i] , FALSE);
			    }
				
				b_UidSearch_SetDIE0Info( &ubBitInfo[ubLastIdPtr] , FALSE);	// clear

			}  // if (ubCurrentIdPtr == 0)
		}   //for ( ; ubBitCnt > 0; ubBitCnt --)

		for(i=0;i<12;i++){
	    	ptr[i] = ubpBytes[i];
		}

		ubSlaveCnt ++;
		ptr += sizeof(S_ORIGA_PUID);

		/* Configure device address from 1 to N for each found device */
		(void)Swi_WriteConfigSpace(SWI_DADR0,  (ubSlaveCnt        & 0xFFu), 0xFFu );
        (void)Swi_WriteConfigSpace(SWI_DADR1, ((ubSlaveCnt >> 8u) & 0xFFu), 0xFFu );

	}while(bSearchDone == FALSE);

	*ubp_DevCnt = ubSlaveCnt;
    return TRUE;	
}

/* ****************************************************************************
   name:      Swi_SelectByPuid()
  
   function:  select device by applying a given device UID.
              after UID selection a device address can be set for easier
              device handling.
  
   input:     IN: ub_BitsToExecute
                number of bits to execute.
              IN: * stp_DeviceToSelect
                pointer to UID structure to hold the UID to use.
              IN: uw_AssignDevAdr
                device address that has to be assigned after UID selection.
                NOTE: assignment is only done, if uw_AssignDevAdr != 0.
   output:    bool
  
   return:    'true', if selection process passed.
              'false', if selection process failed.
  
   date:      v0.93; 2009-05-25: Swi_SendRawWord return treated as void.
   ************************************************************************* */
BOOL Swi_SelectByPuid( UBYTE ub_BitsToExecute, S_ORIGA_PUID * stp_DeviceToSelect, UWORD uw_AssignDevAdr )
{
  
  UBYTE ubByteIndex;
  UBYTE ubBitIndex = 0;
  
  BOOL bRefBit;
  UBYTE * ubpRefBytes = (UBYTE*)stp_DeviceToSelect;

  
  if( (ub_BitsToExecute == 0u) || (ub_BitsToExecute > 96u) )
  {
    return FALSE;
  }

  /* Restore device state */
  Swi_SendRawWordNoIrq( SWI_BC, SWI_DIE0 );

  /* start uid search */
  Swi_SendRawWordNoIrq( SWI_BC, SWI_DISS );
  
  /* do bit select loop */
  for( ; ub_BitsToExecute != 0u; ub_BitsToExecute-- )
  {

    ubByteIndex = ubBitIndex >> 3u;
    bRefBit = (ubpRefBytes[ubByteIndex] & (1u << ( 7u - (ubBitIndex & 0x07u) )));

	if( bRefBit == 0u )
    	Swi_SendRawWordNoIrq( SWI_BC, SWI_DIE0 );
	else
		Swi_SendRawWordNoIrq( SWI_BC, SWI_DIE1 );

    ubBitIndex++;
  }

  /* check, if device assignment requested */
  if( uw_AssignDevAdr != 0u )
  {  
    /* function returns always true, if mask is set to 0xFF */
    (void)Swi_WriteConfigSpace(SWI_DADR0,  (uw_AssignDevAdr        & 0xFFu), 0xFFu );
    (void)Swi_WriteConfigSpace(SWI_DADR1, ((uw_AssignDevAdr >> 8u) & 0xFFu), 0xFFu );
  }

  /* all done */
  return TRUE;

}



/* ****************************************************************************
   name:      Swi_SelectByAddress()
  
   function:  select device by device address.
  
   input:     IN: uw_DeviceAddress
                device address.
   output:    -
  
   return:    -
  
   date:      v0.93; 2009-05-25: Swi_SelectByAddress return treated as void.
   ************************************************************************* */
void Swi_SelectByAddress(UWORD uw_DeviceAddress)
{
  Swi_SendRawWordNoIrq( SWI_EDA, ( (UBYTE)((uw_DeviceAddress >> 8u) & 0xFFu)) );
  Swi_SendRawWordNoIrq( SWI_SDA, ( (UBYTE)( uw_DeviceAddress        & 0xFFu)) ); 
}



/* ****************************************************************************
   name:      Swi_PowerDown()
  
   function:  do hardware power down by pushing SWI to low for minimum of
              200us.
  
   input:     -
   output:    -
  
   return:    -
  
   date:      v1.00; 2011-07-21: alternate pin handling removed.
              v1.01; 2011-07-21: do limit check!
   ************************************************************************* */
void Swi_PowerDown( void )
{
  set_pin(0);

  /* SWI has to be held at low for at least 200us! */
 udelay(g_ulBaudPowerDownTime);

}



/* ****************************************************************************
   name:      Swi_PowerUp()
  
   function:  start power up sequence by setting SWI to high for at least
              10ms.
  
   input:     -
   output:    -
  
   return:    -
  
   date:      v1.00; 2011-07-21: alternate pin handling removed.
              v1.01; 2011-07-21: do limit check!
   ************************************************************************* */
void Swi_PowerUp( void )
{

  set_pin(1);

  udelay(g_ulBaudPowerUpTime);

}



/* ****************************************************************************
   name:      Swi_Reset()
  
   function:  execute bus reset command.
  
   input:     -
   output:    -
  
   return:    -
  
   date:      .
   ************************************************************************* */
void Swi_Reset( void )
{
  Swi_SendRawWordNoIrq( SWI_BC, SWI_BRES );
  udelay(g_ulBaudResetTime);
}



/* ****************************************************************************
   name:      Swi_TreatInvertFlag()
  
   function:  count number of used 3TAUs (ones) and invert data, if number of
              ones is greater than six. invert flag and data if required.
  
   input:     IN/OUT: * up_DataWord
                pointer to U_SWI_WORD that has to hold valid SWI stream data.
                regarding the number of detected ones, the invert flag and data
                is inverted.
   output:    -
  
   return:    -
  
   date:      .
   ************************************************************************* */
void Swi_TreatInvertFlag( U_SWI_WORD * up_DataWord )
{

  UBYTE ubBits;
  UBYTE ubCount = 0u;

  UWORD uwTarget = up_DataWord->uwWord;
  
  /* count '1'es */
  for( ubBits = 0u; ubBits < 16u; ubBits++)
  {
    ubCount += (UBYTE)(uwTarget) & 1u;
    uwTarget >>= 1u;
  }

  /* check, if invert required */
  if( ubCount > 6u )
  {
    up_DataWord->sSwiInvRelevant.SWI_DATA = ~up_DataWord->sSwiInvRelevant.SWI_DATA;
    up_DataWord->sSwiInvRelevant.SWI_INV = 1u;
  }

}



/* ****************************************************************************
   name:      Swi_AbortIrq()
  
   function:  send SWI IRQ to active finish IRQ mode on SWI line.
  
   input:     -
   output:    -
  
   return:    -
  
   date:      v1.00; 2011-07-21: alternate pin handling removed.
   ************************************************************************** */
void Swi_AbortIrq( void )
{

	// set bif gpio as output 
	set_pin_dir(1);

	set_pin(0);
	// delay for 1 tau
	udelay(g_ulBaudLow);
	set_pin(1);	

	// set bif gpio as input
	set_pin_dir(0);

}



/* ****************************************************************************
   name:      Swi_WaitForIrq()
  
   function:  wait for SWI IRQ to happen on SWI line.
  
   input:     OUT: * bp_IrqDetected
                is set to 'true', is IRQ was detected, else is set to 'false'.
              IN: b_Immediate
                if 'true', then poll period for IRQ checking is set to immediate
                time slice, else the long slice timing is used.
   output:    -
  
   return:    -
  
   date:      v0.93; 2009-05-25: changed function return from 'BOOL' to 'void'.
              v1.00; 2011-07-21: alternate pin handling removed.
   ************************************************************************* */
void Swi_WaitForIrq( BOOL* bp_IrqDetected, BOOL b_Immediate)
{

/*************** local variables ***************/
	BOOL bResult = FALSE;
	volatile ULONG ulTimeOut;

/******************* codes *********************/
	
//	ulTimeOut = g_ulIntTimeoutDefault;
	if( b_Immediate )
		ulTimeOut = g_ulResponseTimeOut;
	else
		ulTimeOut = 0x00FFFFFF; // wait for a long time.
	
	*bp_IrqDetected = FALSE;

	while(ulTimeOut)
	{
		//if(!(P0_0_read()))
		if(!(get_pin()))
		{
			bResult = TRUE;
			*bp_IrqDetected = TRUE;
			break;
		}
		ulTimeOut--;
	}

	udelay(g_ulBaudLow);
	udelay(g_ulBaudStop);

	if(!bResult)
		Swi_AbortIrq();
	
	return;

}



/* ****************************************************************************
   name:      Swi_SendRawWord()
  
   function:  translate requested code and data payload to SWI bit stream
              after translation send out SWI stream.
  
   input:     IN: ub_Code
                SWI command to send-
              IN: ub_Data
                SWI command extension or payload data to send.
              IN: b_WaitForInterrupt
                wait until SWI IRQ is detected.
              IN: b_ImmediateInterrupt
                wait until SWI IRQ is detected directly after command is sent.
              OUT: * bp_IrqDetected
                pointer to bool to save detection of SWI IRQ.
                NOTE: if b_WaitForInterrupt == FALSE and
                      b_ImmediateInterrupt == FALSE, then the pointer can
                      be set to NULL.
   output:    -
  
   return:    -
  
   date:      v0.93; 2009-03-31: treat return results.
                     2009-05-25: return value changed from 'BOOL' to 'void'.
              v1.00; 2011-07-21: alternate pin handling removed.
                     2011-07-21: Swi_SendRawWord() send STOP first to have no
                                 issue with following IRQ or read, due to CPU
                                 latency.
              v1.02; 2011-07-21: not needed code before IRQ handling removed.
   ************************************************************************* */
void Swi_SendRawWord( UBYTE ub_Code, UBYTE ub_Data, BOOL b_WaitForInterrupt, BOOL b_ImmediateInterrupt, BOOL * bp_IrqDetected )
{

  BOOL bInterruptOccured = FALSE;
  U_SWI_WORD uSwiWord;
  S_SWI_WORDBITS * sSwiWordBits = &uSwiWord.sSwiBits;


  /* setup required data */
  uSwiWord.uwWord = 0u;
  uSwiWord.sSwiAbstact.SWI_CODE = ub_Code;
  uSwiWord.sSwiAbstact.SWI_DATA = ub_Data;
  Swi_TreatInvertFlag(&uSwiWord);

  /* Send a STOP signal first to have time to receive either IRQ or data! */
	// set bif gpio as output 
	set_pin_dir(1);
  	// send a stop command first.
	set_pin(1);
	udelay(g_ulBaudStop);

   	// send BCF
	set_pin(0);
	sSwiWordBits->SWI_BCF ? udelay(g_ulBaudHigh): udelay(g_ulBaudLow);
	// send _BCF
	set_pin(1);
	sSwiWordBits->SWI_BCFQ ? udelay(g_ulBaudHigh): udelay(g_ulBaudLow);
   	// send BIT9
	set_pin(0);
	sSwiWordBits->SWI_D9 ? udelay(g_ulBaudHigh): udelay(g_ulBaudLow);
	// send BIT8
	set_pin(1);
	sSwiWordBits->SWI_D8 ? udelay(g_ulBaudHigh): udelay(g_ulBaudLow);
   	// send BIT7
	set_pin(0);
	sSwiWordBits->SWI_D7 ? udelay(g_ulBaudHigh): udelay(g_ulBaudLow);
	// send BIT6
	set_pin(1);
	sSwiWordBits->SWI_D6 ? udelay(g_ulBaudHigh): udelay(g_ulBaudLow);
   	// send BIT5
	set_pin(0);
	sSwiWordBits->SWI_D5 ? udelay(g_ulBaudHigh): udelay(g_ulBaudLow);
	// send BIT4
	set_pin(1);
	sSwiWordBits->SWI_D4 ? udelay(g_ulBaudHigh): udelay(g_ulBaudLow);
   	// send BIT3
	set_pin(0);
	sSwiWordBits->SWI_D3 ? udelay(g_ulBaudHigh): udelay(g_ulBaudLow);
	// send BIT2
	set_pin(1);
	sSwiWordBits->SWI_D2 ? udelay(g_ulBaudHigh): udelay(g_ulBaudLow);
   	// send BIT1
	set_pin(0);
	sSwiWordBits->SWI_D1 ? udelay(g_ulBaudHigh): udelay(g_ulBaudLow);
	// send BIT0
	set_pin(1);
	sSwiWordBits->SWI_D0 ? udelay(g_ulBaudHigh): udelay(g_ulBaudLow);

  /* Send Inversion bit */
	set_pin(0);
	sSwiWordBits->SWI_INV ? udelay(g_ulBaudHigh): udelay(g_ulBaudLow);

  
	// Send Stop Command
	set_pin(1);

	udelay(g_ulBaudStop);

	// set bif gpio as input
	set_pin_dir(0);


  /* Check for IRQ event, if requested */
  if( b_WaitForInterrupt == TRUE )
  {
    /* wait for interrupt */
    Swi_WaitForIrq( &bInterruptOccured, b_ImmediateInterrupt );
  }
 
  /* store IRQ data, if requested */
  if( (b_WaitForInterrupt == TRUE) || (b_ImmediateInterrupt == TRUE) )
  {
   *bp_IrqDetected = bInterruptOccured;
  }

}



/* ****************************************************************************
   name:      Swi_SendRawWordNoIrq()
  
   function:  Send out SWI-Word without waiting for IRQ response. 
  
   input:     IN: ub_Code:
                SWI command to send
              IN: ub_Data:
                SWI 2nd part of command or payload, if data is sent. 
   output:    -
  
   return:    -
  
   date:      v0.93; 2009-03-31: function "returns" void.
   ************************************************************************* */
void Swi_SendRawWordNoIrq(UBYTE ub_Code, UBYTE ub_Data )
{
  Swi_SendRawWord( ub_Code, ub_Data, FALSE, FALSE, NULL );
}



/* ****************************************************************************
   name:      Swi_ReceiveRawWord()
  
   function:  receice data via SWI interface.
              measure time between high and low phases and adjust to detected
              high low phase timing. depending on calculated threshold the
              bits of the detected SWI signal are put together to one SWI
              bit stream result.
  
   input:     OUT: * up_Word
                pointer to union to hold detected SWI bit stream.
   output:    BOOL
  
   return:    true, if read was ok.
              false, if read errors detected.
  
   date:      2011-07-21; v1.00: enable IRQs in case of function return, due to
                                 detected error.
   ************************************************************************* */
BOOL Swi_ReceiveRawWord( U_SWI_WORD * up_Word )
{

  BOOL bPreviousSwiState;
  UBYTE ubIndex = 12u;
  UBYTE ubBitsToCapture;
  
  ULONG ulTimeOut = g_ulResponseTimeOut;

  ULONG ulTimes[13];
  ULONG ulMaxTime = 0u;
  ULONG ulMinTime = ~ulMaxTime; 
  ULONG ulCount;
  ULONG ulThreshold;

  //P0_6_reset();
  //P0_6_set();

  //P0_0_set_mode(INPUT);

	while(get_pin() && ulTimeOut)
	{
		ulTimeOut--;
	}
  // exit with fail, if timeout criteria triggered
  if( ulTimeOut == 0u )
  {
    return FALSE;
  }

  //P0_6_reset();
  //P0_6_set();

  /* get port state */   
  bPreviousSwiState = get_pin();


  /* measure time of high and low phases */
  for( ubBitsToCapture = 13u; ubBitsToCapture != 0u; ubBitsToCapture-- )
  {
    ulCount = 0u;
    ulTimeOut = g_ulResponseTimeOut;

    while( get_pin() == bPreviousSwiState && ulTimeOut )
    {
      ulCount++;
	  ulTimeOut--;

    }
    if( ulTimeOut == 0u )
    {
      return FALSE;
    }

    ulTimes[ubIndex] = ulCount;
    ubIndex--;

    bPreviousSwiState =  get_pin();
  }

  //P0_6_reset();
  //P0_6_set();

  /* evaluate detected results */ 
  for( ubIndex = 12u; ubIndex != 0u; ubIndex-- )
  {
    ulCount = ulTimes[ubIndex];
    if( ulCount < ulMinTime)
    {
      ulMinTime = ulCount;
    }
    else if(ulCount > ulMaxTime)
    {
      ulMaxTime = ulCount;
    }
    else
    {
    /*  no change required */
    }
  }
  
  /* calculate threshold */
  ulThreshold = ((ulMaxTime - ulMinTime) >> 1u);
  ulThreshold += ulMinTime;

  /* get bits according threshold */
  up_Word->sSwiBits.SWI_BCF =  (ulTimes[12] > ulThreshold) ? 1u : 0u;
  up_Word->sSwiBits.SWI_BCFQ = (ulTimes[11] > ulThreshold) ? 1u : 0u;
  up_Word->sSwiBits.SWI_D9 =   (ulTimes[10] > ulThreshold) ? 1u : 0u;
  up_Word->sSwiBits.SWI_D8 =   (ulTimes[ 9] > ulThreshold) ? 1u : 0u;
  up_Word->sSwiBits.SWI_D7 =   (ulTimes[ 8] > ulThreshold) ? 1u : 0u;
  up_Word->sSwiBits.SWI_D6 =   (ulTimes[ 7] > ulThreshold) ? 1u : 0u;
  up_Word->sSwiBits.SWI_D5 =   (ulTimes[ 6] > ulThreshold) ? 1u : 0u;
  up_Word->sSwiBits.SWI_D4 =   (ulTimes[ 5] > ulThreshold) ? 1u : 0u;
  up_Word->sSwiBits.SWI_D3 =   (ulTimes[ 4] > ulThreshold) ? 1u : 0u;
  up_Word->sSwiBits.SWI_D2 =   (ulTimes[ 3] > ulThreshold) ? 1u : 0u;
  up_Word->sSwiBits.SWI_D1 =   (ulTimes[ 2] > ulThreshold) ? 1u : 0u;
  up_Word->sSwiBits.SWI_D0 =   (ulTimes[ 1] > ulThreshold) ? 1u : 0u;
  up_Word->sSwiBits.SWI_INV =  (ulTimes[ 0] > ulThreshold) ? 1u : 0u;

  /* check, if payload had to be inverted to to invert flag */ 
  if( up_Word->sSwiBits.SWI_INV == 1u )
  {
    up_Word->sSwiBits.SWI_INV = 0u;
    up_Word->sSwiInvRelevant.SWI_DATA = ~up_Word->sSwiInvRelevant.SWI_DATA;
  }

  /* check that SWI_BCF is not equal to SWI_BCFQ */
  if(up_Word->sSwiBits.SWI_BCF == up_Word->sSwiBits.SWI_BCFQ)
  {
    return FALSE;
  }
  
  /* all done well */
  return TRUE;

}


#endif

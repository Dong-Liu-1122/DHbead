
#include "F2806x_Device.h"
#include "Stepper-Settings.h"

#if (Origa_Ena)

#include "Origa_Ecc.h"
#include "board.h"


#define FIXED_CHALLENGE   0 ///

//ECC Curve Parameters
/** square root of the parameter b of the 
  * elliptic curve E(a,b) defined by the equation 
  * y^2 + xy = x^3 + ax^2 + b,  a = 0
  */
gf2n_t g_gf2n_sqrt_coeff_b = {0x055eu,0x1b80u, 0x5a9bu,0xe2edu, 0xf787u,0x1dcfu, 0x8a70u,0x3d07u, 0x0000u};
/* affine x-coordinate of the base point P = (xP, yP) */
gf2n_t g_gf2n_xP = {0x601fu,0xd285u, 0xb6f5u,0x4ec4u, 0x2fd3u,0x5025u, 0xa61bu,0x0720u, 0x0006u};

//public key
gf2n_t g_gf2nPublicKey ={0x45edu,0x7dfeu,0x2581u,0x6866u,0x0029u,0x8a1eu,0x64abu,0x1666u,0x07u};
//gf2n_t g_gf2nPublicKey ={0x23edu,0x1111u,0x2581u,0x6866u,0x0029u,0x8a1eu,0x64abu,0x1666u,0x07u};

#if (FIXED_CHALLENGE == 1)
gf2n_t gf2n_FixChallenge	= {0x92AE, 0xD159, 0x906C, 0xE125, 0xCE7D, 0x54FC, 0x6AE0, 0x2537, 0x1};
gf2n_t gf2n_FixCheckValue  = {0x371B, 0x3A91, 0xB7FD, 0xBB65, 0xEE7D, 0x127A, 0x77B6, 0x7CB5, 0x5};
#endif

/** !!!!!!!!!! W A R N I N G !!!!!!!!!!
  *
  * this implementation calls the system function rand() for random number generation.
  * rand() is not a cryptographically strong random number generator.
  *
  * !!!!! this implementation must be replaced by a programm using an unpredictable
  * true or pseudo random number generator with good statistics !!!!!
  */

/** generates a 128 bit random number
  * \param[out] erg generated random number
  */
void Ecc_Rng128( gf2n_t gf2n_RandomValue )
{

  UBYTE ubCount;
  UWORD uwRand;

#ifndef ORIGA_ECCSTATIC_TEST
//#error "WARNING: Randomize rand() during each Host startup (e.g. using srand()) !!!"
#endif

  for( ubCount = 0u; ubCount < 8u; ubCount++ )
  {
    #ifndef ORIGA_ECCSTATIC_TEST

    /* NOTE: split for debug purpose (gf2n_RandomValue[ubCount] = rand();) */
    uwRand = rand();
    gf2n_RandomValue[ubCount] = uwRand;

    #endif
  }
  gf2n_RandomValue[8] = 0u;

}

/* ****************************************************************************
   name:      Ecc_DoAuthentication()

   function:  execute a complete authentication sequence.

   input:     OUT: bp_AuthenticationResult
                pointer to bool variable to store authentication result into.
   output:    bool

   return:    true, if all was ok.
              false, if errors detected.

   date:      .
   ************************************************************************* */
BOOL Ecc_DoAuthentication( BOOL * bp_AuthenticationResult )
{

  gf2n_t gf2nRandomValue;
  gf2n_t gf2nChallenge;
  gf2n_t gf2nReturnX;
  gf2n_t gf2nReturnZ;
  gf2n_t gf2nCheck;
  

  /* Generate Challenge using the ECC routine */
  *bp_AuthenticationResult = FALSE;
  
  if( Ecc_GenerateChallenge( gf2nChallenge, gf2nRandomValue ) == FALSE)
  {
	  return FALSE;
  }

  if( Ecc_GenerateCheckValue( gf2nCheck, gf2nRandomValue, g_gf2nPublicKey )== FALSE)
  {
	  return FALSE;
  }

  if( Ecc_SendChallengeAndGetResponse( gf2nChallenge, gf2nReturnX, gf2nReturnZ, FALSE ) == FALSE )
  {
	  return FALSE;
  }

  if( Ecc_VerifyResponse( gf2nReturnX, gf2nReturnZ, gf2nCheck ) == TRUE )
  {
	  *bp_AuthenticationResult = TRUE;
  }
    
  return *bp_AuthenticationResult;

}

/* ****************************************************************************
   name:      Ecc_GenerateChallenge()
  
   function:  create a new challenge to be sent to ORIGA.
  
   input:     OUT: gf2n_Challenge
                gf2n_t array for challenge to be stored into.  
              OUT: gf2n_RandomValue
                gf2n_t array for random values to be stored into.
   output:    bool
  
   return:    true, if all was ok.
              false, if errors detected.
  
   date:      .
   ************************************************************************* */
BOOL Ecc_GenerateChallenge( gf2n_t gf2n_Challenge, gf2n_t gf2n_RandomValue )
{
	//UBYTE i=0;
#if (FIXED_CHALLENGE == 1)
	for(i=0; i< (sizeof(gf2n_FixChallenge)/2);i++)
		gf2n_Challenge[i] = gf2n_FixChallenge[i];
    return TRUE;
#endif
    enum E_GF2N_RETURNVALUE eResult;


     /* get new random value and calculate challenge */
     Ecc_Rng128( gf2n_RandomValue );
     eResult = scalar_mul( gf2n_Challenge, g_gf2n_xP, gf2n_RandomValue );

     /* do active check that creation passed */
     if( eResult == GF2N_RETURN_OK )
     {
       return TRUE;
     }

     /* error case */
     return FALSE;
}

/* ****************************************************************************
   name:      Ecc_GenerateCheckValue()
  
   function:  get check value that is needed to verify the ORIGA response.
              the checkvalue is linked to the random value and pubkey!
  
   input:     OUT: gf2n_CheckValue
                gf2n_t array for checkvalue to be stored into.  
              IN: gf2n_RandomValue
                random values that are used for the current challenge.
              IN: gf2n_PublicKey
                public key that is used for the current challenge.
                 
   output:    bool
  
   return:    true, if all was ok.
              false, if errors detected.
  
   date:      .
   ************************************************************************* */
BOOL Ecc_GenerateCheckValue( gf2n_t gf2n_CheckValue, gf2n_t gf2n_RandomValue, gf2n_t gf2n_PublicKey )
{

#if (FIXED_CHALLENGE == 1)
	UBYTE i=0;
	for(i=0; i< (sizeof(gf2n_FixCheckValue)/2);i++)
		gf2n_CheckValue[i] = gf2n_FixCheckValue[i];
    return TRUE;
#endif


    enum E_GF2N_RETURNVALUE eResult;


    /* calculate checkvalze */
    eResult = scalar_mul( gf2n_CheckValue, gf2n_PublicKey, gf2n_RandomValue );

    /* do active check that creation passed */
    if(eResult == GF2N_RETURN_OK)
    {
      return TRUE;
    }

    /* error case */
    return FALSE;

}

/* ****************************************************************************
   name:      Ecc_SendChallengeAnGetResponse()
  
   function:  send a calculated challenge to ORIGA and start ECC engine of
              the ORIGA IC.
              if ORIGA indicates calculation finished, the results are read
              from ORIGA memory space into the arrays gf2n_XResponse and
              gf2n_ZResponse.
  
   input:     IN: gf2n_Challenge
                gf2n_t array holding the challenge to be issued.
              OUT: gf2n_XResponse
                gf2n_t array holding the x part of the ORIGA response.  
              OUT: gf2n_ZResponse  
                gf2n_t array holding the z part of the ORIGA response.  
              IN: bPolling
                polling mode to check ORIGA engine for calculation finished
                state.
                if 'true', then a wait of 200ms is done before the data is
                read back, else the host waits for SWI IRQ signal.
                NOTE: please use FALSE, as that is the most efficient way
                      to handle the ECC engine.
   output:    bool
  
   return:    true, if all was ok.
              false, if errors detected.
  
   date:      v0.93; 2009-05-25: Swi_SendRawWord return treated as void.
              v1.03; 2011-07-22: Restrict reading of XResponse to 8 bytes.
                     2011-07-28: Some Swi_SendRawWord() replaced by 
                                 Swi_SendRawWordNoIrq.
************************************************************************* */
BOOL Ecc_SendChallengeAndGetResponse( gf2n_t gf2n_Challenge, gf2n_t gf2n_XResponse, gf2n_t gf2n_ZResponse, BOOL bPolling )
{

  BOOL bEccIrq;
  UBYTE ubCap7Value;
  UBYTE ubInt0Value;
  UBYTE ubIndex;
  UBYTE ubWordIndex;
  UBYTE ubData;
    
  /* en/disable capabilities according polling selection */
  ubCap7Value = 0x80u;
  ubInt0Value = 0x01u;

  if( bPolling == TRUE )
  {
    ubCap7Value = 0x00u;
    ubInt0Value = 0x00u;
  }

  if( Swi_WriteConfigSpace( SWI_CAP7, ubCap7Value, 0x80u ) == FALSE )
  {
    return FALSE;
  }

  if( Swi_WriteConfigSpace( SWI_INT0, ubInt0Value, 0x01u ) == FALSE )
  {
    return FALSE;
  }

  /*select device register set */
  Swi_SendRawWordNoIrq(SWI_BC, SWI_EREG );

     
  /* write 16 bytes of challenge */
  for( ubIndex = 0u; ubIndex < 16u; ubIndex++ )
  {
    /* set start each aligned 8 byte addresses */
    if( (ubIndex & 0x07u) == 0u )
    {
      Swi_SendRawWordNoIrq(SWI_ERA, (((0x0040u + ubIndex) >> 8u) & 0xFFu) );
      Swi_SendRawWordNoIrq(SWI_WRA, ( (0x0040u + ubIndex)        & 0xFFu) );
    }

    ubWordIndex = (ubIndex >> 1u);
    if( (ubIndex & 1u) == 1u )
    {
      ubData = (UBYTE)((gf2n_Challenge[ubWordIndex] >> 8u) & 0x00FFu);
    }
    else
    {
      ubData = (UBYTE)(gf2n_Challenge[ubWordIndex] & 0xFFu);
    }
    
    /* write data w/o any interruption */
    Swi_SendRawWordNoIrq( SWI_WD, ubData );
  }
  
  /* write remaining last bytes */
  Swi_SendRawWordNoIrq(SWI_ERA, ((0x0340u >> 8u) & 0xFFu) );
  Swi_SendRawWordNoIrq(SWI_WRA, ( 0x0340u        & 0xFFu) );

  ubData = (UBYTE)(gf2n_Challenge[8] & 0xFFu);
  Swi_SendRawWordNoIrq( SWI_WD, ubData );

  /* start ECC calculation */
  Swi_SendRawWordNoIrq(SWI_BC,SWI_ORIGA_ECCSTART);

  if( bPolling == TRUE )
  {
	udelay(50000);
	udelay(50000);
	udelay(50000);
  }
  else
  {
    Swi_SendRawWord( SWI_BC, SWI_EINT, TRUE, FALSE, &bEccIrq);
    if( bEccIrq == FALSE )
    {
      return FALSE;
    }
	Swi_WriteConfigSpace( SWI_CAP7, 0x00u, 0x80u );
	Swi_WriteConfigSpace( SWI_INT0, 0x00u, 0x01u );
  }

  /* extract responses */
  /* set burst length to 1 byte at a time*/
  Swi_SendRawWordNoIrq(SWI_BC, SWI_RBL0 );
  /* select register set */
  Swi_SendRawWordNoIrq(SWI_BC, SWI_EREG );


  /* extract ZResponse */
  for( ubIndex = 0u; ubIndex < 16u; ubIndex++ )
  {
      Swi_SendRawWordNoIrq(SWI_ERA, (((0x0030u + ubIndex) >> 8u) & 0xFFu) );
      Swi_SendRawWordNoIrq(SWI_RRA, ( (0x0030u + ubIndex)        & 0xFFu) );

    /* read out data */
    if( Swi_ReadActualSpace( &ubData ) == FALSE )
    {
      return FALSE;
    }

    /* store data properly */
    ubWordIndex = (ubIndex >> 1u);
    if( (ubIndex & 1u) == 1u )
    {
      gf2n_ZResponse[ubWordIndex] |= ((UWORD)ubData << 8u);
    }
    else
    {
      gf2n_ZResponse[ubWordIndex] = ((UWORD)ubData & 0xFFu);
    }
  }

  /* extract XResponse */


  for( ubIndex = 0u; ubIndex < 8u; ubIndex++ )
  {
	Swi_SendRawWordNoIrq(SWI_ERA, ((0x0010u+ubIndex) >> 8u)& 0xFFu );
  	Swi_SendRawWordNoIrq(SWI_RRA, ((0x0010u+ubIndex)       & 0xFFu));

    // read out data
    if( Swi_ReadActualSpace( &ubData ) == FALSE )
    {
      return FALSE;
    }

    /* store data properly */
    ubWordIndex = (ubIndex >> 1u);
    if( (ubIndex & 1u) == 1u )
    {
      gf2n_XResponse[ubWordIndex] |= ((UWORD)ubData << 8u);
    }
    else
    {
      gf2n_XResponse[ubWordIndex] = ((UWORD)ubData & 0xFFu);
    }
  }
  /* only lower 8 bytes of XResponse needed */
  gf2n_XResponse[4] = 0u;
  gf2n_XResponse[5] = 0u;
  gf2n_XResponse[6] = 0u;
  gf2n_XResponse[7] = 0u;
  gf2n_XResponse[8] = 0u;


  /* read remaining ZResponse bits */
  /* use High level function to switch back to RBL0 and set new address */
  if( Swi_ReadRegisterSpace( 0x0330u, &ubData ) == FALSE )
  {
    return FALSE;
  }
  gf2n_ZResponse[8] = ((UWORD)ubData & 0xFFu );


  /* all done correct. */
  return TRUE;

}

/* ****************************************************************************
   name:      Ecc_VerifyResponse()
  
   function:  check, if response to sent challenge was valid.
  
   input:     IN: gf2n_XResponse
                gf2n_t array holding the x part of the ORIGA response.  
              IN: gf2n_ZResponse  
                gf2n_t array holding the z part of the ORIGA response.  
              IN: gf2n_CheckValue
                gf2n_t array holding checkvalue.
   output:    bool
  
   return:    true, if authentication is valid.
              false, if authentication failed.
  
   date:      .
   ************************************************************************* */
BOOL Ecc_VerifyResponse( gf2n_t gf2n_XResponse, gf2n_t gf2n_ZResponse, gf2n_t gf2n_CheckValue )
{

  UWORD uwResult;
  gf2n_t gf2n_AV;
  
      
  /* z must not be zero */
  if( gf2n_iszero(gf2n_ZResponse) == GF2N_RETURN_TRUE )
  {
    return FALSE;
  }
      
  /* do last calculation step and check values */
  gf2n_mul( gf2n_AV, gf2n_CheckValue, gf2n_ZResponse);
  uwResult  = (gf2n_XResponse[0] ^ gf2n_AV[0]);
  uwResult |= (gf2n_XResponse[1] ^ gf2n_AV[1]);
  uwResult |= (gf2n_XResponse[2] ^ gf2n_AV[2]);
  uwResult |= (gf2n_XResponse[3] ^ gf2n_AV[3]);

  /* check state of authentication */
  if( uwResult == 0 )
  {
    return TRUE;
  }
  
  /* authentication failed */
  return FALSE;

}

#endif

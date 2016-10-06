/**
 @file     ADF4351.c
 @date     22 Sept 2016
 @brief    ADF4351 driver
 * Calculate register values for ADF4351 given:
 *    RFout:    Required output frequency in Hz
 *    REFin:    Reference clock in Hz
 *    Spacing:  Output channel spacing in Hz

 @par Copyright &copy; 2015 Mike Gore, GPL License
 @par You are free to use this code under the terms of GPL
  please retain a copy of this notice in any code you use it in.

  This is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option)
  any later version.
  
  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* define ADF4351 global data */
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "adf4351_hal.h"
#include "adf4351.h"

// ADF4351_DEBUG
// 1 = errors
// 2 = calculations
// 4 = register dumps included

/// @brief ADF4351 register buffer
adf4351_regs_t regs;
/// =============================================================
/// =============================================================

/// @brief Sync settings to ADF4351 registers
/// @param[in] all: sync all registers
MEMSPACE
void ADF4351_sync(int all)
{
    int i;
	uint32_t value;
    for (i=5; i>= 0; --i)
	{
		value = ADF4351_GetReg32(i);
		(void) ADF4351_spi_txrx(value);
	}
#if ADF4351_DEBUG & 4
	ADF4351_dump_registers();
#endif
}
/// =============================================================
/// =============================================================


/**
 *  \brief Returns 32bit register value from register buffer
 *
 * @param  addr: 								Register address
 * @retval register value
 */
MEMSPACE
uint32_t ADF4351_GetReg32(int addr)
{
	uint32_t R;
    switch (addr)
    {
        case 0 : 
    		R =  0UL
				| ((regs.r0.FRAC           & 0x0fffUL ) <<  3 )
				| ((regs.r0.INT            & 0xffffUL ) << 15 );
			return(R );
        case 1 : 
    		R =  1UL
				| (( regs.r1.MOD           & 0x0fffUL ) <<  3 )
				| (( regs.r1.Phase         & 0x0fffUL ) << 15 )
				| (( regs.r1.Prescaler     &      1UL ) << 27 )
				| (( regs.r1.PhaseAdjust   &      1UL ) << 28 );
			return(R );
        case 2 : 
    		R =  2UL
				| (( regs.r2.Reset_R_N     &      1UL ) <<  3 )
				| (( regs.r2.CP3S          &      1UL ) <<  4 )
				| (( regs.r2.PowerDown     &      1UL ) <<  5 )
				| (( regs.r2.PhasePolarity &      1UL ) <<  6 )
				| (( regs.r2.LDP           &      1UL ) <<  7 )
				| (( regs.r2.LDF           &      1UL ) <<  8 )
				| (( regs.r2.CPC           &    0xfUL ) <<  9 )
				| (( regs.r2.DBufRFDiv     &      1UL ) << 13 )
				| (( regs.r2.R             &  0x3ffUL ) << 14 )
				| (( regs.r2.REFinDIV2     &      1UL ) << 24 )
				| (( regs.r2.REFinMUL2     &      1UL ) << 25 )
				| (( regs.r2.MuxOut        &      7UL ) << 26 )
				| (( regs.r2.NoiseSpurMode &      3UL ) << 29 );
			return(R );
        case 3 : 
    		R =  3UL
				| (( regs.r3.ClkDiv        & 0x0fffUL ) <<  3 )
				| (( regs.r3.ClkDivMode    &      3UL ) << 15 )
				| (( regs.r3.CSR           &      1UL ) << 18 )
				| (( regs.r3.CPC           &      1UL ) << 21 )
				| (( regs.r3.ABPW          &      1UL ) << 22 )
				| (( regs.r3.BandClkMode   &      1UL ) << 23 );
			return(R );
        case 4 : 
    		R =  4UL
				| (( regs.r4.RFOutPower    &      3UL ) <<  3 )
				| (( regs.r4.RFOutEnable   &      1UL ) <<  5 )
				| (( regs.r4.AuxOutPower   &      3UL ) <<  6 )
				| (( regs.r4.AuxOutEnable  &      1UL ) <<  8 )
				| (( regs.r4.AuxOutSel     &      1UL ) <<  9 )
				| (( regs.r4.MTLD          &      1UL ) << 10 )
				| (( regs.r4.VCOPowerDown  &      1UL ) << 11 )
				| (( regs.r4.BandClkDiv    &   0xffUL ) << 12 )
				| (( regs.r4.RFDivSel      &      7UL ) << 20 )
				| (( regs.r4.FeedbackVCO   &      1UL ) << 23 );
			return(R );
        case 5 : 
    		R =  5UL
				| (( 3UL )                              << 19 )	// reserved high
				| (( regs.r5.LDPinMode     &      3UL ) << 22 );
			return(R);
    }
    return 0xffffffffU;
}


/**
 *  \brief Convert charge pump current in uA to R2 register value
 *
 * @param  none
 */
MEMSPACE
int ADF4351_CHARGE_PUMP_uA(int32_t x)
{
	int32_t val = (((x)-312) / 312);
	if(val < 0)
		val = 0;
	if(val > 0x0f)
	{
		val = 0x0f;
	}
	return(val);
}

/**
 *  \brief Init all ADF4351 registers to default values
 *
 * @param  none
 */
MEMSPACE
void ADF4351_Init(void)
{
    ADF4351_spi_init();

// Register Address bits
    regs.r0.ControlBits = 0U;
    regs.r0.FRAC = 0;
    regs.r0.INT = 0;
    regs.r0.res0L = 0;

    regs.r1.ControlBits   = 1U;
    regs.r1.MOD           = 2;	// minimum value is 2
    regs.r1.Phase         = 1;
    regs.r1.Prescaler     = ADF4351_PRESCALER_4_5;
    regs.r1.PhaseAdjust   = ADF4351_DISABLE;
    regs.r1.res0L = 0;

    regs.r2.ControlBits   = 2U;
    regs.r2.Reset_R_N     = ADF4351_DISABLE;
    regs.r2.CP3S          = ADF4351_DISABLE;
    regs.r2.PowerDown     = ADF4351_DISABLE;
    regs.r2.PhasePolarity = ADF4351_POLARITY_POS;

// INT-N mode [LDP:LDF]  = 11 integer-N
// regs.r2.LDP           = ADF4351_LDP_6NS;
// regs.r2.LDF           = ADF4351_LDF_INT_N; // 1
// FRAC-N mode [LDP:LDF] = 00 fractional-N

    regs.r2.LDP           = ADF4351_LDP_10NS;		// fractional-N
    regs.r2.LDF           = ADF4351_LDF_FRAC_N;	// fractional-N
    regs.r2.CPC           = ADF4351_CHARGE_PUMP_uA(2500);
    regs.r2.DBufRFDiv     = ADF4351_DISABLE;
    regs.r2.R             = 1;
    regs.r2.REFinDIV2     = ADF4351_DISABLE;	// used for cycle slip mode
    regs.r2.REFinMUL2     = ADF4351_DISABLE;
    regs.r2.MuxOut        = ADF4351_MUX_THREESTATE;	/* enable ONLY for reads */
    regs.r2.NoiseSpurMode = ADF4351_LOW_NOISE_MODE;
	regs.r2.res0L         = 0;

    regs.r3.ControlBits   = 3U;
    regs.r3.ClkDiv        = 150;
    regs.r3.ClkDivMode    = 0;	
    regs.r3.res0L         = 0;
    regs.r3.CSR           = ADF4351_DISABLE;	// disable cycle slip
    regs.r3.res1L         = 0;
	// CPC 0 for fractional-N, 1 for integer-N
    regs.r3.CPC           = ADF4351_DISABLE;  // fractional-N, ENABLE integer-N
    regs.r3.ABPW          = ADF4351_ABP_6NS; // fractional N, 3ns integer-N
    regs.r3.BandClkMode   = 0; // for PFD < 125kHz
    regs.r3.res2L         = 0;

    regs.r4.ControlBits   = 4U;
    regs.r4.RFOutPower    = ADF4351_POWER_N4DB;
    regs.r4.RFOutEnable   = ADF4351_ENABLE;
    regs.r4.AuxOutPower   = ADF4351_POWER_N4DB;
    regs.r4.AuxOutEnable  = ADF4351_ENABLE;
    regs.r4.AuxOutSel     = ADF4351_AUXOUT_FROM_RF_DIVIDERS;
    regs.r4.MTLD          = ADF4351_DISABLE;
    regs.r4.VCOPowerDown  =  ADF4351_DISABLE;
    regs.r4.BandClkDiv    = 200;	/* minimum value  is 1 */
    regs.r4.RFDivSel      = 0; 	/* by 1 */
    regs.r4.FeedbackVCO   = 1;
    regs.r4.res0L         = 0;

    regs.r5.ControlBits   = 5U;
    regs.r5.res0L         = 0;
    regs.r5.res1H         = 3;
    regs.r5.res2L         = 0;
    regs.r5.LDPinMode     = ADF4351_LD_PIN_DIGITAL_LOCK;
    regs.r5.res3L         = 0;

    ADF4351_sync(1);
}

#if ADF4351_DEBUG & 4

/**
 *  \brief Display ADF4351 registers in detail
 * @param  none
 */
MEMSPACE
void ADF4351_dump_registers(void)
{
    printf("R0:\n");
    printf("     FRAC         = %d\n", regs.r0.FRAC);
    printf("     INT          = %d\n", regs.r0.INT);

    printf("R1:\n");
    printf("    MOD           = %d\n", regs.r1.MOD);
    printf("    Phase         = %d\n", regs.r1.Phase);
    printf("    Prescaler     = %d\n", regs.r1.Prescaler);
    printf("    PhaseAdjust   = %d\n", regs.r1.PhaseAdjust);
    
    printf("R2:\n");
    printf("    Reset_R_N     = %d\n", regs.r2.Reset_R_N);
    printf("    CP3S          = %d\n", regs.r2.CP3S);
    printf("    PowerDown     = %d\n", regs.r2.PowerDown);
    printf("    PhasePolarity = %d\n", regs.r2.PhasePolarity);
    printf("    LDP           = %d\n", regs.r2.LDP);
    printf("    LDF           = %d\n", regs.r2.LDF);
    printf("    CPC           = %d\n", regs.r2.CPC);
    printf("    DBufRFDiv     = %d\n", regs.r2.DBufRFDiv);
    printf("    R             = %d\n", regs.r2.R);
    printf("    REFinDIV2     = %d\n", regs.r2.REFinDIV2);
    printf("    REFinMUL2     = %d\n", regs.r2.REFinMUL2);
    printf("    MuxOut        = %d\n", regs.r2.MuxOut);
    printf("    NoiseSpurMode = %d\n", regs.r2.NoiseSpurMode);
    
    printf("R3:\n");
    printf("    ClkDiv        = %d\n", regs.r3.ClkDiv);
    printf("    ClkDivMode    = %d\n", regs.r3.ClkDivMode);
    printf("    CSR           = %d\n", regs.r3.CSR);
    printf("    CPC           = %d\n", regs.r3.CPC);
    printf("    ABPW          = %d\n", regs.r3.ABPW);
    printf("    BandClkMode   = %d\n", regs.r3.BandClkMode);

    printf("R4:\n");
    printf("    RFOutPower    = %d\n", regs.r4.RFOutPower);
    printf("    RFOutEnable   = %d\n", regs.r4.RFOutEnable);
    printf("    AuxOutPower   = %d\n", regs.r4.AuxOutPower);
    printf("    AuxOutEnable  = %d\n", regs.r4.AuxOutEnable);
    printf("    AuxOutSel     = %d\n", regs.r4.AuxOutSel);
    printf("    MTLD          = %d\n", regs.r4.MTLD);
    printf("    VCOPowerDown  = %d\n", regs.r4.VCOPowerDown);
    printf("    BandClkDiv    = %d\n", regs.r4.BandClkDiv);
    printf("    RFDivSel      = %d\n", regs.r4.RFDivSel);
    printf("    FeedbackVCO   = %d\n", regs.r4.FeedbackVCO);
   
    printf("R5:\n");
    printf("    LDPinMode     = %d\n", regs.r5.LDPinMode);
	printf("\n");
}
#endif

/**
 *  \brief Read ADF4351 status
 *  Only of value if spi bus input is tied to mux out
 *  This function has not been tested!
 * @param  mode: muxout mode
 * @return 32bit result
 */
MEMSPACE
uint32_t ADF4351_status( uint8_t mode)
{
    uint32_t ret;

    regs.r2.MuxOut = mode & 7;
    ADF4351_sync(0);

// should read the status value based on mode
    ret = ADF4351_spi_txrx(ADF4351_GetReg32(2));

// tristate bus
    regs.r2.MuxOut = ADF4351_MUX_THREESTATE;
    ADF4351_sync(0);

    return(ret);
}


/**
 *  \brief Return the GCD of two unsigned 32bit numbers
 * @param  u: first number 
 * @param  v: second number 
 * @return GCD of u and v
 */
MEMSPACE
uint32_t ADF4351_GCD32(uint32_t u, uint32_t v)
{
    if (u == 0)
        return v;
    if (v == 0)
        return u;

    if (u > v)
        return ADF4351_GCD32(u % v, v);
    else
        return ADF4351_GCD32(u, v % u);
}


/**
 *  \brief Compute PFD
 * @param  REFin: Reference in frequency
 * @param  R: Reference divider
 */
MEMSPACE
// PFD = REFin × [(1 + REfinMUL2)/(R × (1 + REFinDIV2))]
//  REFin is the reference frequency input.
//  REFinMUL2 is the REFin doubler bit (0 or 1).
//  R is the RF reference division factor (1 to 1023).
//  REFinDIV2 is the reference divide-by-2 bit (0 or 1).
MEMSPACE
double ADF4351_PFD(double REFin, int R)
{
	double PFD = (uint32_t) REFin
		* (regs.r2.REFinMUL2 ? 2 : 1)
		/ (R * (regs.r2.REFinDIV2 ? 2 : 1) );
	return(PFD);
}

/** \brief Verbose Error messages
*/
#if ADF4351_DEBUG & 1
adf4351_err_t adf4351_errors[] =
{
   { ADF4351_NOERROR,"No error" },
   { ADF4351_RFout_RANGE,"RFout_RANGE" },
   { ADF4351_RFoutDIV_RANGE,"RFoutDIV_RANGE" },
   { ADF4351_RFout_MISMATCH,"RFout_MISMATCH" },
   { ADF4351_REFin_RANGE,"REFin_RANGE" },
   { ADF4351_BandSelectClockFrequency_RANGE,"BandSelectClockFrequency_RANGE" },
   { ADF4351_R_RANGE,"R_RANGE" },
   { ADF4351_PFD_RANGE,"PFD_RANGE" },
   { ADF4351_N_RANGE,"N_RANGE" },
   { ADF4351_INT_RANGE,"INT_RANGE" },
   { ADF4351_MOD_RANGE,"MOD_RANGE" },
   { ADF4351_FRAC_RANGE,"FRAC_RANGE"},
   { ADF4351_ERROR_END,NULL}
};
#endif

/**
 *  \brief Display ADF4351 error return code
 * @param  error: error code
 */
MEMSPACE
void ADF4351_display_error(int error)
{
	int i;
	int found = 0;
#if ADF4351_DEBUG & 1
	for(i=0;i<ADF4351_ERROR_END;++i)
	{
		if( adf4351_errors[i].val == error )
		{
			printf("ADF4351 error %s\n", adf4351_errors[i].msg);
			found = 1;
		}
	}
	if(!found)
	{
		printf("ADF4351 missing error message:[%d]\n", error);
	}
#else
	printf("ADF4351 error:[%d]\n", error);
#endif
}

/**
 *  \brief Calculate register values for ADF4351
 * @param  RFout: 	Required output frequency in Hz
 * @param  REFin:	Reference clock in Hz
 * @param  Spacing:	Output channel spacing in Hz
 * @paramOut  RFoutCalc: Calculated actual output frequency in Hz
 * @retval 0=OK, or Error code 
 *
 */
MEMSPACE
int ADF4351_Config(double RFout, double REFin, double ChannelSpacing, double *RFoutCalc )
{

	uint32_t	div_gcd;
    uint32_t 	r0_INT;
    uint32_t 	r0_FRAC;
    uint32_t 	r1_MOD;
	uint32_t    r1_Prescaler;
	uint32_t	r2_R;
    uint32_t 	r4_RFDivSel;
	uint32_t 	r4_BandClkDiv;
    uint32_t    RFoutDIV;	
	double 		N;
	uint32_t    N_min;

	double 		PFD;
    double 		BandSelectClockFrequency;
	double      Fres;

	double 		dscale;
	uint32_t 	temp;

	*RFoutCalc = 0.0;

/**
 * RFoutVCO = [r0_INT + (r0_FRAC/r1_MOD)] × (PFD)
 * RFout = [r0_INT + (r0_FRAC/r1_MOD)] × (PFD /RFoutDIV)
 *   RFout is the RF frequency output.
 *   r0_INT is the integer division factor.
 *   r0_FRAC is the numerator of the fractional division (0 to MOD − 1).
 *   r1_MOD is the preset fractional modulus (2 to 4095).
 *   RFoutDIV is the VCO output divider.
 * PFD = REFin × [(1 + REFinMUL2)/(R × (1 + REFinDIV2))]
 *   REFin is the reference frequency input.
 *   REFinMUL2 is the REFin doubler bit (0 or 1).
 *   r2_R is REFin division factor (1 to 1023).
 *   REFinDIV2 is the reference divide-by-2 bit (0 or 1).
 * Fres = ChannelSpacing * RFoutDIV
 * r1_MOD=REFin / Fres
*/

	// ==========================
    if (RFout > ADF4351_RFOUT_MAX)
	{
#if ADF4351_DEBUG & 1
		printf("RFout > %.2f\n", (double) ADF4351_RFOUT_MAX);
#endif
		return(ADF4351_RFout_RANGE);
	}

    if (RFout < ADF4351_RFOUT_MIN)
	{
#if ADF4351_DEBUG & 1
		printf("RFout < %.2f\n", (double) ADF4351_RFOUT_MIN);
#endif
		return(ADF4351_RFout_RANGE);
	}

    if (REFin > ADF4351_REFIN_MAX)
	{
#if ADF4351_DEBUG & 1
		printf("REFin > %.2f\n", (double) ADF4351_REFIN_MAX);
#endif
		return(ADF4351_REFin_RANGE);
	}

	// ==========================
    // Compute RFout divider and R4 register value
	r4_RFDivSel = 0;
	RFoutDIV = 1;
	while(((RFout * RFoutDIV) < ADF4351_VCO_MIN)  && RFoutDIV < 64)
	{
		RFoutDIV <<= 1;
		r4_RFDivSel++;
	}

	// Compute r1_prescale selector based on RFout
	if(RFout > ADF4351_MAX_FREQ_45PRE)
	{
		r1_Prescaler = 1;
		N_min = 75;
	}
	else
	{
		r1_Prescaler = 0;
		N_min = 23;
	}


	// ==========================
	// PFD = REFin × [(1 + REFinMUL2)/(R × (1 + REFinDIV2))]
	//  REFin is the reference frequency input.
	//  REFinMUL2 is the REFin doubler bit (0 or 1).
	//  r2_R is REFin division factor (1 to 1023).
	//  REFinDIV2 is the reference divide-by-2 bit (0 or 1).
	r2_R = 1;
	while(r2_R < 4096 )
	{
		PFD = ADF4351_PFD(REFin, r2_R);
		if(PFD < ADF4351_PFD_MAX)
			break;
		r2_R++;
	}

	if(r2_R == 4096)
	{
#if ADF4351_DEBUG & 1
        printf("r2_R == 4096\n");
#endif
        return(ADF4351_R_RANGE);
	}

	// ==========================
	// Compute N based on R4 feedback path select
    if (regs.r4.FeedbackVCO)
        N = ((double)RFout * (double)RFoutDIV) / (double) PFD;
    else
        N = ((double)RFout / (double) PFD);

	if(N < N_min || N > 65535U )
	{
#if ADF4351_DEBUG & 1
		printf("N %.2f out of range\n", (double) N);
#endif
		return(ADF4351_N_RANGE);
	}

	// ==========================
	// Integer
	r0_INT = (uint32_t) N;

	// r0_INt range check
    if (r0_INT > 65535U)
    {
#if ADF4351_DEBUG & 1
        printf("INT: %lu\n", (unsigned long) r0_INT);
#endif
		return(ADF4351_INT_RANGE);
    }

	// ==========================
	// Modulus
	// FIXME the AD windows driver Main_Form.cs disagrees with their own datasheet
	// Specifically the RF output divider is not included in the calculation
	// Datasheet:
	// Fres is the VCO Channel Spacing
 	//   r1_MOD=REFin/Fres
 	//   Fres = ChannelSpacing/RFoutDIV
	//   r1_MOD=(uint32_t) round((double) REFin / (double) Fres);

	// The tested working solution found in the AD driver Main_Form.cs
	r1_MOD=(uint32_t) round((double) PFD / (double) ChannelSpacing );

	// ==========================
	// Fractional
	r0_FRAC = (uint32_t)round( ((double)N-(double)r0_INT)*(double) r1_MOD);

	// ==========================
	// Reduce r1_MOD and r0_FRAC by greatest common divisor

	div_gcd = ADF4351_GCD32(r1_MOD, r0_FRAC);
	r1_MOD /= div_gcd;
	r0_FRAC /= div_gcd;

	// FIXME - why is this needed ?
	if (r1_MOD == 1)
		r1_MOD = 2;

	// ==========================
	// r1_MOD Range check
	if(r1_MOD == 0 || r1_MOD > 4095U) 
	{
#if ADF4351_DEBUG & 1
        printf("*MOD: %lu, INT: %lu, FRAC: %lu\n", (unsigned long) r1_MOD, (unsigned long) r0_INT, (unsigned long) r0_FRAC);
#endif
		return(ADF4351_MOD_RANGE);
	}
	
	// ==========================
	// r0_FRAC range check
    if (r0_FRAC > 4095U)
    {
#if ADF4351_DEBUG & 1
        printf("MOD: %lu, INT: %lu, *FRAC: %lu\n", (unsigned long) r1_MOD, (unsigned long) r0_INT, (unsigned long) r0_FRAC);
#endif
		return(ADF4351_FRAC_RANGE);
    }

	// ==========================
	// Check for PFD range errors
	if (regs.r3.BandClkMode == 0)
    {
		if( PFD > ADF4351_PFD_MAX )
		{
#if ADF4351_DEBUG & 1
			printf("PFD: %.2f > %lu && BandClkMode == 0\n", 
				(double) PFD, (unsigned long) ADF4351_PFD_MAX);
#endif
			return(ADF4351_PFD_RANGE);
		}
    }
    else 
    {
		if( PFD > ADF4351_PFD_MAX && r0_FRAC != 0)
		{
#if ADF4351_DEBUG & 1
			printf("PFD: %.2f > %lu && BandClkMode == 0\n", 
				(double) PFD, (unsigned long) ADF4351_PFD_MAX);
#endif
			return(ADF4351_PFD_RANGE);
		}
		if (PFD > 90 && r0_FRAC != 0)
		{
#if ADF4351_DEBUG & 1
			printf("PFD: %.2f > 90 Band Clock Mode && r0_FRAC != 0\n", 	
				(double) PFD);
#endif
			return(ADF4351_PFD_RANGE);
		}
	}

	// ==========================
	// Band Clock Divider 
	// FIXME Add user override to pick value
    // FIXME perhaps we could consider setting regs.r3.BandClkMode based on PFD ?
	if (regs.r3.BandClkMode == 0)
		dscale = 8;
	else
		dscale = 2;
	temp = (uint32_t)round(dscale * PFD);
	if ((dscale * PFD - temp) > 0)
		temp++;

	temp = (temp > 255) ? 255 : temp;
	r4_BandClkDiv = temp;

	// ==========================
	// Band Clock Frequency
	// FYI temp will always be > 0
	BandSelectClockFrequency = (PFD / (double)r4_BandClkDiv);

	// ==========================
	// Band Clock Range Check 
	if (BandSelectClockFrequency > 500000.0)
    {
#if ADF4351_DEBUG & 1
        printf("Band Select Clock Frequency %.2f > 500000\n", 
			(double) BandSelectClockFrequency);
#endif
		return(ADF4351_BandSelectClockFrequency_RANGE);
    }

    if ((BandSelectClockFrequency > 125000.0) & (regs.r3.BandClkMode))
    {
#if ADF4351_DEBUG & 1
        printf("Band Select Clock Frequency %.2f > 125000 && regs.r3.BandClkMode\n", 
			(double) BandSelectClockFrequency);
#endif
		return(ADF4351_BandSelectClockFrequency_RANGE);
    }

	// ==========================
	// Noise Spur Mode
    if ((regs.r2.NoiseSpurMode == ADF4351_LOW_SPUR_MODE) && (r1_MOD < 50))
    {
#if ADF4351_DEBUG & 1
        printf("regs.r2.NoiseSpurMode == ADF4351_LOW_SPUR_MODE) && (r1_MOD(%lu) < 50\n",
		(unsigned long) r1_MOD);
#endif
		return(ADF4351_MOD_RANGE);
    }

	// ==========================
	// Write Registers
	regs.r0.INT = r0_INT;
    regs.r0.FRAC = r0_FRAC;
	regs.r1.MOD = r1_MOD;
	regs.r1.Prescaler = r1_Prescaler;

	regs.r2.R = r2_R;

	regs.r4.BandClkDiv = r4_BandClkDiv;
	regs.r4.RFDivSel = r4_RFDivSel;

	// ==========================
	// Compute actual RFout

    *RFoutCalc = (double) 
		( (double) r0_INT + ((double)r0_FRAC / (double)r1_MOD) ) 
		* ( (double)PFD / (double)(RFoutDIV) );

#if ADF4351_DEBUG & 2
    printf("RFout:     %.2f Hz\n", RFout);
    printf("RFoutCalc: %.2f Hz\n", *RFoutCalc);
    printf("RFin:      %.2f Hz\n", (double) REFin);
    printf("PFD:       %.2f Hz\n", (double) PFD);
	printf("RFoutDIV:  %d\n", (int) RFoutDIV);
    printf("  Channel Spacing: %.2f Hz\n", (double) ChannelSpacing);
    printf("  BandSelectClockFrequency: %.2f Hz\n", (double) BandSelectClockFrequency);
    printf("  VCO FeedbackVCO %s\n", regs.r4.FeedbackVCO ? "VCO" : "Divided" );
    printf("  N: %.2f Hz\n", (double) N);
	printf("  r0_INT: %lu, r0_FRAC: %lu, r1_MOD: %lu\n",
		(unsigned long) r0_INT, (unsigned long)r0_FRAC, (unsigned long) r1_MOD);
	printf("  r2_R:%lu\n", (unsigned long) r2_R);
	printf("  r1_Prescaler: %s\n", r1_Prescaler ? "8/9" : "4/5");
	printf("  r4_BandClkDiv %lu\n", (unsigned long) r4_BandClkDiv);
#endif

	// VCO frequency error ?
	if (*RFoutCalc != RFout)
    {
        return(ADF4351_RFout_MISMATCH);
    }

    return (0);
}


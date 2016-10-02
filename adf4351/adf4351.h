/**
 @file     ADF4351.h
 @brief    ADF4351 driver
 @date     22 Sept 2016

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


#ifndef _ADF4351_H_
#define _ADF4351_H_

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "adf4351_hal.h"

// Named address space
#ifndef MEMSPACE
#define MEMSPACE /**/
#endif

// Weak attribute
#ifndef WEAK_ATR
#define WEAK_ATR __attribute__((weak))
#endif

#define ADF4351_RFOUT_MAX      4400000000.0
#define ADF4351_RFOUT_MIN        34375000.0
#define ADF4351_VCO_MIN        2200000000.0
#define ADF4351_MAX_FREQ_45PRE 3000000000.0
#define ADF4351_PFD_MAX          32000000.0
#define ADF4351_REFIN_MAX       250000000.0
#define ADF4351_BANDSEL_MAX        125000

/** \brief  ADF4351 R0
 *
 * Integer Value (INT) 16bits: [DB30:DB15]) 
 * Integer part of the feedback division factor. 
 *
 * Fractional Value (FRAC) 12bits [DB14:DB3]
 * Numerator of the fraction input to Σ-Δ modulator. 
 * FRAC values from 0 to (MOD − 1) cover channels over a 
 * frequency range equal to the PFD reference frequency.
 *
 * RF OUT = PFD × (INT + (FRAC/MOD))
 */
typedef struct
{
	uint32_t ControlBits   :3;  /*!< bit:  0.. 2  control bits */
	uint32_t FRAC          :12; /*!< bit:  3..14  12bit FRACTIONAL value (FRAC) */
	uint32_t INT           :16; /*!< bit: 15..30  16bit INTEGER VALUE (INT) */
	uint32_t res0L     	   :1;  /*!< bit: 31      reserved */
} r0_t;

/** \brief  ADF4351 R1 
 * Phase Adjust bit [DB28]
 * [DB28] = 1, Adjust output 
 *   Do not perform VCO band selection or phase resync when R0 updated. 
 *   Only use when frequency adjustments will be under 1MHz!
 * [DB28] = 0, Perform VCO band selection and phase resync when R0 updated.
 *   (only if phase resync is enabled in R3 [DB16:DB15])
 *
 * Prescaler bit [DB27]
 * [DB27] = 1, prescaler = 8/9, min N is 75
 * [DB27] = 0, prescaler = 4/5, min N is 23, Max VCO = 3.6GHz
 *
 * Phase Value 12bits [DB26:DB15] 
 * RF output phase from 0° to 360° with resolution 360°/MOD
 * Must be < MOD 
 *
 * Mudulus (MOD) 12bits (Bits[DB14:DB3]) 
 * Ratio of the PFD frequency to the channel step resolution on RF out.
 */
typedef struct
{
	uint32_t ControlBits   :3;  /*!< bit:  0.. 2  Control bits */
	uint32_t MOD           :12; /*!< bit:  3..14  12bit modulus value */
	uint32_t Phase         :12; /*!< bit: 15..26  12bit phase value */
	uint32_t Prescaler     :1;  /*!< bit:  27     prescaler */
	uint32_t PhaseAdjust   :1;  /*!< bit:  28     phase adjust */
	uint32_t res0L         :3;  /*!< bit: 29..31  reserved */
} r1_t;

/** \brief  ADF4351 R2 structure
 * Noise Mode 2bits [DB30:DB29] (NoiseSpureMode)
 * The noise mode optimizes either improved spurious or improved phase noise.
 *
 * Low Spur Mode 
 * Used for fast-locking applications when the PLL closed-loop bandwidth 
 * is greater then 1/10 of RF OUT step resolution (f RES).
 * Dither is enabled. 
 * ( Dither improves spurious respose by randomizes the fractional 
 *    quantization noise to resembles white noise rather than spurious noise. )
 * Down side: Wide loop filter can not attenuate spurs as much as a 
 * narrow loop bandwidth.
 *
 * Low Noise Mode 
 * Used for best noise performance.
 * Dither is disabled. 
 * Pick narrow loop filter bandwidth to ensures low noise and attenuate spurs. 
 *
 * MUXOUT 3bits Bits[DB28:DB26] (MufOut)
 * Note N counter output must be disabled for VCO band selection to operate.
 *
 * REFin doubler [DB25] (REFinM2)
 * [DB25] = 1 enable doubler, REFin maximum is 30MHz
 * [DB25] = 0 disables doubler
 * 
 * Reference divide by 2 (REFinD2)
 * [DB24] = 1 enable divider
 * [DB24] = 0 disable divider
 *
 * R Counter [DB23:DB14] (R)
 *   Must be between 1 and 1023
 *   Divides REFin to provide PFD.
 *
 * Double Buffer RF divider [DB13] (DBufRFDiv)
 * [DB13] = 1 enables double buffering of RF Divider [DB22:DB20] in R4
 *
 * Charge Pump Current [DB12:DB9] (CPC)
 * Set the charge pump current to match your loop filter design.
 * 
 * Lock Detect Function [DB8] (LDF)
 * Examine 40 or 5 PFD cycles to ascertain if lock achieved. 
 * [DB8] = 0, examine 40 PFD cycles
 *            Use this for fractional-N mode.
 * [DB8] = 1, examine 5 PFD cycles. 
 *            Use this integer-N mode.
 * [DB8] = 1, the number of PFD cycles monitored is 5. 
 *
 * Lock Detect Precision [DB7] (LDP)
 * [DB7] = 0 comparison window of lock detect circuit is 6ns.
 * [DB7] = 1 comparison window of lock detect circuit is 10ns.
 * Lock Detect circuit goes high when consecutive PFD
 * cycles are less than either 40 or 5 as set by [DB8] (LDF)
 * So if [DB8:DB7] = 00 then 40 consecutive PFD cycles of 10 ns 
 * or less must occur before digital lock detect goes high.
 * [DB8:DB7] = 00, use this for fractional-N 
 * [DB8:DB7] = 11, use this for integer-N 
 *
 * Phase Detector Polarity [DB6]
 * The DB6 bit sets the phase detector polarity. 
 * [DB6] = 1, use this if you have a passive loop filter or 
 *   noninverting active loop filter.
 * [DB6] = 0, use this if you have an an inverting active filter.
 *
 * Power Down [DB5] (PowerDown)
 * [DB5] = 1 
 *   Synthesizer counters forced their load state conditions.
 *   VCO is powered down.
 *   Charge pump is forced into three-state mode.
 *   Digital lock detect circuitry is reset.
 *   RF OUT buffers are disabled.
 *
 * Charge Pump Three State [DB4] (CP3S)
 * [DB4] = three-state mode
 * [DB4] = 0 for normal operation.
 * 
 * Counter Reset [DB3] (Reset_R_N)
 * [DB3] = 1 reset R counter and N counter
 * [DB3] = 0 for normal operation.
 */
typedef struct
{
	uint32_t ControlBits   :3;  /*!< bit:  0.. 2  Control bits */
	uint32_t Reset_R_N     :1;  /*!< bit:  3      R and N Reset */
	uint32_t CP3S          :1;  /*!< bit:  4      Charge Pump 3 state */
	uint32_t PowerDown     :1;  /*!< bit:  5      Power Down */
	uint32_t PhasePolarity :1;  /*!< bit:  6      Phase Detector Polarity */
	uint32_t LDP           :1;  /*!< bit:  7      Lock Detect Precision */
	uint32_t LDF           :1;  /*!< bit:  8      Lock Detect Function */
	uint32_t CPC           :4;  /*!< bit:  9..12  Charge Pump Current */
	uint32_t DBufRFDiv     :1;  /*!< bit:  13     Double Buffer RF divider */
	uint32_t R             :10; /*!< bit: 14..23  10bit R Counter */
	uint32_t REFinDIV2     :1;  /*!< bit: 24      REFin / 2 */
	uint32_t REFinMUL2     :1;  /*!< bit: 25      REFin * 2 */
	uint32_t MuxOut        :3;  /*!< bit: 26..28  MUXOUT */
	uint32_t NoiseSpurMode :2;  /*!< bit: 29..30  Low Noise and Low Spur Modes  */
	uint32_t res0L         :1;  /*!< bit: 31      reserved */
} r2_t;

/** \brief  ADF4351 R3 structure
 * Band Select Clock Mode [DB23] (BSCM)
 * [DB23] = 1, use if you need fast band selection
 *   Suitable for PFD frequencies > 125kHz.
 *   Keep band select clock divider <= 254.
 * [DB23] = 0, use if PFD frequencies are < 125kHz.
 *
 * Antibacklash Pulse Width [DB22] (ABPW)
 * [DB22] = 0, PFD antibacklash pulse width = 6ns
 *             Use for fraction-N mode.
 * [DB22] = 1, PFD antibacklash pulse width = 3ns 
 *             Use for integer-N mode.
 *             Gives better noise and spur values.
 *
 * Charge Pump Cancelation [DB21] (CPC)
 * [DB21] = 0, use this when you choose fraction-N mode
 * [DB21] = 1, use this when you choose integer-N mode
 *
 * Cycle Slip Reduction [DB18] (CSR)
 * [DB18] = 1, enable cycle slip reduction
 *             Gives fater lock times
 *             PFD MUST be 50% to work
 *             Charge Pump Current must be minimum
 *
 * Clock Divider Mode [DB16:DB15] (ClkDivMode)
 * [DB16:DB15] = 10, activate phase resync.
 * [DB16:DB15] = 01, activate faster lock.
 * [DB16:DB15] = 00, disable Clock Divider
 *
 * Clock Divider 12bits [DB14:DB3] (ClkDiv)
 * Timeout counter for activation of phase resync also sets
 * timeout for fast lock.
 */
typedef struct
{
	uint32_t ControlBits   :3;  /*!< bit:  0.. 2   control bits */
	uint32_t ClkDiv        :12; /*!< bit:  3..14   12bit Clock Divider */
	uint32_t ClkDivMode    :2;  /*!< bit:  15..16  Clock Divider Mode */
	uint32_t res0L         :1;  /*!< bit:  17      reserved */
	uint32_t CSR           :1;  /*!< bit:  18      CSR Enable */
	uint32_t res1L         :2;  /*!< bit:  19..20  reserved */
	uint32_t CPC           :1;  /*!< bit:  21      Charge Cancelation */
	uint32_t ABPW          :1;  /*!< bit:  22      Antibacklash Pulse Width */
	uint32_t BandClkMode   :1;  /*!< bit:  23      Band Select Clock Mode */
	uint32_t res2L         :8;  /*!< bit:  24..31  reserved */
} r3_t;

/** \brief  ADF4351 R4 structure
* 
* Feedback Select [DB23] (FeedbackVCO)
* [DB23] = 1, feedback is directly from VCO to N counters
 * [DB23] = 0, feedback is from output of VCO RF dividers to N counters.
 *
 * RF Divider Select [DB22:20] (RFDivSel)
 *
 * Band Select Clock Divider Value [DB19:12] (BandClkDiv)
 * [DB19:12] Divide output of R counter for band select logic
 * Use if R > 125kHz
 * 
 * VCO Power Down [DB11] (VCOPowerDown)
 * [DB11] = 1 enables VCO Power Down
 * [DB11] = 0 normal operation
 *
 * Mute Till Lock Detect [DB10] (MTLD)
 * [DB10] = 1 Power Down RFout section until lock achived.
 * [DB10] = 0 normal operation
 *
 * AUX Output Select [DB9] (AUXOutSel)
 * [DB9] = 1 Auxiliary RF is VCO frequency
 * [DB9] = 0 Auxiliary RF is RF dividers
 *
 * AUX Output Enable [DB8] (AUXOutEnable)
 * [DB8] = 1 Enable AUX RF out
 * [DB8] = 0 Disable AUX RF out
 *
 * AUX Output Power [DB7:DB6] (AUXOutPower)
 * [DB8:DB7] = AUX RF out level
 *
 * RF Output Enable [DB5] (RFOutEnable)
 * [DB5] = 1 Enable RF out
 * [DB5] = 0 Disable RF out
 *
 * RF Output Power [DB4:DB3] (RFOutPower)
 * [DB4:DB3] = RF out level
 *
 */
typedef struct
{
	uint32_t ControlBits   :3;  /*!< bit:  0.. 2  control bits */
	uint32_t RFOutPower    :2;  /*!< bit:  3.. 4  Output Power */
	uint32_t RFOutEnable   :1;  /*!< bit:  5      RF Output Enable */
	uint32_t AuxOutPower   :2;  /*!< bit:  6.. 7  AUX Output Power */
	uint32_t AuxOutEnable  :1;  /*!< bit:  8      AUX Output Enable */
	uint32_t AuxOutSel     :1;  /*!< bit:  9      AUX Output Select */
	uint32_t MTLD          :1;  /*!< bit: 10      Mute Till Lock Detect (MTLD) */
	uint32_t VCOPowerDown  :1;  /*!< bit: 11      VCO Power-Down */
	uint32_t BandClkDiv    :8;  /*!< bit: 12..19  Band Select Clock Divider Val */
	uint32_t RFDivSel      :3;  /*!< bit: 20..22  RF Divider Select */
	uint32_t FeedbackVCO   :1;  /*!< bit: 23      VCO Feedback Direct or Divided */
	uint32_t res0L         :8;  /*!< bit: 24..31  reserved */
} r4_t;

/** \brief  ADF4351 R5 structure
* Lock Detect Pin Operation [DB23:22] (LDPinMode)
 * [DB23:22] 00 = LOW
 * [DB23:22] 01 = Digital Lock Detect
 * [DB23:22] 10 = LOW
 * [DB23:22] 11 = HIGH
 */
typedef struct
{
	uint32_t ControlBits   :3;  /*!< bit:  0.. 2  control bits */
	uint32_t res0L         :16; /*!< bit:  3..18  reserved */
	uint32_t res1H         :2;  /*!< bit: 19..20  reserved */
	uint32_t res2L         :1;  /*!< bit: 21      reserved */
	uint32_t LDPinMode     :2;  /*!< bit: 22..23  LD Pin Mode */
	uint32_t res3L         :8;  /*!< bit: 24..31  reserved */
} r5_t;

/** \brief ADF4351 all registers type
*/
typedef struct {
	r0_t r0;
	r1_t r1;
	r2_t r2;
	r3_t r3;
	r4_t r4;
	r5_t r5;
} adf4351_regs_t;

/** \brief ADF4351 Error structure
*/
typedef struct
{
	int val;
	char *msg;
} adf4351_err_t;

/** \brief Error numbers
*/
enum
{
    ADF4351_NOERROR,
    ADF4351_RFout_RANGE,
    ADF4351_RFoutDIV_RANGE,
    ADF4351_RFout_MISMATCH,
    ADF4351_REFin_RANGE,
    ADF4351_BandSelectClockFrequency_RANGE,
    ADF4351_R_RANGE,
    ADF4351_PFD_RANGE,
    ADF4351_N_RANGE,
    ADF4351_INT_RANGE,
    ADF4351_MOD_RANGE,
    ADF4351_FRAC_RANGE,
    ADF4351_ERROR_END
};

/** \brief Disable/Enable  
 */
enum
{
    ADF4351_DISABLE,
    ADF4351_ENABLE
};


/** \brief dual-modulus prescaler (P/P + 1)
*/
enum
{
	ADF4351_PRESCALER_4_5,   /*!< Prescaler = 4/5: NMIN = 23 */
    ADF4351_PRESCALER_8_9    /*!< Prescaler = 8/9: NMIN = 75 */
};

/** \brief Low Noise and Low Spur Modes  
 */
enum
{
    ADF4351_LOW_NOISE_MODE,
    ADF4351_LOW_SPUR_MODE = 3
};

/** \brief MUXOUT  multiplexer
 */
enum
{
    ADF4351_MUX_THREESTATE,
    ADF4351_MUX_DVDD,
    ADF4351_MUX_DGND,
    ADF4351_MUX_RCOUNTER,
    ADF4351_MUX_NDIVIDER,
    ADF4351_MUX_ANALOGLOCK,
    ADF4351_MUX_DIGITALLOCK
};


/** \brief Lock Detect Function  FRAC or INT mode
 */
enum
{
    ADF4351_LDF_FRAC_N,
    ADF4351_LDF_INT_N
};

/** \brief  The lock detect window time for lock detect circuit.
 */
enum
{
    ADF4351_LDP_10NS,
    ADF4351_LDP_6NS
};

/** \brief Phase Detector Polarity  
 */
enum
{
    ADF4351_POLARITY_NEG,     /*!< inverting loop filter */
    ADF4351_POLARITY_POS	  /*!< noninverting loop filter */
};


/** \brief  Antibacklash Pulse Width 
 *
 */
enum
{
    ADF4351_ABP_6NS,
    ADF4351_ABP_3NS
};

/** \brief Clock Divider PLL lock mode
 *
 */
enum
{
    ADF4351_CLKDIVMODE_OFF,
    ADF4351_CLKDIVMODE_FAST_LOCK,
    ADF4351_CLKDIVMODE_RESYNC
};

/** \brief Lock Detect Pin Operation  type
 *
 */
enum
{
    ADF4351_LD_PIN_LOW,
    ADF4351_LD_PIN_DIGITAL_LOCK,
    ADF4351_LD_PIN_LOW_,
    ADF4351_LD_PIN_HIGH
};


/** \brief Output Power  
 *
 */
enum
{
    ADF4351_POWER_N4DB,
    ADF4351_POWER_N1DB,
    ADF4351_POWER_2DB,
    ADF4351_POWER_5DB
};

/** \brief AUXout from VCO or dividers
 *
 */
enum
{
    ADF4351_AUXOUT_FROM_RF_DIVIDERS, /*!< RFout is VCO after RF dividers */
    ADF4351_AUXOUT_FROM_VCO          /*!< RFout direct from VCO */
};


#endif

MEMSPACE void ADF4351_sync ( int all );
MEMSPACE uint32_t ADF4351_GetReg32 ( int addr );
MEMSPACE int ADF4351_CHARGE_PUMP_uA ( int32_t x );
MEMSPACE void ADF4351_Init ( void );
MEMSPACE void ADF_dump_registers ( void );
MEMSPACE uint32_t ADF4351_status ( uint8_t mode );
MEMSPACE uint32_t ADF4351_GCD ( uint32_t u , uint32_t v );
MEMSPACE double ADF4351_PFD ( double REFin , int R );
MEMSPACE void ADF4351_display_error ( int error );
MEMSPACE int ADF4351_Config( double RFout , double REFin , double ChannelSpacing , double *RFoutCalc );


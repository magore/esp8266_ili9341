/**
 @file fatfs/unicode.c

 @brief FatFS Unicode include file  R0.10b (C) ChaN, 2014.
 @par Copyright &copy; 2014 ChaN.

*/

#include "user_config.h"
#include "ff.h"

#if _USE_LFN != 0
#if _CODE_PAGE == 437
MEMSPACE
WCHAR ff_convert (WCHAR wch, UINT dir)
{
	if (wch < 0x80) {
		/* ASCII Char */
		return wch;
	}

	/* I don't support unicode it is too big! */
	return 0;
}

MEMSPACE
WCHAR ff_wtoupper (WCHAR wch)
{
	if (wch < 0x80) {
		/* ASCII Char */
		if (wch >= 'a' && wch <= 'z') {
			wch &= ~0x20;
		}
		return wch;
	}

	/* I don't support unicode it is too big! */
	return 0;
}


#elif _CODE_PAGE == 932	/* Japanese Shift_JIS */
#include "cc932.c"
#elif _CODE_PAGE == 936	/* Simplified Chinese GBK */
#include "cc936.c"
#elif _CODE_PAGE == 949	/* Korean */
#include "cc949.c"
#elif _CODE_PAGE == 950	/* Traditional Chinese Big5 */
#include "cc950.c"

#else /* Small character-set */
#include "ccsbcs.c"
#endif

#endif

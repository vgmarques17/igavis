/*
 * ------------------------ TYPES definition ------------------------------
 *
 * $Id: im_types.h 13 2015-06-12 08:57:53Z potsem@usi.ch $
 *
 * added IGB_* alternatives for safer coding.
 * removed old macros after refactoring the code.
 */

#define	 IGB_BYTE         1 /* -- byte ------------------------------------ */
#define	 IGB_CHAR	  2 /* -- Char ------------------------------------ */
#define	 IGB_SHORT	  3 /* -- short ----------------------------------- */
#define	 IGB_LONG	  4 /* -- long ------------------------------------ */
#define	 IGB_FLOAT	  5 /* -- float ----------------------------------- */
#define	 IGB_DOUBLE	  6 /* -- Double ---------------------------------- */
#define	 IGB_COMPLEX	  7 /* -- 2 x float (real part, imaginary part) --- */
#define	 IGB_D_COMPLEX	  8 /* -- 2 x Double (real part, imaginary part) -- */
#define	 IGB_RGBA	  9 /* -- 4 x byte (red, green, blue, alpha) ------ */
#define	 IGB_STRUCTURE	 10 /* -- Structure ------------------------------- */
#define	 IGB_POINTER	 11 /* -- void * ---------------------------------- */
#define	 IGB_LIST	 12 /* -- List   ---------------------------------- */
#define	 IGB_MIN_TYPE	  1
#define	 IGB_MAX_TYPE	 12


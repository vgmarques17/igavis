/*---------------------------------------------------------------------------*\

    systemes.h:	Definition hiearchique des systemes supportes 

    Auteur:	Andre Bleau, ing.

    Laboratoire de Modelisation Biomedicale (LMB)
    Institut de Genie Biomedical
    Ecole Polytechnique / Faculte de Medecine
    Universite de Montreal

    Revision:	16 mars 2000  -- see RCS Id below for followup (MP)

    $Id: systemes.h 13 2015-06-12 08:57:53Z potsem@usi.ch $

\*---------------------------------------------------------------------------*/

#ifdef IRIS_3000
#undef IRIS_3000
#endif
#ifdef IRIS_4D
#undef IRIS_4D
#endif
#ifdef ULTRIX
#undef ULTRIX
#endif
#ifdef SUNOS
#undef SUNOS
#endif
#ifdef LINUX
#undef LINUX
#endif
#ifdef WINNT
#undef WINNT
#endif
#ifdef BSD                // conflict with /usr/include/pm_config.h (libpbm)
#undef BSD
#endif

/*
    Architectures materielles
*/
#define	    ARCH_MSB 0
#define	    ARCH_LSB 1

/*
    Systeme d'operation (64 possibilites)
*/
#define	    FACT_SYS_OP	    26
#define	    SYS_OP	    0xFC000000
/*
    Sous-systeme (64 possibilites)
*/
#define	    FACT_S_SYS_OP   20
#define	    S_SYS_OP	    0xFFF00000
/*
    Version (64 possibilites)
*/
#define	    FACT_VERSION    14
#define	    VERSION	    0xFFFFC000
/*
    Marque (64 possibilites)
*/
#define	    FACT_MARQUE	    8
#define	    MARQUE	    0xFFFFFF00
/*
    Modele (256 possibilites)
*/
#define	    FACT_MODELE	    0
#define	    MODELE	    0xFFFFFFFF

#define	    INCONNU	0

#define	    UNIX    (1<<FACT_SYS_OP)
#define		SYSTEM_V    (UNIX+(1<<FACT_S_SYS_OP))
#define			IRIS	(SYSTEM_V+(1<<FACT_MARQUE))
#define			    IRIS_3000   (IRIS+(1<<FACT_MODELE))
#define			    IRIS_4D	    (IRIS+(2<<FACT_MODELE))
#define		BSD	    (UNIX+(2<<FACT_S_SYS_OP))
#define		ULTRIX	    (UNIX+(3<<FACT_S_SYS_OP))
#define		XENIX	    (UNIX+(4<<FACT_S_SYS_OP))
#define		SUNOS	    (UNIX+(5<<FACT_S_SYS_OP))
#define			SUN		(SUNOS+(1<<FACT_MARQUE))
#define			    SUN2	    (SUN+(2<<FACT_MODELE))
#define			    SUN3	    (SUN+(3<<FACT_MODELE))
#define			    SUN4	    (SUN+(4<<FACT_MODELE))
#define			    SUN386	    (SUN+(5<<FACT_MODELE))
#define		SOLARIS	    (UNIX+(6<<FACT_S_SYS_OP))
#define			SPARC		(SOLARIS+(1<<FACT_MARQUE))
#define			SOLARISPC	(SOLARIS+(2<<FACT_MARQUE))
#define		IRIX	    (UNIX+(7<<FACT_S_SYS_OP))
#define		    IRIX3	(IRIX+(1<<FACT_VERSION))
#define		    IRIX4	(IRIX+(2<<FACT_VERSION))
#define		    IRIX5	(IRIX+(3<<FACT_VERSION))
#define		    IRIX6	(IRIX+(4<<FACT_VERSION))
#define		LINUX	    (UNIX+(8<<FACT_S_SYS_OP))
#define	    VMS	    (2<<FACT_SYS_OP)
#define	    DOS	    (3<<FACT_SYS_OP)
#define		    DOS3	    (DOS+(3<<FACT_VERSION))
#define		    DOS4	    (DOS+(4<<FACT_VERSION))
#define		    DOS5	    (DOS+(5<<FACT_VERSION))
#define		    DOS6	    (DOS+(6<<FACT_VERSION))
#define	    OS2	    (4<<FACT_SYS_OP)
#define	    WINNT   (5<<FACT_SYS_OP)
#define			WINNTINTEL		(WINNT+(1<<FACT_MARQUE))
#define			WINNTMIPS		(WINNT+(2<<FACT_MARQUE))
#define			WINNTALPHA		(WINNT+(3<<FACT_MARQUE))
#define			WINNTPOWER		(WINNT+(4<<FACT_MARQUE))

#define	    N_SYSTEMES	35

#ifdef	HEADER_GLOBALS
/*
    Header_Systeme contient les noms des differents systemes
*/
char	
    *Header_Systeme[N_SYSTEMES] = {
	"unix",
	    "system_v",
		    "iris",
			"iris_3000",
			"iris_4d",
	    "bsd",
	    "ultrix", 
	    "xenix", 
	    "sunos", 
		    "sun",
			"sun2",
			"sun3",
			"sun4",
			"sun386",
	    "solaris", 
		    "sparc",
		    "solarispc",
	    "irix", 
		"irix3",
		"irix4",
		"irix5",
		"irix6",
	    "linux", 
	"vms",
	"dos",
		"dos3",
		"dos4",
		"dos5",
		"dos6",
	"os2",
	"winnt",
		    "winntintel",
		    "winntmips",
		    "winntalpha",
		    "winntpower"
    };

/*
    Header_Systeme_No contient les numeros des differents systemes
*/
long	unsigned
    Header_Systeme_No[N_SYSTEMES] = {
	UNIX,
	    SYSTEM_V,
		    IRIS,
			IRIS_3000,
		 	IRIS_4D,
	    BSD,
	    ULTRIX, 
	    XENIX,
	    SUNOS,
		    SUN,
			SUN2,
			SUN3,
			SUN4,
			SUN386,
	    SOLARIS,
		    SPARC,
		    SOLARISPC,
	    IRIX,
		IRIX3,
		IRIX4,
		IRIX5,
		IRIX6,
	    LINUX, 
	VMS,
	DOS,
		DOS3,
		DOS4,
		DOS4,
		DOS5,
	OS2,
	WINNT,
		    WINNTINTEL,
		    WINNTMIPS,
		    WINNTALPHA,
		    WINNTPOWER
    };
#endif

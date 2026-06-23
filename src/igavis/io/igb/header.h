/*---------------------------------------------------------------------------*\

    header.h:   contient les definitions necessaires a l'utilisation des
		fonction de header.c

    Auteurs:	Yves Martel
		Andre Bleau, ing.

    Laboratoire de Modelisation Biomedicale (LMB)
    Institut de Genie Biomedical
    Ecole Polytechnique / Faculte de Medecine
    Universite de Montreal

    Revision:	29 janvier 2001

\*---------------------------------------------------------------------------*/

#ifndef HEADER_H
#define HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include    <stdio.h>
#include    "systemes.h"
#include    "systeme.h"
#include    "im_types.h"

/* ------------------- Structure contenant l'information ----------------- */
typedef struct Header {
    int	    
	    x, y, z, t ;	    /* --- dimensions -------------------- */
    int	    
	    type ;		    /* --- type arithmetique ------------- */
    int	    
	    taille ;		    /* --- taille des pixels 
					   (type STRUCTURE) -------------- */
    int
	    architecture ;	    /* --- architecture materielle ------- */
    unsigned int
	    systeme ;		    /* --- systeme informatique ---------- */
    int	    
	    num ;		    /* --- numero de la tranche ---------- */
    int	    
	    bin ;		    /* --- nombre de couleurs ------------ */ 
    int	    
	    trame ;		    /* --- trame(connectivite) ----------- */ 
    unsigned int	    
	    lut ;		    /* --- nombre de bytes table couleurs  */
    unsigned int
	    comp ;		    /* --- nombre de bytes table compres.  */
    float   
	    epais ;		    /* --- epaiseur d'une tranche -------- */
    float   
	    org_x, org_y, org_z, org_t ;   /* --- coin sup gauche -------- */
    float   
	    inc_x, inc_y, inc_z, inc_t ;   /* --- distance entre pixels -- */
    float   
	    dim_x, dim_y, dim_z, dim_t ;   /* --- dimension totale ------- */
    float   
	    fac_x, fac_y, fac_z, fac_t ;   /* --- facteurs d'echelle par pixel
					   en X, Y, Z et T --------------- */
    float
	    *vect_z ;               /* --- coord z de chaque tranche ----- */
    char
	    unites_x[41], unites_y[41], unites_z[41], unites_t[41] ;
				    /* --- unites de mesure -------------- */
    char
	    unites[41] ;	    /* --- unites de mesure pour les 
					   valeurs des pixels ------------ */
    float
	    facteur, zero ;	    /* --- facteur d'echelle et valeur du
					   zero -------------------------- */
    char    
	    struct_desc[41] ;	    /* --- description de la structure --- */
    char    
	    aut_name[41] ;	    /* --- nom de l'auteur --------------- */
    char    
	    id[41] ;		    /* --- numero de dossier ou autre ---- */
    char    
	    date[11] ;		    /* --- date de creation AAAA/MM/JJ --- */
    char    
	    **comment ;		    /* --- commentaires ------------------ */
} Header ;

/* ----------- Structure determinant l'information lue ou ecrite --------- */
typedef struct Bool_Header {
    int	    x, y, z, t ;	    /* --- dimensions -------------------- */
    int	    type ;		    /* --- type arithmetique ------------- */
    int	    taille ;		    /* --- taille des pixels 
					   (type STRUCTURE) -------------- */
    int     architecture ;	    /* --- architecture materielle ------- */
    int	    systeme ;		    /* --- systeme informatique ---------- */
    int	    num ;		    /* --- numero de la tranche ---------- */
    int	    bin ;		    /* --- nombre de couleurs ------------ */ 
    int	    trame ;		    /* --- trame(connectivite) ----------- */ 
    int	    lut ;		    /* --- nombre de bytes table couleurs  */
    int	    comp ;		    /* --- nombre de bytes table compres.  */
    int	    epais ;		    /* --- epaiseur d'une tranche -------- */
    int	    org_x, org_y, org_z, org_t ;   /* --- coin sup gauche -------- */
    int	    inc_x, inc_y, inc_z, inc_t ;   /* --- distance entre pixels -- */
    int	    dim_x, dim_y, dim_z, dim_t ;   /* --- dimension totale ------- */
    int     fac_x, fac_y, fac_z, fac_t ;   /* --- facteurs d'echelle par pixel
					   en X, Y, Z et T --------------- */
    int     vect_z ;                /* --- coord z de chaque tranche ----- */
    int	    unites_x, unites_y, unites_z, unites_t ;
		        	    /* --- unites de mesure -------------- */
    int	    unites ;		    /* --- unites de mesure pour la
					   valeur des pixels ------------- */
    int     facteur, zero ;	    /* --- facteur d'echelle et valeur du
					   zero -------------------------- */
    int     struct_desc ;	    /* --- description de la structure --- */
    int	    aut_name ;		    /* --- nom de l'auteur --------------- */
    int	    id ;		    /* --- numero de dossier ou autre ---- */
    int	    date ;		    /* --- date de creation AAAA/MM/JJ --- */
    int	    comment ;		    /* --- commentaires ------------------ */
} Bool_Header ;

/* 
    Definition des types List, bytes, Char, Double, complex d_complex 
    et rgba 
*/

#ifndef PrMTYPES
#define PrMTYPES
typedef	    struct List {
    int     nitems;
    char    *items;
} List;
typedef	   unsigned char	byte;
#if !defined(__GL_GL_H__) && !defined(Byte)
typedef     unsigned char	Byte;
#ifndef _XtIntrinsic_h
typedef     char		*String;
#endif
#endif

typedef	    signed char	    	Char;
typedef	    struct S_Complex	S_Complex;
typedef	    double	    	Double;
typedef     struct D_Complex	D_Complex;
typedef     float		Float;
typedef     int			Int;
#if (S_SYS_OP&SYSTEME)==IRIX6
typedef     int			Long;
#else
typedef     long		Long;
#endif
typedef     short		Short;
typedef     int			BooleaN;
typedef     int			Flag;
typedef     char		*RDir;
typedef     char		*RFile;
typedef     char		*RWDir;
typedef     char		*RWFile;
typedef     char		*WDir;
typedef     char		*WFile;
typedef     char		**Text;
typedef     void		Any;
struct S_Complex {
    Float	real, imag;
};
struct D_Complex {
    Double	real, imag;
};
#ifndef _COMPLEX_DEFINED
#define _COMPLEX_DEFINED
typedef struct complex {
    float   reel ;
    float   imag ;
} complex ;
typedef struct d_complex {
    Double  reel ;
    Double  imag ;
} d_complex ;
#endif
typedef union rgba {
    unsigned	int     l;
    byte		b[4];
} rgba ;
/* Indice de chaque composante dans le vecteur b[] de l'union rgba */
#if (ARCHITECTURE==ARCH_LSB)
#define RGBA_ROUGE 0
#define RGBA_VERT  1
#define RGBA_BLEU  2
#define RGBA_ALPHA 3
#else
#define RGBA_ROUGE 3
#define RGBA_VERT  2
#define RGBA_BLEU  1
#define RGBA_ALPHA 0
#endif
#endif /* ifndef PrMTYPES */

/* ------------------- Prototypes des fonctions de header.c ------------- */

int	Header_Read( FILE *file, Header *head, Bool_Header *bool_head );
int	Header_Serialize( Header*, Bool_Header*, char* , int, int* );
int	Header_Write( FILE *file, Header *header,
	 Bool_Header *bool_header );
int	Aj_ligne_com( Header *header_ptr, int argc, char *argv[] );
int	Ajoute_com( Header *header_ptr1, Header *header_ptr2, int indent );
int	ImageInvalide( char *nom );
void	Ajoute_ligne( Header *header_ptr, char *ligne );
void	Retire_lignes( Header *header_ptr, int n_lig_ret );
void	Ini_comment( Header *header_ptr );
void	Header_Cpy( Header *header_ptr1, Header *header_ptr2 );
void	Bool_Header_Cpy( Bool_Header *b_header_ptr1,
	 Bool_Header *b_header_ptr2 );
void	Header_Clear( Header *header_ptr );
void	Header_Ini( Header *header_ptr );
void	Bool_Header_Ini( Bool_Header *bool_header_ptr );
char	*Img_Ext( void );
char	*conv_date( long date_num, int format );

/* -------------- Definition du type des variables globales de header.c - */

#ifndef	HEADER_GLOBALS
extern	int		 Header_Quiet;
extern	char		*Header_Type[IGB_MAX_TYPE+1];
extern	short		 Header_Size[IGB_MAX_TYPE+1];
extern	char		*Header_Systeme[N_SYSTEMES];
extern  unsigned int	 Header_Systeme_No[N_SYSTEMES];
extern	char		 Header_Message[256];
#endif

/* -------------- Bits de statut pour Header_Read et Header_Write ------ */

#define	    MOT_CLEF_INV    2
#define	    GRANDEUR_INV    4

/* -------------- Constantes diverses ---------------------------------- */

#define	HEADER_MAXL  80	    /* --- Longueur maximale d'une ligne d'entete */

#ifndef	    TRUE
#define	    TRUE	    1
#endif

#ifndef	    FALSE
#define	    FALSE	    0
#endif

#ifndef	    VRAI
#define	    VRAI	    1
#endif

#ifndef	    FAUX
#define	    FAUX	    0
#endif

/* -------------- Extension par defaut pour les fichiers d'images ------ */

#ifndef	    IMG_EXT
#define	    IMG_EXT	    ".igb"
#endif

/* -------------- Limite de longueur pour les noms de fichiers --------- */

#if (SYS_OP&SYSTEME)==DOS
#define MAXFILENAME 8
#else
#define MAXFILENAME 32
#endif

/* -------------- Definitions pour conv_date --------------------------- */

#define	FRANCAIS	0
#define ANGLAIS		1
#define NUMERIQUE	2
#ifndef DATE
#define DATE		0
#endif

#ifdef __cplusplus
}
#endif

#endif /* ifndef HEADER_H */

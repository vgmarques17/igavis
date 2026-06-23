/*---------------------------------------------------------------------------*\

    im_alloc.h:	contient les definitions necessaires a l'utilisation des
		fonction de im_alloc.c

    Auteur:	Andre Bleau, ing.

    Laboratoire de Modelisation Biomedicale (LMB)
    Institut de Genie Biomedical
    Ecole Polytechnique / Faculte de Medecine
    Universite de Montreal

    Revision:	26 janvier 2001

\*---------------------------------------------------------------------------*/

/*
    #include "im_alloc.h" doit etre precede par:
    #include <stdio.h>
    #include "header.h"
    #include "histo.h"
    #include "struct_types.h"
*/

/*
    Indices pour dim[][]
*/
#define	IM_X 0
#define	IM_Y 1
#define	IM_Z 2
#define	IM_T 3

/* Numeros de sequence speciaux */
#define	IM_TRANCHE -1
#define IM_MULTI2D -2

/*
    Definition du type Image
*/
#ifdef __STDC__
typedef struct Image {
    int     ndim;	    /* Nombre de dimensions de l'image */
    int     dim[4][2];	    /* Bornes desirees des 4 dimensions possibles */
    int     vrai_dim[4][2]; /* Bornes reeles des 4 dimensions possibles */
    int     type;	    /* Type de l'image */
    void    *im_1D;	    /* Image 1D */
    void    **im_2D;	    /* Image 2D */
    void    ***im_3D;	    /* Image 3D */
    void    ****im_4D;	    /* Image 4D */
    int     p_type;         /* Type des donnees pointees (pointer et list) */
    int     p_size;         /* Grandeur des donnees pointees (pointer et list) */
    int     maxnitems;      /* Nombre d'items maximal (list) */
    int     trame;          /* Trame (connectivite) */
    int	    sequence;	    /* Numero unique et sequentiel */
    char    *struct_desc;   /* Description (type==STRUCTURE) */
    char    *commentaire;
} Image;
#else  /* __STDC__ */
typedef struct Image {
    int     ndim;	    /* Nombre de dimensions de l'image */
    int     dim[4][2];	    /* Bornes desirees des 4 dimensions possibles */
    int     vrai_dim[4][2]; /* Bornes reeles des 4 dimensions possibles */
    int     type;	    /* Type de l'image */
    char    *im_1D;	    /* Image 1D */
    char    **im_2D;	    /* Image 2D */
    char    ***im_3D;	    /* Image 3D */
    char    ****im_4D;	    /* Image 4D */
    int     p_type;         /* Type des donnees pointees (pointer et list) */
    int     p_size;         /* Grandeur des donnees pointees (pointer et list) */
    int     maxnitems;      /* Nombre d'items maximal (list) */
    int     trame;          /* Trame (connectivite) */
    int	    sequence;	    /* Numero unique et sequentiel */
    char    *struct_desc;   /* Description (type==STRUCTURE) */
    char    *commentaire;
} Image;
#endif /* __STDC__ */

#ifdef __STDC__

void	Header_From_Image( Header *header, Image *image, int bordure );
void	Bool_Header_From_Image( Bool_Header *bool_header, Image *image );

Image	*Im_alloc( int ndim, int dim[4][2], int type );
Image	*Im_alloc_comment( int ndim, int dim[4][2], int type,
	 char *comment );
void	Im_clean( Image *image );
void	Im_clean_sequence( int sequence );
void	Im_comment( Image *image, char *comment );
int	Im_compatible( Image *image, int ndim, int dim[4][2],
	 int type );
int	Im_compatible_comment( Image *image, int ndim, int dim[4][2],
	 int type, char *comment );
int 	Im_cpy( Image *im_new, Image *im_old );
Image	*Im_create( int ndim, int dim[4][2], int type );
void	Im_destroy( Image *image );
void	Im_destroy_sequence( int sequence );
void	Im_duplicate( Image *im_new, Image *im_old );
void	Im_free( Image *image );
unsigned long	Im_get_mem( void );
int	Im_get_n_alloc( void );
int	Im_get_n_free( void );
int	Im_get_prefered( int ndim, int prefered_dim[4][2] );
int	Im_get_sequence( void );
int	Im_incompatible( Image *image, int ndim, int dim[4][2],
	 int type );
void	Im_list_alloc( void );
void	Im_list_free( void );
Image	*Im_multi2D( Image *image2D, int nz );
void	Im_set_prefered( int ndim, int prefered_dim[4][2], int prefered );
void	Im_struct_desc( Image *image, char *struct_desc );
Image	*Im_tranche( Image *image3D, Image *image2D, int z, int t );

Any	*T_alloc( int ndim, int dim[4][2], int type );
Any	*T_alloc_comment( int ndim, int dim[4][2], int type,
	 char *comment );
Any	*T_create( int ndim, int dim[4][2], int type );
Any	*T_create_comment( int ndim, int dim[4][2], int type,
	 char *comment );
int	T_destroy( Any *tableau );
int	T_free( Any *tableau );
void	T_list_alloc( void );
Image	*Trame( int ndim, int dim[4][2], int type );
Image	*Inter_Trame( Image *trame1, Image *trame2, Image *trame3 );
void	Libere_Trame( Image *image );

char	*newstr( char *str );

#else  /* __STDC__ */

void	Header_From_Image(/* Header *header, Image *image, int bordure */);
void	Bool_Header_From_Image(/* Bool_Header *bool_header, Image *image */);

Image	*Im_alloc(/* int ndim, int dim[4][2], int type */);
Image	*Im_alloc_comment(/* int ndim, int dim[4][2], int type,
	 char *comment */);
void	Im_clean(/* Image *image */);
void	Im_clean_sequence(/* int sequence */);
void	Im_comment(/* Image *image, char *comment */);
int	Im_compatible(/* Image *image, int ndim, int dim[4][2],
	 int type */);
int 	Im_cpy(/* Image *im_new, Image *im_old */);
Image	*Im_create(/* int ndim, int dim[4][2], int type */);
void	Im_destroy(/* Image *image */);
void	Im_destroy_sequence(/* int sequence */);
void	Im_duplicate(/* Image *im_new, Image *im_old */);
void	Im_free(/* Image *image */);
unsigned long	Im_get_mem(/* void */);
int	Im_get_n_alloc(/* void */);
int	Im_get_n_free(/* void */);
int	Im_get_prefered(/* int ndim, int prefered_dim[4][2] */);
int	Im_get_sequence(/* void */);
int	Im_incompatible(/* Image *image, int ndim, int dim[4][2],
	 int type */);
void	Im_list_alloc(/* void */);
void	Im_list_free(/* void */);
Image	*Im_multi2D(/* Image *image2D, int nz */);
void	Im_set_prefered(/* int ndim, int prefered_dim[4][2],
	 int prefered */);
void	Im_struct_desc(/* Image *image, char *struct_desc */);
Image	*Im_tranche(/* Image *image3D, Image *image2D, int z, int t */);

Any	*T_alloc(/* int ndim, int dim[4][2], int type */);
Any	*T_alloc_comment(/* int ndim, int dim[4][2], int type,
	 char *comment */);
Any	*T_create(/* int ndim, int dim[4][2], int type */);
Any	*T_create_comment(/* int ndim, int dim[4][2], int type,
	 char *comment */);
int	T_destroy(/* Any *tableau */);
int	T_free(/* Any *tableau */);
void	T_list_alloc(/* void */);
Image	*Trame(/* int ndim, int dim[4][2], int type */);
Image	*Inter_Trame(/* Image *trame1, Image *trame2, Image *trame3 */);
void	Libere_Trame(/* Image *image */);

char	*newstr(/* char *str */);

#endif /* __STDC__ */

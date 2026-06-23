/*---------------------------------------------------------------------------*\

    $Id: header.c 13 2015-06-12 08:57:53Z potsem@usi.ch $

    hacks by Mark Potse
  
    header.c:   contient les fonctions necessaires a la lecture et a
		l'ecriture des etiquettes format "igb" et quelques
		utilitaires.

    Auteurs:	Yves Martel
		Andre Bleau, ing.

    Revision:	29 janvier 2001

\*---------------------------------------------------------------------------*/

#include    <stdio.h>	    /* --- fopen(), fgets(), fclose() ---------- */
#include    <string.h>	    /* --- pour strtok(), strcpy(), strcmp() --- */
#include    <ctype.h>	    /* --- pour isprint(), tolower() ----------- */

#define	    HEADER_GLOBALS
#include    "header.h"
#include    "im_alloc.h"
#include    "trame.h"
#include    <stdlib.h>	    /* --- pour atoi() atof() malloc() realloc() */

#define	LF  0x0A
#define FF  0x0C
#define	CR  0x0D

#define NALLOC	    100	    /* --- Nombre de lignes allouees a la fois */
#define	N_MAX_ITEMS  38	    /* --- Nombre maximal d'items optionnels */
#define	L_MAX_ITEM   49	    /* --- Longueur maximale pour un item optionnel */

/*
    Header_Type contient les noms des differents types
*/
char	
    *Header_Type[] = {
	"", "byte", "char", "short", "long", "float", "double", "complex",
	"double_complex", "rgba", "structure", "pointer", "list"
    };

/*
    Header_Size contient l'espace-memoire utilise par les differents types
*/
short	
    Header_Size[] = {
	0, sizeof(byte), sizeof(char), sizeof(short), sizeof(long), 
	sizeof(float), sizeof(Double), sizeof(complex), sizeof(d_complex),
	sizeof(rgba), 0, sizeof(char *), sizeof(List)
    };

/*
    Header_Message contient le dernier message-diagnostic emis par les
    fonctions Header_Read et Header_Write ci-dessous
*/
char	
    Header_Message[256];

/*
    Header_Quiet gouverne l'affichage des erreurs sur stderr;
	0	Les erreurs seront affichees sur stderr
	autre	Les erreurs ne seront pas affichees sur stderr
*/
int	
    Header_Quiet = 0;

union float_chars {
    float f;
    char c[sizeof(float)];
};

/*---------------------------------------------------------------------------*\

    fonction:	Header_Read()

    description:Lit le header d'un fichier d'image
		l'information est place dans une structure de type Header

    parametres:	file (FILE *)		  input stream
		head (Header *)		  pointeur a la structure
		bool_head (Bool_Header *) indique les membres de la
					  structure trouves dans
					  l'etiquette lue

    retour:	(int)	    0		        si erreur
			    bit 1		si etiquette lisible
			    bit MOT_CLEF_INV    si mot-clef non-reconnu
			    bit GRANDEUR_INV    si la grandeur de
						l'etiquette est invalide

    revision:	23/03/1988    y.m.
		29/01/2001    Andre Bleau, ing.

\*---------------------------------------------------------------------------*/

int	Header_Read(
    FILE	    *file,
    Header	    *head,
    Bool_Header *bool_head
) {

  char	str[1025];  /* was str[HEADER_MAXL+1]; allow buggy headers */
  
  char	tmp, *pt_1, *pt_2, *pt_3 ;
  int	go = VRAI;
  int	nosup = VRAI;	/*  Vrai tant que l'on n'a pas rencontre le mot-clef
			    "sup" */
  int	i ;
  int	in ;
  int	com = 0;
  int	s ;
  int	statut = 1;

  union float_chars f_c;

  float	dim_x, dim_y, dim_z, dim_t ;

    if (getenv("HEADER_QUIET")) Header_Quiet = VRAI;

    if (file==NULL) {
	if (!Header_Quiet) 
	    fprintf(stderr, "\nERREUR: descripteur de fichier nul\n");
	sprintf(Header_Message, "\nERREUR: descripteur de fichier nul\n");
	return( 0 ) ;
    }

    /* --- initialisation des variables --- */
    Header_Ini( head );
    Bool_Header_Ini( bool_head );

    /* --- pour toutes les lignes de l'entete (def=8) ou jusqu'a un <FF> -- */
    for ( s=8; (s>0 || nosup) && go; s-- ) {

	/* --- lit la ligne dans le fichier --- */
	i = 0 ;

	while(1) {

	    /* handle missing FF at end of header: at byte 1024,
	       behave as if there was a FF */
	    if(i>=1024){
		str[i] = '\000' ;
		go = FAUX ;
		break ;
	    }
	      
	    
	    /* --- lit le caractere suivant --- */
	    in = getc( file ) ;

	    /* --- (EOF dans l'entete) --> erreur --- */
	    if ( in == EOF ) {
		if (!Header_Quiet) 
		    fprintf(stderr,
		     "\nERREUR Fin de fichier dans l'entete !\n" ) ;
		sprintf(Header_Message,
		 "\nERREUR Fin de fichier dans l'entete !\n" ) ;
		return( 0 ) ;
	    }

	    /* --- FF --> termine le header --- */
	    else if ( in == FF ) {
		str[i] = '\000' ;
		go = FAUX ;
		break ;
	    }
 
	    /* --- CR et LF --> termine la ligne --- */
	    else if ( in == CR ) {
		str[i] = '\000' ;
	    }
	    else if ( in == LF ) {
		str[i] = '\000' ;
		break ;
	    }

	    /* --- (0x20 <= caractere) si non --> erreur --- */
	    else if ( in && ( in < 0x20 || in==0x7F ) ) {
		if (!Header_Quiet) 
		    fprintf(stderr, 
		     "\nERREUR caract. non imprim. 0x%.2X dans l'entete !\n",
		     in);
		sprintf(Header_Message,
		 "\nERREUR caract. non imprim. 0x%.2X dans l'entete !\n",
		 in);
		return( 0 ) ;
	    }
	    else {
		str[i++] = (char) in ;
	    }

	}

	/* --- ----- ----- ----- sauve les commentaires ----- ----- --- */
	for ( pt_1=str; *pt_1; pt_1++ ) {
	    
	    /* --- '*' ou '#' = commentaires ------> dans comment --- */
	    if ( *pt_1 == '*' || *pt_1 == '#') {
		if ( *(pt_1+1) ) {
		    head->comment[com] = malloc( strlen(pt_1+1) + 1 ) ;
		    strcpy( head->comment[com++], pt_1+1 ) ;
		    if (com%NALLOC == 0) 
		    	head->comment = (char **)
			 realloc(head->comment, (com+NALLOC)*sizeof(char *));
		    head->comment[com] = NULL ;
		    bool_head->comment = VRAI;
		}
		*pt_1 = '\000' ;
		break ;
	    }

	}


	/* --- ----- ----- ----- analyse la ligne ----- ----- ----- --- */
	for ( pt_1=strtok(str," ,;\t"); pt_1; pt_1=strtok(NULL," ,;\t") ) {

	    /* --- separe la chaine de caract. --- */
	    /* --- pt_1 pointe au key_word ---- */
	    /* --- pt_2 pointe a la donne  ---- */
	    for ( pt_2 = pt_1; *pt_2 != ':'; pt_2 ++ ) {
		if ( ! *pt_2 ) {
		    if (!Header_Quiet) 
			fprintf(stderr, 
			 "\nERREUR de syntaxe dans l'entete (%s)\n", pt_1);
		    sprintf(Header_Message,
		     "\nERREUR de syntaxe dans l'entete (%s)\n", pt_1);
		    return( 0 ) ;
		}
		/* --- convertit les majuscules du mot-clefs en minuscules - */
		if (isupper(*pt_2)) *pt_2 = tolower( *pt_2 ) ;
	    }

	    *pt_2++ = '\000' ;

	    /* --- recherche le mot-clef --- */
	    if ( ! strcmp( pt_1, "x" ) ) {
		head->x = atoi( pt_2 ) ;		
		bool_head->x = VRAI;

	    } else if ( ! strcmp( pt_1, "y" ) ) {
		head->y = atoi( pt_2 ) ;		
		bool_head->y = VRAI;

	    } else if ( ! strcmp( pt_1, "z" ) ) {
		head->z = atoi( pt_2 ) ;		
		bool_head->z = VRAI;

	    } else if ( ! strcmp( pt_1, "t" ) ) {
		head->t = atoi( pt_2 ) ;		
		bool_head->t = VRAI;

	    /* Pour compatibilite avec les vielles images */
	    } else if ( ! strcmp( pt_1, "sup" ) ) {
		s += atoi( pt_2 ) ;
		nosup = FAUX;

	    } else if ( ! strcmp( pt_1, "type" ) ) {

		for ( pt_3 = pt_2; *pt_3 != '\000'; pt_3 ++ ) {
		    /* --- convertit les majuscules en minuscules --- */
		    if (isupper(*pt_3)) *pt_3 = tolower( *pt_3 ) ;
		}
		if ( ! strcmp( pt_2, "byte" ) ) {	
		    head->type = IGB_BYTE ;		    
		} else if ( ! strcmp( pt_2, "char" ) ) {
		    head->type = IGB_CHAR ;
		} else if ( ! strcmp( pt_2, "short" ) ) {
		    head->type = IGB_SHORT ;
		} else if ( ! strcmp( pt_2, "long" ) ) {	
		    head->type = IGB_LONG ;
		} else if ( ! strcmp( pt_2, "float" ) ) {	
		    head->type = IGB_FLOAT ;
		} else if ( ! strcmp( pt_2, "double" ) ) {	
		    head->type = IGB_DOUBLE ;
		} else if ( ! strcmp( pt_2, "complex" ) ) {
		    head->type = IGB_COMPLEX ;
		} else if ( ! strcmp( pt_2, "double_complex" ) ) {
		    head->type = IGB_D_COMPLEX ;
		} else if ( ! strcmp( pt_2, "rgba" ) ) {
		    head->type = IGB_RGBA ;
		} else if ( ! strcmp( pt_2, "structure" ) ) {
		    head->type = IGB_STRUCTURE ;
		}
		bool_head->type = VRAI;

	    } else if ( ! strcmp( pt_1, "taille" ) ) {
		head->taille = atoi( pt_2 ) ;
		bool_head->taille = VRAI;

	    } else if ( ! strcmp( pt_1, "architecture" ) ) {

		for ( pt_3 = pt_2; *pt_3 != '\000'; pt_3 ++ ) {
		    /* --- convertit les majuscules en minuscules --- */
		    if (isupper(*pt_3)) *pt_3 = tolower( *pt_3 ) ;
		}
		if ( ! strcmp( pt_2, "msb" ) ) {	
		    head->architecture = ARCH_MSB ;		    
		} else if ( ! strcmp( pt_2, "lsb" ) ) {
		    head->architecture = ARCH_LSB ;
		}
		bool_head->architecture = VRAI;

	    } else if ( ! strcmp( pt_1, "systeme" ) ) {

		for ( pt_3 = pt_2; *pt_3 != '\000'; pt_3 ++ ) {
		    /* --- convertit les majuscules en minuscules --- */
		    if (isupper(*pt_3)) *pt_3 = tolower( *pt_3 ) ;
		}
		head->systeme = INCONNU;
		for (i=0;i<N_SYSTEMES;i++) {
		    if ( ! strcmp( pt_2, Header_Systeme[i] ) ) { 
			head->systeme = Header_Systeme_No[i] ;
			break;
		    }
		}

	    } else if ( ! strcmp( pt_1, "bin" ) ) {
		head->bin = atoi( pt_2 ) ;
		bool_head->bin = VRAI;

	    } else if ( ! strcmp( pt_1, "trame" ) ) {

		for ( pt_3 = pt_2; *pt_3 != '\000'; pt_3 ++ ) {
		    /* --- convertit les majuscules en minuscules --- */
		    if (isupper(*pt_3)) *pt_3 = tolower( *pt_3 ) ;
		}
		if ( ! strcmp( pt_2, "c8" ) ) {	
		    head->trame = C8 ;		    
		} else if ( ! strcmp( pt_2, "c4" ) ) {
		    head->trame = C4 ;
		} else if ( ! strcmp( pt_2, "hex" ) ) {
		    head->trame = HEX ;
		} else if ( ! strcmp( pt_2, "hexedges" ) ) {	
		    head->trame = HEXEDGES ;
		} else if ( ! strcmp( pt_2, "hexbridges" ) ) {	
		    head->trame = HEXBRIDGES ;
		} else if ( ! strcmp( pt_2, "hexlines" ) ) {	
		    head->trame = HEXLINES ;
		} else if ( ! strcmp( pt_2, "hex2" ) ) {	
		    head->trame = HEX2 ;
		}
		bool_head->trame = VRAI;

	    } else if ( ! strcmp( pt_1, "num" ) ) {
		head->num = atoi( pt_2 ) ;
		bool_head->num = VRAI;

	    } else if ( ! strcmp( pt_1, "comp" ) ) {
		head->comp = atoi( pt_2 ) ;
		bool_head->comp = VRAI;

	    } else if ( ! strcmp( pt_1, "lut" ) ) {
		head->lut = atoi( pt_2 ) ;
		bool_head->lut = VRAI;

	    } else if ( ! strcmp( pt_1, "dim_x" ) ) {
		head->dim_x = (float)atof( pt_2 ) ;
		bool_head->dim_x = VRAI;

	    } else if ( ! strcmp( pt_1, "dim_y" ) ) {
		head->dim_y = (float)atof( pt_2 ) ;
		bool_head->dim_y = VRAI;

	    } else if ( ! strcmp( pt_1, "dim_z" ) ) {
		head->dim_z = (float)atof( pt_2 ) ;
		bool_head->dim_z = VRAI;

	    } else if ( ! strcmp( pt_1, "dim_t" ) ) {
		head->dim_t = (float)atof( pt_2 ) ;
		bool_head->dim_t = VRAI;

	    } else if ( ! strcmp( pt_1, "fac_x" ) ) {
		head->fac_x = (float)atof( pt_2 ) ;
		bool_head->fac_x = VRAI;

	    } else if ( ! strcmp( pt_1, "fac_y" ) ) {
		head->fac_y = (float)atof( pt_2 ) ;
		bool_head->fac_y = VRAI;

	    } else if ( ! strcmp( pt_1, "fac_z" ) ) {
		head->fac_z = (float)atof( pt_2 ) ;
		bool_head->fac_z = VRAI;

	    } else if ( ! strcmp( pt_1, "fac_t" ) ) {
		head->fac_t = (float)atof( pt_2 ) ;
		bool_head->fac_t = VRAI;

	    } else if ( ! strcmp( pt_1, "inc_x" ) ) {
		head->inc_x = (float)atof( pt_2 ) ;
		bool_head->inc_x = VRAI;

	    } else if ( ! strcmp( pt_1, "inc_y" ) ) {
		head->inc_y = (float)atof( pt_2 ) ;
		 bool_head->inc_y = VRAI;

	    } else if ( ! strcmp( pt_1, "inc_z" ) ) {
		head->inc_z = (float)atof( pt_2 ) ;
		bool_head->inc_z = VRAI;

	    } else if ( ! strcmp( pt_1, "inc_t" ) ) {
		head->inc_t = (float)atof( pt_2 ) ;
		bool_head->inc_t = VRAI;

	    } else if ( ! strcmp( pt_1, "org_x" ) ) {
		head->org_x = (float)atof( pt_2 ) ;
		bool_head->org_x = VRAI;

	    } else if ( ! strcmp( pt_1, "org_y" ) ) {
		head->org_y = (float)atof( pt_2 ) ;
		bool_head->org_y = VRAI;

	    } else if ( ! strcmp( pt_1, "org_z" ) ) {
		head->org_z = (float)atof( pt_2 ) ;
		bool_head->org_z = VRAI;

	    } else if ( ! strcmp( pt_1, "org_t" ) ) {
		head->org_t = (float)atof( pt_2 ) ;
		bool_head->org_t = VRAI;

	    } else if ( ! strcmp( pt_1, "vect_z" ) ) {
		bool_head->vect_z = VRAI;

	    } else if ( ! strcmp( pt_1, "unites_x" ) ) {
		for ( pt_3 = pt_2; *pt_3 != '\000'; pt_3 ++ ) {
		    /* --- convertit les soulignes en espaces --- */
		    if (*pt_3=='_') *pt_3 = ' ' ;
		}
		strncpy( head->unites_x, pt_2, 40 ) ;
		bool_head->unites_x = VRAI;

	    } else if ( ! strcmp( pt_1, "unites_y" ) ) {
		for ( pt_3 = pt_2; *pt_3 != '\000'; pt_3 ++ ) {
		    /* --- convertit les soulignes en espaces --- */
		    if (*pt_3=='_') *pt_3 = ' ' ;
		}
		strncpy( head->unites_y, pt_2, 40 ) ;
		bool_head->unites_y = VRAI;

	    } else if ( ! strcmp( pt_1, "unites_z" ) ) {
		for ( pt_3 = pt_2; *pt_3 != '\000'; pt_3 ++ ) {
		    /* --- convertit les soulignes en espaces --- */
		    if (*pt_3=='_') *pt_3 = ' ' ;
		}
		strncpy( head->unites_z, pt_2, 40 ) ;
		bool_head->unites_z = VRAI;

	    } else if ( ! strcmp( pt_1, "unites_t" ) ) {
		for ( pt_3 = pt_2; *pt_3 != '\000'; pt_3 ++ ) {
		    /* --- convertit les soulignes en espaces --- */
		    if (*pt_3=='_') *pt_3 = ' ' ;
		}
		strncpy( head->unites_t, pt_2, 40 ) ;
		bool_head->unites_t = VRAI;

	    } else if ( ! strcmp( pt_1, "epais" ) ) {
		head->epais = (float)atof( pt_2 ) ;
		bool_head->epais = VRAI;

	    } else if ( ! strcmp( pt_1, "unites" ) ) {
		for ( pt_3 = pt_2; *pt_3 != '\000'; pt_3 ++ ) {
		    /* --- convertit les soulignes en espaces --- */
		    if (*pt_3=='_') *pt_3 = ' ' ;
		}
		strncpy( head->unites, pt_2, 40 ) ;
		bool_head->unites = VRAI;

	    } else if ( ! strcmp( pt_1, "facteur" ) ) {
		head->facteur = (float)atof( pt_2 ) ;
		bool_head->facteur = VRAI;

	    } else if ( ! strcmp( pt_1, "zero" ) ) {
		head->zero = (float)atof( pt_2 ) ;
		bool_head->zero = VRAI;

	    } else if ( ! strcmp( pt_1, "struct" ) ) {
		for ( pt_3 = pt_2; *pt_3 != '\000'; pt_3 ++ ) {
		    /* --- convertit les soulignes en espaces --- */
		    if (*pt_3=='_') *pt_3 = ' ' ;
		}
		strncpy( head->struct_desc, pt_2, 40 ) ;
		bool_head->struct_desc = VRAI;

	    } else if ( ! strcmp( pt_1, "aut" ) ) {
		for ( pt_3 = pt_2; *pt_3 != '\000'; pt_3 ++ ) {
		    /* --- convertit les soulignes en espaces --- */
		    if (*pt_3=='_') *pt_3 = ' ' ;
		}
		strncpy( head->aut_name, pt_2, 40 ) ;
		bool_head->aut_name = VRAI;

	    } else if ( ! strcmp( pt_1, "id" ) ) {
		for ( pt_3 = pt_2; *pt_3 != '\000'; pt_3 ++ ) {
		    /* --- convertit les soulignes en espaces --- */
		    if (*pt_3=='_') *pt_3 = ' ' ;
		}
		strncpy( head->id, pt_2, 40 ) ;
		bool_head->id = VRAI;

	    } else if ( ! strcmp( pt_1, "date" ) ) {
		for ( pt_3 = pt_2; *pt_3 != '\000'; pt_3 ++ ) {
		    /* --- convertit les soulignes en espaces --- */
		    if (*pt_3=='_') *pt_3 = ' ' ;
		}
		strncpy( head->date, pt_2, 10 ) ;
		bool_head->date = VRAI;

	    } else {
		if (!Header_Quiet) 
		    fprintf(stderr, 
		     "\nATTENTION: mot-clef %s non reconnu !\n", pt_1 ) ;
		sprintf(Header_Message, 
		 "\nATTENTION: mot-clef %s non reconnu !\n", pt_1 ) ;
		statut |= MOT_CLEF_INV;
	    }
	}
    }

    /* --- l'info x y et type est obligatoire --- */
    if ( !bool_head->x || !bool_head->y || !head->type ) {
	if (!Header_Quiet) 
	    fprintf(stderr, "\nERREUR x, y ou type non definis\n") ;
	sprintf(Header_Message, "\nERREUR x, y ou type non definis\n") ;
	return( 0 ) ;
    }

    /* --- calcul des inc et dim --- */

    if ( bool_head->dim_x ) 
	if ( bool_head->inc_x ) {
	    dim_x = head->inc_x * head->x ;
	    if ( dim_x != head->dim_x ) {
		if (!Header_Quiet) {
		    fprintf(stderr, "\nATTENTION:\n") ;
		    fprintf(stderr, 
"conflit entre x (%d) * inc_x (%.12g) = %.12g et dim_x (%.12g)\n",
		     head->x, head->inc_x, dim_x, head->dim_x) ;
		}
		sprintf(Header_Message, 
"conflit entre x (%d) * inc_x (%.12g) = %.12g et dim_x (%.12g)\n",
		 head->x, head->inc_x, dim_x, head->dim_x) ;
		statut = 0 ;
	    }
	}
	else {
	    head->inc_x = head->dim_x / head->x ;
	    bool_head->inc_x = VRAI;
	}
    else {
	head->dim_x = head->x * head->inc_x ;
	if ( bool_head->inc_x ) 
	    bool_head->dim_x = VRAI;
    }

    if ( bool_head->dim_y ) 
	if ( bool_head->inc_y ) {
	    dim_y = head->inc_y * head->y ;
	    if ( dim_y != head->dim_y ) {
		if (!Header_Quiet) {
		    fprintf(stderr, "\nATTENTION:\n") ;
		    fprintf(stderr, 
"conflit entre y (%d) * inc_y (%.12g) = %.12g et dim_y (%.12g)\n",
		     head->y, head->inc_y, dim_y, head->dim_y) ;
		}
		sprintf(Header_Message, 
"conflit entre y (%d) * inc_y (%.12g) = %.12g et dim_y (%.12g)\n",
		 head->y, head->inc_y, dim_y, head->dim_y) ;
		statut = 0 ;
	    }
	}
	else {
	    head->inc_y = head->dim_y / head->y ;
//	    if ( bool_head->inc_y ) 
		bool_head->inc_y = VRAI;
	}
    else {
	head->dim_y = head->y * head->inc_y ;
	if ( bool_head->inc_y ) 
	    bool_head->dim_y = VRAI;
    }

    if ( bool_head->dim_z ) 
	if ( bool_head->inc_z ) {
	    dim_z = head->inc_z * head->z ;
	    if ( dim_z != head->dim_z ) {
		if (!Header_Quiet) {
		    fprintf(stderr, "\nATTENTION:\n") ;
		    fprintf(stderr, 
"conflit entre z (%d) * inc_z (%.12g) = %.12g et dim_z (%.12g)\n",
		     head->z, head->inc_z, dim_z, head->dim_z) ;
		}
		sprintf(Header_Message, 
"conflit entre z (%d) * inc_z (%.12g) = %.12g et dim_z (%.12g)\n",
		 head->z, head->inc_z, dim_z, head->dim_z) ;
		statut = 0 ;
	    }
	}
	else {
	    head->inc_z = head->dim_z / head->z ;
	    bool_head->inc_z = VRAI;
	}
    else {
	head->dim_z = head->z * head->inc_z ;
	if ( bool_head->inc_z ) 
	    bool_head->dim_z = VRAI;
    }

    if ( bool_head->dim_t ) 
	if ( bool_head->inc_t ) {
	    dim_t = head->inc_t * head->t ;
	    if ( dim_t != head->dim_t ) {
		if (!Header_Quiet) {
		    fprintf(stderr, "\nATTENTION:\n") ;
		    fprintf(stderr, 
"conflit entre t (%d) * inc_t (%.12g) = %.12g et dim_t (%.12g)\n",
		     head->t, head->inc_t, dim_t, head->dim_t) ;
		}
		sprintf(Header_Message, 
"conflit entre t (%d) * inc_t (%.12g) = %.12g et dim_t (%.12g)\n",
		 head->t, head->inc_t, dim_t, head->dim_t) ;
		statut = 0 ;
	    }
	}
	else {
	    head->inc_t = head->dim_t / head->t ;
	    bool_head->inc_t = VRAI;
	}
    else {
	head->dim_t = head->t * head->inc_t ;
	if ( bool_head->inc_t ) 
	    bool_head->dim_t = VRAI;
    }

    if ( bool_head->taille ) {
	if (head->type!=IGB_STRUCTURE) {
	    if (!Header_Quiet) 
		fprintf(stderr,
		 "\nERREUR taille redefinie pour type autre que structure\n") ;
	    sprintf(Header_Message, 
	     "\nERREUR taille redefinie pour type autre que structure\n") ;
	    return( 0 ) ;
	}
    }
    else {
	if (head->type==IGB_STRUCTURE) {
	    if (!Header_Quiet) 
		fprintf(stderr,
		 "\nERREUR taille non definie pour type structure\n") ;
	    sprintf(Header_Message, 
	     "\nERREUR taille non definie pour type structure\n") ;
	    return( 0 ) ;
	}
	else {
	    head->taille = Header_Size[head->type];
	}
    }

    if (ftell(file)!=1024) {
	if (!Header_Quiet) {
	    fprintf(stderr,
	     "\nATTENTION: etiquette de grandeur non-standard \n");
	}
	sprintf(Header_Message,
	 "\nATTENTION: etiquette de grandeur non-standard \n");
	statut |= GRANDEUR_INV;
    }
    else {
	sprintf(Header_Message, 
	 "\nHeader_Read: Etiquette lue sans problemes\n");
    }

    if (bool_head->vect_z) {
	head->vect_z = (float *)malloc(head->z*sizeof(float));
	fread(head->vect_z, sizeof(float), head->z, file);
	if (head->architecture!=ARCHITECTURE) {
	    for ( i=0; i<head->z; i++ ) {
		f_c.f = head->vect_z[i];
		for ( pt_1=&f_c.c[0],pt_2=&f_c.c[sizeof(float)-1];
		 pt_1<pt_2; pt_1++,pt_2-- ) {
		    tmp = *pt_1;
		    *pt_1 = *pt_2;
		    *pt_2 = tmp;
		}
		head->vect_z[i] = f_c.f;
	    }
	}
    }

    return( statut ) ;

}


/* The function Header_Serialize serializes the header using the information
   in the bool_header. The serialized version is stored in the buffer buf 
   which is of size n. In k, the number of written chars is returned. */
int Header_Serialize( Header* header, Bool_Header* bool_header, char* buf, int n, int* k ) {
    long i, j;

    auto	    char
        *architecture,
        **comment, 
        items[N_MAX_ITEMS+1][L_MAX_ITEM], 
        ligne[73],
        *pt_1,
        *pt_2,
        *pt_3,
        *systeme,
        tmp,
        *type;
    auto	    int
        l_item[N_MAX_ITEMS+1],
        n_blocs, 
        n_car,
        n_car_dl,
        n_car_sup, 
        n_car_total, 
        n_comment, 
        n_items, 
        n_lignes,
        n_lig_sup, 
        statut = 1;

    union float_chars
        f_c;

    	if (getenv("HEADER_QUIET")) Header_Quiet = VRAI;

        if( !buf ) {
            if( !Header_Quiet )
                fprintf( stderr, "\nERROR: passed NULL buffer to " 
                                 "Header_Serialize.\n" );
            // FIXME: Overflow?
            sprintf( Header_Message, "\nERROR: passed NULL buffer to "
                                     "Header_Serialize.\n" );

            return 0;
        }

    if (header->type<IGB_MIN_TYPE || header->type>IGB_MAX_TYPE) {
        if (!Header_Quiet) 
    	fprintf(stderr, "\nHeader_Write: type inconnu: %d\n", header->type);
        sprintf(Header_Message, "\nHeader_Write: type inconnu: %d\n", 
    	header->type);
        return (0);
    }
    type = Header_Type[header->type];

    if (header->type==IGB_STRUCTURE && header->taille<1) {
        if (!Header_Quiet) 
    	fprintf(stderr, "\nHeader_Write: taille invalide: %d\n",
    	 header->taille);
        sprintf(Header_Message, "\nHeader_Write: taille invalide: %d\n",
    	 header->taille);
        return (0);
    }

    if (header->trame<MIN_TRAME || header->trame>MAX_TRAME) {
        if (!Header_Quiet) 
    	fprintf(stderr, "\nHeader_Write: trame inconnue: %d\n", header->trame);
        sprintf(Header_Message, "\nHeader_Write: trame inconnue: %d\n", 
    	header->trame);
        return (0);
    }

    *k = 0; /* Access k only if we can be sure that we do not bail out
               because of invalid input */

    if (bool_header->architecture) {
        if (header->architecture==ARCH_MSB)
	  architecture = "msb";
        else if (header->architecture==ARCH_LSB)
	  architecture = "lsb";
        else {
    	if (!Header_Quiet) 
	  fprintf(stderr, "\nHeader_Write: architecture inconnue: %d\n", 
		  header->architecture);
    	sprintf(Header_Message, "\nHeader_Write: architecture inconnue: %d\n", 
    	 header->architecture);
    	if (ARCHITECTURE==ARCH_MSB)
	  architecture = "msb";
    	else if (ARCHITECTURE==ARCH_LSB)
	  architecture = "lsb";
        }
    }
    else {   /* If no architecture was set in the header struct,
		assume that the data will be written in the machine's
		native byte order */
        if (ARCHITECTURE==ARCH_MSB)
	  architecture = "msb";
        else if (ARCHITECTURE==ARCH_LSB)
	  architecture = "lsb";
    }

    if (bool_header->systeme) {
      for (i=0;i<N_SYSTEMES;i++) {
    	if ( (MODELE&header->systeme) == Header_Systeme_No[i] ) {
	  systeme = Header_Systeme[i] ;
	  break;
    	}
      }
      if (i == N_SYSTEMES) {
    	if (!Header_Quiet) 
	  fprintf(stderr, "\nHeader_Write: systeme inconnu: %d\n", 
		  header->systeme);
    	sprintf(Header_Message, "\nHeader_Write: systeme inconnu: %d\n", 
		header->systeme);
    	systeme = "inconnu";
      }
    }

    if (bool_header->t) {
      if (bool_header->z) {
    	sprintf(ligne, "x:%d y:%d z:%d t:%d type:%s architecture:%s ", 
    	 header->x, header->y, header->z, header->t, type, architecture);
      }
      else {
    	sprintf(ligne, "x:%d y:%d t:%d type:%s architecture:%s ", 
    	 header->x, header->y, header->t, type, architecture);
      }
    }
    else {
      if (bool_header->z) {
	sprintf(ligne, "x:%d y:%d z:%d type:%s architecture:%s ", 
		header->x, header->y, header->z, type, architecture);
      }
      else {
    	sprintf(ligne, "x:%d y:%d type:%s architecture:%s ", 
		header->x, header->y, type, architecture);
      }
    }
    n_car = strlen(ligne);

    n_lignes = 1;
    n_items = 0;
    /*
        Le mot-clef "taille" n'est ecrit que pour le type STRUCTURE mais il est
        obligatoire pour ce cas.
    */
    if (header->type==IGB_STRUCTURE) {
        sprintf(&items[n_items][0], "taille:%d ", header->taille);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->systeme) {
        sprintf(&items[n_items][0], "systeme:%s ", systeme);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->org_x) {
        sprintf(&items[n_items][0], "org_x:%g ", header->org_x);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->org_y) {
        sprintf(&items[n_items][0], "org_y:%g ", header->org_y);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->org_z) {
        sprintf(&items[n_items][0], "org_z:%g ", header->org_z);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->org_t) {
        sprintf(&items[n_items][0], "org_t:%g ", header->org_t);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->fac_x) {
        sprintf(&items[n_items][0], "fac_x:%g ", header->fac_x);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->fac_y) {
        sprintf(&items[n_items][0], "fac_y:%g ", header->fac_y);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->fac_z) {
        sprintf(&items[n_items][0], "fac_z:%g ", header->fac_z);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->fac_t) {
        sprintf(&items[n_items][0], "fac_t:%g ", header->fac_t);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->dim_x) {
        sprintf(&items[n_items][0], "dim_x:%g ", header->dim_x);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    else if (bool_header->inc_x) {
        sprintf(&items[n_items][0], "dim_x:%g ", header->inc_x * header->x);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->dim_y) {
        sprintf(&items[n_items][0], "dim_y:%g ", header->dim_y);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    else if (bool_header->inc_y) {
        sprintf(&items[n_items][0], "dim_y:%g ", header->inc_y * header->y);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->dim_z) {
        sprintf(&items[n_items][0], "dim_z:%g ", header->dim_z);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    else if (bool_header->inc_z) {
        sprintf(&items[n_items][0], "dim_z:%g ", header->inc_z * header->z);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->dim_t) {
        sprintf(&items[n_items][0], "dim_t:%g ", header->dim_t);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    else if (bool_header->inc_t) {
        sprintf(&items[n_items][0], "dim_t:%g ", header->inc_t * header->t);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->vect_z) {
        sprintf(&items[n_items][0], "vect_z:1 ");
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->unites_x) {
        sprintf(&items[n_items][0], "unites_x:%.40s", header->unites_x);
        for ( pt_3 = &items[n_items][0]; *pt_3 != '\000'; pt_3 ++ ) {
    	/* --- convertit les separateurs en soulignes --- */
    	if (*pt_3==' '||*pt_3==','||*pt_3==';'||*pt_3=='\t'||*pt_3=='\n') 
    	    *pt_3 = '_' ;
        }
        *pt_3++ = ' ' ;
        *pt_3++ = '\000' ;
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->unites_y) {
        sprintf(&items[n_items][0], "unites_y:%.40s", header->unites_y);
        for ( pt_3 = &items[n_items][0]; *pt_3 != '\000'; pt_3 ++ ) {
    	/* --- convertit les separateurs en soulignes --- */
    	if (*pt_3==' '||*pt_3==','||*pt_3==';'||*pt_3=='\t'||*pt_3=='\n') 
    	    *pt_3 = '_' ;
        }
        *pt_3++ = ' ' ;
        *pt_3++ = '\000' ;
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->unites_z) {
        sprintf(&items[n_items][0], "unites_z:%.40s", header->unites_z);
        for ( pt_3 = &items[n_items][0]; *pt_3 != '\000'; pt_3 ++ ) {
    	/* --- convertit les separateurs en soulignes --- */
    	if (*pt_3==' '||*pt_3==','||*pt_3==';'||*pt_3=='\t'||*pt_3=='\n') 
    	    *pt_3 = '_' ;
        }
        *pt_3++ = ' ' ;
        *pt_3++ = '\000' ;
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->unites_t) {
        sprintf(&items[n_items][0], "unites_t:%.40s", header->unites_t);
        for ( pt_3 = &items[n_items][0]; *pt_3 != '\000'; pt_3 ++ ) {
    	/* --- convertit les separateurs en soulignes --- */
    	if (*pt_3==' '||*pt_3==','||*pt_3==';'||*pt_3=='\t'||*pt_3=='\n') 
    	    *pt_3 = '_' ;
        }
        *pt_3++ = ' ' ;
        *pt_3++ = '\000' ;
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->num) {
        sprintf(&items[n_items][0], "num:%d ", header->num);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->bin) {
        sprintf(&items[n_items][0], "bin:%d ", header->bin);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->trame) {
        switch(header->trame) {
        case C8:
    	sprintf(&items[n_items][0], "trame:c8 ");
    	break;
        case C4:
    	sprintf(&items[n_items][0], "trame:c4 ");
    	break;
        case HEX:
    	sprintf(&items[n_items][0], "trame:hex ");
    	break;
        case HEXEDGES:
    	sprintf(&items[n_items][0], "trame:hexedges ");
    	break;
        case HEXBRIDGES:
    	sprintf(&items[n_items][0], "trame:hexbridges ");
    	break;
        case HEXLINES:
    	sprintf(&items[n_items][0], "trame:hexlines ");
    	break;
        case HEX2:
    	sprintf(&items[n_items][0], "trame:hex2 ");
    	break;
        }
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->lut) {
        sprintf(&items[n_items][0], "lut:%d ", header->lut);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->comp) {
        sprintf(&items[n_items][0], "comp:%d ", header->comp);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->epais) {
        sprintf(&items[n_items][0], "epais:%g ", header->epais);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->unites) {
        sprintf(&items[n_items][0], "unites:%.40s", header->unites);
        for ( pt_3 = &items[n_items][0]; *pt_3 != '\000'; pt_3 ++ ) {
    	/* --- convertit les separateurs en soulignes --- */
    	if (*pt_3==' '||*pt_3==','||*pt_3==';'||*pt_3=='\t'||*pt_3=='\n') 
    	    *pt_3 = '_' ;
        }
        *pt_3++ = ' ' ;
        *pt_3++ = '\000' ;
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->facteur) {
        sprintf(&items[n_items][0], "facteur:%g ", header->facteur);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->zero) {
        sprintf(&items[n_items][0], "zero:%g ", header->zero);
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->struct_desc) {
        sprintf(&items[n_items][0], "struct:%.40s", header->struct_desc);
        for ( pt_3 = &items[n_items][0]; *pt_3 != '\000'; pt_3 ++ ) {
    	/* --- convertit les separateurs en soulignes --- */
    	if (*pt_3==' '||*pt_3==','||*pt_3==';'||*pt_3=='\t'||*pt_3=='\n') 
    	    *pt_3 = '_' ;
        }
        *pt_3++ = ' ' ;
        *pt_3++ = '\000' ;
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->aut_name) {
        sprintf(&items[n_items][0], "aut:%.40s", header->aut_name);
        for ( pt_3 = &items[n_items][0]; *pt_3 != '\000'; pt_3 ++ ) {
    	/* --- convertit les separateurs en soulignes --- */
    	if (*pt_3==' '||*pt_3==','||*pt_3==';'||*pt_3=='\t'||*pt_3=='\n') 
    	    *pt_3 = '_' ;
        }
        *pt_3++ = ' ' ;
        *pt_3++ = '\000' ;
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->id) {
        sprintf(&items[n_items][0], "id:%.40s", header->id);
        for ( pt_3 = &items[n_items][0]; *pt_3 != '\000'; pt_3 ++ ) {
    	/* --- convertit les separateurs en soulignes --- */
    	if (*pt_3==' '||*pt_3==','||*pt_3==';'||*pt_3=='\t'||*pt_3=='\n') 
    	    *pt_3 = '_' ;
        }
        *pt_3++ = ' ' ;
        *pt_3++ = '\000' ;
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }
    if (bool_header->date) {
        sprintf(&items[n_items][0], "date:%.40s", header->date);
        for ( pt_3 = &items[n_items][0]; *pt_3 != '\000'; pt_3 ++ ) {
    	/* --- convertit les separateurs en soulignes --- */
    	if (*pt_3==' '||*pt_3==','||*pt_3==';'||*pt_3=='\t'||*pt_3=='\n') 
    	    *pt_3 = '_' ;
        }
        *pt_3++ = ' ' ;
        *pt_3++ = '\000' ;
        l_item[n_items] = strlen(&items[n_items][0]);
        n_items++;
    }

    n_car_total = n_comment = 0;
    if (bool_header->comment) {
        comment = header->comment;
        while (*comment != NULL) {
    	n_comment++;
    	n_car_total += strlen(*comment++) + 3;
        }
    }

    /*
        Ecrit tous les items, sauf les commentaires
    */
    for (i=0;i<n_items;i++) {
        if (n_car+l_item[i]<71) {		/*  Ajoute a la ligne courante s'il
    					    reste de la place */
    	strcat(ligne, &items[i][0]);
    	n_car += l_item[i];
        }
        else {				/*  Sinon, ecrit cette ligne et 
    					    commence-en une autre */
    	ligne[n_car++] = '\r';
    	ligne[n_car++] = '\n';
    	ligne[n_car]   = '\000';
    	n_car_total += n_car;
        
        if( n_car > n-*k ) {
            if( !Header_Quiet)
                fprintf( stderr, "\nHeader_Serialize: n_car (%d) > n-*k (%d-%d)\n",
                                  n_car, n,*k );
            // FIXME Overflow?
            sprintf( Header_Message, "\nHeader_Serialize: n_car (%d) > n-*k (%d-%d)\n",
                                  n_car, n,*k );
            return 0;
        }

        memcpy( buf+*k, ligne, n_car*sizeof(char) );
        *k += n_car;
    	
        strcpy(ligne, &items[i][0]);
    	n_car = l_item[i];
    	n_lignes++;
        }
    }

    /*
        Termine la derniere ligne
    */
    ligne[n_car++] = '\r';
    ligne[n_car++] = '\n';
    ligne[n_car]   = '\000';
    n_car_total += n_car;

    if( n_car > n-*k ) {
        if( !Header_Quiet)
            fprintf( stderr, "\nHeader_Serialize: n_car (%d) > n-*k (%d-%d)\n",
                              n_car, n,*k );
        // FIXME Overflow?
        sprintf( Header_Message, "\nHeader_Serialize: n_car (%d) > n-*k (%d-%d)\n",
                              n_car, n,*k );
        return 0;
    }

    memcpy( buf+*k, ligne, n_car*sizeof(char) );    *k += n_car;

        n_lignes++;

    /*
        Determine le nombre de caracteres et de lignes supplementaires 
        necessaires
    */
    n_blocs   = 1 + (n_car_total-1)/1024;
    n_car_sup = n_blocs*1024 - n_car_total;
    if (n_car_sup>0) {
        n_lig_sup = 1 + (n_car_sup-1)/72;
    }
    else {
        n_lig_sup = 0;
    }
    n_car_dl  = 1 + (n_car_sup-1)%72;

    /*
        Ecrit les commentaires
    */
    if (bool_header->comment) {
        comment = header->comment;
        while (*comment != NULL) {
            i = snprintf( buf+*k, n-*k, "#%.80s\r\n", *comment++ );

            if( i >= n-*k ) {
                if( !Header_Quiet ) {   // Comments not supported at the moment
                    fprintf( stderr, "\nHeader_Serialize: Failed writing comment\n" );
                }
                sprintf( Header_Message, "\nnHeader_Serialize: Failed writing comment\n" );
            }

            *k += i;
        }
    } 

    /*
        Complete l'entete a un multiple de 1024 caracteres
    */
    for (i=0;i<70;i++) ligne[i] = ' ';
    ligne[70] = '\r';
    ligne[71] = '\n';
    ligne[72] = '\000';

    for( i = 0; i < n_lig_sup-1; ++i ) {
        if( 72 > n-*k ) {
            if( !Header_Quiet)
                fprintf( stderr, "\nHeader_Serialize: 72 > n-*k (%d-%d)\n",
                                  n,*k );
            // FIXME Overflow?
            sprintf( Header_Message, "\nHeader_Serialize: 72 > n-*k (%d-%d)\n",
                                  n,*k );
            return 0;
        }

        memcpy( buf+*k, ligne, 72*sizeof(char) );
        *k += 72;
    }

    /*
        La derniere ligne se termine par un saut de page (FF)
    */
    for (i=0;i<n_car_dl-2;i++) ligne[i] = ' ';
        if (n_car_dl>2) ligne[n_car_dl-3] = '\r';
        if (n_car_dl>1) ligne[n_car_dl-2] = '\n';
        ligne[n_car_dl-1] = FF;
        ligne[n_car_dl] = '\000';

    if( n_car_dl > n-*k ) {
        if( !Header_Quiet)
            fprintf( stderr, "\nHeader_Serialize: n_car_dl (%d) > n-*k (%d-%d)\n",
                                n_car_dl, n,*k );
        // FIXME Overflow?
        sprintf( Header_Message, "\nHeader_Serialize: n_car_dl (%d) > n-*k (%d-%d)\n",
                                n_car_dl, n,*k );
        return 0;
    }

    memcpy( buf+*k, ligne, n_car_dl*sizeof(char) );
    *k += n_car_dl;        

    if (n_car_total>1024) {
        if (!Header_Quiet) 
    	fprintf(stderr,
    	 "\nHeader_Write ATTENTION: etiquette de grandeur non-standard \n");
        sprintf(Header_Message,
         "\nHeader_Write ATTENTION: etiquette de grandeur non-standard \n");
        statut |= GRANDEUR_INV;
    }
    else {
        sprintf(Header_Message, 
    	"\nHeader_Write: Entete transcrite sans problemes\n");
    }

    if (bool_header->vect_z) {

        if (header->vect_z==NULL) {
    	if (!Header_Quiet) 
    	    fprintf(stderr,"\nERREUR: vect_z nul\n");
    	sprintf(Header_Message,	"\nERREUR: vect_z nul\n");
    	return(0) ;
        }
        if (header->architecture!=ARCHITECTURE) {
    	for ( i=0; i<header->z; i++ ) {
    	   f_c.f = header->vect_z[i];
    	   for ( pt_1=&f_c.c[0],pt_2=&f_c.c[sizeof(float)-1];
    	    pt_1<pt_2; pt_1++,pt_2-- ) {
    		tmp = *pt_1;
    		*pt_1 = *pt_2;
    		*pt_2 = tmp;
    	    }
    	    header->vect_z[i] = f_c.f;
    	}
        }

        for( i = 0; i < header->z; ++i ) {
            if( *k == n ) {
                if( !Header_Quiet)
                    fprintf( stderr, "\nHeader_Serialize: ran out of buffer space"
                                     "while writing header->vec_z\n" );
                // FIXME Overflow?
                sprintf( Header_Message, "\nHeader_Serialize: ran out of buffer space"
                                         "while writing header->vec_z\n" );
                return 0;                
            }

            for( j = 0; j < sizeof(float); ++j )
                buf[(*k)++] = ( (char* )&header->vect_z[i] )[j];
        }

        if (header->architecture!=ARCHITECTURE) {
    	for ( i=0; i<header->z; i++ ) {
    	   f_c.f = header->vect_z[i];
    	   for ( pt_1=&f_c.c[0],pt_2=&f_c.c[sizeof(float)-1];
    	    pt_1<pt_2; pt_1++,pt_2-- ) {
    		tmp = *pt_1;
    		*pt_1 = *pt_2;
    		*pt_2 = tmp;
    	    }
    	    header->vect_z[i] = f_c.f;
    	}
        }
    }

    return (statut);}


/*---------------------------------------------------------------------------*\

    fonction:   Header_Write()

    description:Ecrit le header d'un fichier d'image
                l'information vient dans une structure de type Header
                les elements a transcrire sont indiques par une
                structure de type Bool_Header

    parametres: file (FILE *)               output stream
                header (Header *)           information
                bool_header (Bool_Header *) ce qui sera ecrit

    retour:     (int)       0           si erreur
                                        bit 1               si etiquette
                                                            ecrite
                                        bit GRANDEUR_INV    si grandeur de
                                                            l'etiquette 
                                                            >1024

    revision:   29/01/2001    Andre Bleau, ing.

\*---------------------------------------------------------------------------*/

int Header_Write( FILE* file, Header* header, Bool_Header* bool_header ) {
    char buf[1024];
    int  k, e;

    /* Here we assume that the buffer will fit into 1024 bytes.
       According to Mark Potse this is pretty much the only size you
       will see around.
       In case it is not sufficient, it is easy to increase the
       size (if a feasible compile-time upper bound is known -- it
       should be ;) ) or iterate over the size until it fits. */

    if( 0 == ( e = Header_Serialize( header, bool_header, buf, 1024, &k )))
        return e;

    fwrite( buf, sizeof(char), k, file );
    return e;
}

/*--------------------------------------------------------------------------*\

    fonction:	Ini_comment

    description:Initialise le membre ".comment" d'une structure Header

    parametres:	header_ptr (Header *) pointeur a la structure
  				      de type Header

    revision:	29/01/2001    Andre Bleau, ing.

\*--------------------------------------------------------------------------*/

void	Ini_comment(
    Header	*header_ptr
) {

header_ptr->comment = (char **) malloc( sizeof(char *) ) ;
header_ptr->comment[0] = NULL ;

}

/*--------------------------------------------------------------------------*\

    fonction:	Ajoute_ligne

    description:Ajoute une ligne de commentaire a une structure Header

    parametres:	header_ptr (Header *) pointeur a la structure
  				      de type Header
  		ligne (*char) 	      la ligne ajoutee

    revision:	29/01/2001    Andre Bleau, ing.

\*--------------------------------------------------------------------------*/

void	Ajoute_ligne(
    Header	*header_ptr,
    char	*ligne
) {

register    int
    i = 0, 
    n_lignes = 0;

while (header_ptr->comment[i++]!=NULL) n_lignes++;

header_ptr->comment[n_lignes] = malloc(strlen(ligne) + 1) ;
strcpy(header_ptr->comment[n_lignes++], ligne) ;

header_ptr->comment = 
    (char **)realloc(header_ptr->comment, (n_lignes+1)*sizeof(char *));
header_ptr->comment[n_lignes] = NULL;

}

/*--------------------------------------------------------------------------*\

    fonction:	Aj_ligne_com

    description:Ajoute un vecteur de chaines aux commentaires d'une
  		structure Header

    parametres:	header_ptr (Header *) pointeur a la structure
  				      de type Header
  		argc (int) le nombre d'elements du vecteur
  		argv (char **) le vecteurs de chaines

    retour: 	le nombre de lignes ajoutees

    revision:	29/01/2001    Andre Bleau, ing.

\*--------------------------------------------------------------------------*/

int	Aj_ligne_com(
    Header	*header_ptr,
    int		argc,
    char	*argv[]
) {

auto	    int
    i,
    longueur,
    n_lignes = 0;
auto	    char
    ligne[81];

if (argc<1) return(0);

strcpy(ligne, argv[0]);

for (i=1;i<argc;i++) {
    longueur = strlen(ligne) + strlen(argv[i]) + 1;
    if (longueur+1 > 72) {
	strcat(ligne, " \\");
	Ajoute_ligne(header_ptr, ligne);
	sprintf(ligne, " %s", argv[i]);
	n_lignes++;
    }
    else {
	strcat(ligne, " ");
	strcat(ligne, argv[i]);
    }
}
Ajoute_ligne(header_ptr, ligne);
n_lignes++;

return(n_lignes);

}

/*--------------------------------------------------------------------------*\

    fonction:	Retire_lignes

    description:Retire des lignes de commentaire a une structure Header

    parametres:	header_ptr (Header *) pointeur a la structure
  				      de type Header
  		n_lig_ret (int)       le nombre de lignes a retirer

    revision:	29/01/2001    Andre Bleau, ing.

\*--------------------------------------------------------------------------*/

void	Retire_lignes(
    Header	*header_ptr,
    int		n_lig_ret
) {

register    int
    i = 0, 
    n_lignes = 0;

while (header_ptr->comment[i++]!=NULL) n_lignes++;
if (n_lignes<n_lig_ret) n_lig_ret = n_lignes;

while (n_lig_ret-- > 0) {
    free (header_ptr->comment[--n_lignes]);
}

header_ptr->comment[n_lignes] = NULL;

}

/*--------------------------------------------------------------------------*\

    fonction:	Ajoute_com

    description:Ajoute les commentaires d'une structure Header a ceux
  		d'une autre structure Header

    parametres:	header_ptr1 (Header *) pointeur a la structure
  				       cible de type Header
  		header_ptr2 (Header *) pointeur a la structure
  				       source de type Header
  		indent (int)           le nombre d'espace dont
  				       on indente les commentaires
  				       de header_ptr2

    retour:     le nombre de lignes ajoutees

    revision:	29/01/2001    Andre Bleau, ing.

\*--------------------------------------------------------------------------*/

int	Ajoute_com(
    Header	*header_ptr1,
    Header	*header_ptr2,
    int		indent
) {

register    int
    i = 0,
    j,
    n_lignes1 = 0, 
    n_lignes2 = 0;

while (header_ptr1->comment[i++]!=NULL) n_lignes1++;
i = 0;
while (header_ptr2->comment[i++]!=NULL) n_lignes2++;

header_ptr1->comment = 
    (char **)realloc(header_ptr1->comment, 
		(n_lignes1+n_lignes2+1)*sizeof(char *));

i = -1;
while (header_ptr2->comment[++i]!=NULL) {
    header_ptr1->comment[n_lignes1] = 
	malloc(strlen(header_ptr2->comment[i]) + indent + 1);
    header_ptr1->comment[n_lignes1][0] = '\0';
    for (j=0;j<indent;j++) strcat(header_ptr1->comment[n_lignes1], " ");
    strcat(header_ptr1->comment[n_lignes1++], header_ptr2->comment[i]);
}

header_ptr1->comment[n_lignes1] = NULL;

return(n_lignes2);

}

/*--------------------------------------------------------------------------*\

    fonction:	Header_Cpy

    description:Copie les membres d'une structure Header dans
  		une autre structure Header a l'exception des commentaires

    parametres:	header_ptr1 (Header *) pointeur a la structure
  				       cible de type Header
  		header_ptr2 (Header *) pointeur a la structure
  				       source de type Header

    revision:	29/01/2001    Andre Bleau, ing.

\*--------------------------------------------------------------------------*/

void	Header_Cpy(
    Header	*header_ptr1,
    Header	*header_ptr2
) {

if (header_ptr1==NULL || header_ptr2==NULL) return;

header_ptr1->x             = header_ptr2->x;
header_ptr1->y	           = header_ptr2->y;
header_ptr1->z	           = header_ptr2->z;
header_ptr1->t	           = header_ptr2->t;
header_ptr1->type          = header_ptr2->type;
header_ptr1->taille        = header_ptr2->taille;
header_ptr1->architecture  = header_ptr2->architecture;
header_ptr1->systeme       = header_ptr2->systeme;
header_ptr1->bin           = header_ptr2->bin;
header_ptr1->trame         = header_ptr2->trame;
header_ptr1->num           = header_ptr2->num;
header_ptr1->epais         = header_ptr2->epais;
header_ptr1->org_x         = header_ptr2->org_x;
header_ptr1->org_y         = header_ptr2->org_y;
header_ptr1->org_z         = header_ptr2->org_z;
header_ptr1->org_t         = header_ptr2->org_t;
header_ptr1->inc_x         = header_ptr2->inc_x;
header_ptr1->inc_y         = header_ptr2->inc_y;
header_ptr1->inc_z         = header_ptr2->inc_z;
header_ptr1->inc_t         = header_ptr2->inc_t;
header_ptr1->dim_x         = header_ptr2->dim_x;
header_ptr1->dim_y         = header_ptr2->dim_y;
header_ptr1->dim_z         = header_ptr2->dim_z;
header_ptr1->dim_t         = header_ptr2->dim_t;
header_ptr1->fac_x         = header_ptr2->fac_x;
header_ptr1->fac_y         = header_ptr2->fac_y;
header_ptr1->fac_z         = header_ptr2->fac_z;
header_ptr1->fac_t         = header_ptr2->fac_t;
header_ptr1->vect_z        = header_ptr2->vect_z;
strcpy(header_ptr1->unites_x,header_ptr2->unites_x);
strcpy(header_ptr1->unites_y,header_ptr2->unites_y);
strcpy(header_ptr1->unites_z,header_ptr2->unites_z);
strcpy(header_ptr1->unites_t,header_ptr2->unites_t);
header_ptr1->lut           = header_ptr2->lut;
header_ptr1->comp          = header_ptr2->comp;
strcpy(header_ptr1->unites,  header_ptr2->unites);
header_ptr1->facteur       = header_ptr2->facteur;
header_ptr1->zero          = header_ptr2->zero;
strcpy(header_ptr1->aut_name,header_ptr2->aut_name);
strcpy(header_ptr1->id,      header_ptr2->id);
strcpy(header_ptr1->date,    header_ptr2->date);

}

/*--------------------------------------------------------------------------*\

    fonction:	Bool_Header_Cpy

    description:Copie les membres d'une structure Bool_Header dans
  		une autre structure Bool_Header,
                a l'exception du champs commentaires

    parametres:	b_header_ptr1 (Bool_Header *) pointeur a la structure
  				              cible de type Bool_Header
  		b_header_ptr2 (Bool_Header *) pointeur a la structure
  				              source de type Bool_Header

    revision:	29/01/2001    Andre Bleau, ing.

\*--------------------------------------------------------------------------*/

void	Bool_Header_Cpy(
    Bool_Header	*b_header_ptr1,
    Bool_Header	*b_header_ptr2
) {

if (b_header_ptr1==NULL || b_header_ptr2==NULL) return;

b_header_ptr1->x             = b_header_ptr2->x;
b_header_ptr1->y             = b_header_ptr2->y;
b_header_ptr1->z             = b_header_ptr2->z;
b_header_ptr1->t             = b_header_ptr2->t;
b_header_ptr1->type          = b_header_ptr2->type;
b_header_ptr1->taille        = b_header_ptr2->taille;
b_header_ptr1->architecture  = b_header_ptr2->architecture;
b_header_ptr1->systeme       = b_header_ptr2->systeme;
b_header_ptr1->bin           = b_header_ptr2->bin;
b_header_ptr1->trame         = b_header_ptr2->trame;
b_header_ptr1->num           = b_header_ptr2->num;
b_header_ptr1->epais         = b_header_ptr2->epais;
b_header_ptr1->org_x         = b_header_ptr2->org_x;
b_header_ptr1->org_y         = b_header_ptr2->org_y;
b_header_ptr1->org_z         = b_header_ptr2->org_z;
b_header_ptr1->org_t         = b_header_ptr2->org_t;
b_header_ptr1->inc_x         = b_header_ptr2->inc_x;
b_header_ptr1->inc_y         = b_header_ptr2->inc_y;
b_header_ptr1->inc_z         = b_header_ptr2->inc_z;
b_header_ptr1->inc_t         = b_header_ptr2->inc_t;
b_header_ptr1->dim_x         = b_header_ptr2->dim_x;
b_header_ptr1->dim_y         = b_header_ptr2->dim_y;
b_header_ptr1->dim_z         = b_header_ptr2->dim_z;
b_header_ptr1->dim_t         = b_header_ptr2->dim_t;
b_header_ptr1->fac_x         = b_header_ptr2->fac_x;
b_header_ptr1->fac_y         = b_header_ptr2->fac_y;
b_header_ptr1->fac_z         = b_header_ptr2->fac_z;
b_header_ptr1->fac_t         = b_header_ptr2->fac_t;
b_header_ptr1->vect_z        = b_header_ptr2->vect_z;
b_header_ptr1->unites_x      = b_header_ptr2->unites_x;
b_header_ptr1->unites_y      = b_header_ptr2->unites_y;
b_header_ptr1->unites_z      = b_header_ptr2->unites_z;
b_header_ptr1->unites_t      = b_header_ptr2->unites_t;
b_header_ptr1->lut           = b_header_ptr2->lut;
b_header_ptr1->comp          = b_header_ptr2->comp;
b_header_ptr1->unites        = b_header_ptr2->unites;
b_header_ptr1->facteur       = b_header_ptr2->facteur;
b_header_ptr1->zero          = b_header_ptr2->zero;
b_header_ptr1->aut_name      = b_header_ptr2->aut_name;
b_header_ptr1->id            = b_header_ptr2->id;
b_header_ptr1->date          = b_header_ptr2->date;

}

/*--------------------------------------------------------------------------*\

    fonction:	Header_Clear

    description:Libere les membres alloues d'une structure Header

    parametres:	header_ptr (Header *) pointeur a la structure
  				      cible de type Header

    revision:	29/01/2001    Andre Bleau, ing.

\*--------------------------------------------------------------------------*/

void	Header_Clear(
    Header	*header_ptr
) {

auto	int
    i = 0;
    
if (header_ptr==NULL) return;

if (header_ptr->vect_z) {
    free(header_ptr->vect_z);
    header_ptr->vect_z = NULL;
}
if (header_ptr->comment) {
    while (header_ptr->comment[i])
	free(header_ptr->comment[i++]);
    free(header_ptr->comment);
    header_ptr->comment = NULL;
}

}

/*--------------------------------------------------------------------------*\

    fonction:	Header_Ini

    description:Initialise les membres d'une structure Header

    parametres:	header_ptr (Header *) pointeur a la structure
  				      cible de type Header

    revision:	29/01/2001    Andre Bleau, ing.

\*--------------------------------------------------------------------------*/

void	Header_Ini(
    Header	*header_ptr
) {

if (header_ptr==NULL) return;

header_ptr->x = header_ptr->y = header_ptr->type = 0 ;
header_ptr->architecture = ARCHITECTURE;
header_ptr->systeme = SYSTEME;
header_ptr->comp = header_ptr->lut = 
header_ptr->num = header_ptr->bin = 0 ;
header_ptr->trame = C8 ;
header_ptr->z = header_ptr->t = 1 ;
header_ptr->epais = 0.0 ;
header_ptr->inc_x = header_ptr->inc_y = 
header_ptr->inc_z = header_ptr->inc_t = 1.0 ;
header_ptr->org_x = header_ptr->org_y = 
header_ptr->org_z = header_ptr->org_t = 1.0 ;
header_ptr->fac_x = header_ptr->fac_y = 
header_ptr->fac_z = header_ptr->fac_t = 1.0 ;
header_ptr->vect_z = NULL ;
header_ptr->unites_x[0] = header_ptr->unites_x[40] = '\000' ;
header_ptr->unites_y[0] = header_ptr->unites_y[40] = '\000' ;
header_ptr->unites_z[0] = header_ptr->unites_z[40] = '\000' ;
header_ptr->unites_t[0] = header_ptr->unites_t[40] = '\000' ;
header_ptr->unites[0] = header_ptr->unites[40] = '\000' ;
header_ptr->facteur = 1.0 ;
header_ptr->zero = 0.0 ;
header_ptr->aut_name[0] = header_ptr->aut_name[40] = '\000' ;
header_ptr->struct_desc[0] = header_ptr->struct_desc[40] = '\000' ;
header_ptr->id[0] = header_ptr->id[40] = '\000' ;
header_ptr->date[0] = header_ptr->date[10] = '\000' ;
header_ptr->comment = (char **) malloc( NALLOC*sizeof(char *) ) ;
header_ptr->comment[0] = NULL ;

}

/*--------------------------------------------------------------------------*\

    fonction:	Bool_Header_Ini

    description:Initialise les membres d'une structure Bool_Header
    
    parametres:	b_header_ptr (Bool_Header *) pointeur a la structure
  				             cible de type Bool_Header

    revision:	29/01/2001    Andre Bleau, ing.

\*--------------------------------------------------------------------------*/

void	Bool_Header_Ini(
    Bool_Header	*b_header_ptr
) {

if (b_header_ptr==NULL) return;

b_header_ptr->x             = FAUX;
b_header_ptr->y             = FAUX;
b_header_ptr->z             = FAUX;
b_header_ptr->t             = FAUX;
b_header_ptr->type          = FAUX;
b_header_ptr->taille        = FAUX;
b_header_ptr->architecture  = FAUX;
b_header_ptr->systeme       = FAUX;
b_header_ptr->bin           = FAUX;
b_header_ptr->trame         = FAUX;
b_header_ptr->num           = FAUX;
b_header_ptr->epais         = FAUX;
b_header_ptr->org_x         = FAUX;
b_header_ptr->org_y         = FAUX;
b_header_ptr->org_z         = FAUX;
b_header_ptr->org_t         = FAUX;
b_header_ptr->inc_x         = FAUX;
b_header_ptr->inc_y         = FAUX;
b_header_ptr->inc_z         = FAUX;
b_header_ptr->inc_t         = FAUX;
b_header_ptr->dim_x         = FAUX;
b_header_ptr->dim_y         = FAUX;
b_header_ptr->dim_z         = FAUX;
b_header_ptr->dim_t         = FAUX;
b_header_ptr->fac_x         = FAUX;
b_header_ptr->fac_y         = FAUX;
b_header_ptr->fac_z         = FAUX;
b_header_ptr->fac_t         = FAUX;
b_header_ptr->vect_z        = FAUX;
b_header_ptr->unites_x      = FAUX;
b_header_ptr->unites_y      = FAUX;
b_header_ptr->unites_z      = FAUX;
b_header_ptr->unites_t      = FAUX;
b_header_ptr->lut           = FAUX;
b_header_ptr->comp          = FAUX;
b_header_ptr->unites        = FAUX;
b_header_ptr->facteur       = FAUX;
b_header_ptr->zero          = FAUX;
b_header_ptr->aut_name      = FAUX;
b_header_ptr->id            = FAUX;
b_header_ptr->date          = FAUX;

}

/*--------------------------------------------------------------------------*\

    fonction:	Img_Ext()

    description:Donne l'extension a utiliser pour les fichiers d'images
  		en fonction de la variable d'environnement IMG_EXT et de
  		la valeur par defaut definie dans header.h.

    parametres:	aucun

    retour:	(char *)	Pointeur statique a la chaine de
   				caracteres

    revision:	29/01/2001    Andre Bleau, ing.

\*--------------------------------------------------------------------------*/

char	    *Img_Ext(void)

{

static	    char
    *extension;		/*  Extension pour les fichiers d'images */

if (extension==NULL) {
    extension = getenv("IMG_EXT");
    if (extension==NULL) {
	extension = malloc(strlen(IMG_EXT)+1);
	strcpy(extension, IMG_EXT);
    }
}
return(extension);

}

/*---------------------------------------------------------------------------*\

    fonction:	ImageInvalide()  (int)

    description:Retourne 0 si le nom donne en argument est celui d'un fichier
		contenant une image valide (lisible); retourne 1 sinon.

    parametres:	nom (char *)	le nom du fichier a verifier

    retour:	int		0 (c'est bien une image) ou 
				1 (ce n'est pas une image)

    revision:	29/01/2001    Andre Bleau, ing.

\*---------------------------------------------------------------------------*/

int	    ImageInvalide(
    char	*nom
) {

static	    char
    nom_complet[257];
static	    Header
    header;
static	    Bool_Header
    bool_header;
auto	    FILE
    *df_entree;
auto	    int
    quiet,
    statut;

strcpy(nom_complet, nom);
strcat(nom_complet, Img_Ext());
df_entree = fopen(nom_complet, "r");
if (df_entree==NULL) {
    strcpy(nom_complet, nom);
    df_entree = fopen(nom_complet, "r");
}
if (df_entree==NULL) {
    return(1);
}
quiet = Header_Quiet;
Header_Quiet = 1;
statut = Header_Read(df_entree, &header, &bool_header);
Header_Quiet = quiet;
if (statut==0) {
    return(1);
}
else {
    return(0);
}

}


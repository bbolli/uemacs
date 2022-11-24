/*
 * The routines in this file provide support for the IBM-PC and other
 * compatible terminals. It goes directly to the graphics RAM to do
 * screen output. It compiles into nothing if not an IBM-PC driver
 * Supported monitor cards include CGA, MONO and EGA/VGA.
 */

#define termdef 1			/* don't define "term" external */

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

#if	IBMPC
#define NROW	60			/* Max Screen size.		*/
#define NCOL	132			/* Edit if you want to. 	*/
#define MARGIN	4			/* size of minimum margin and	*/
#define SCRSIZ	60			/* scroll size for extended lines */
#define NPAUSE	200			/* # times thru update to pause */
#define BEL	'\x07'                  /* BEL character.               */
#define ESC	'\x1B'                  /* ESC character.               */
#define SPACE	' '                     /* space character              */

#define SCADC	0xB8000000L		/* CGA address of screen RAM	*/
#define SCADM	0xB0000000L		/* MONO address of screen RAM	*/
#define SCADE	0xB8000000L		/* EGA address of screen RAM	*/

#define MONOCRSR 0x0B0D 		/* monochrome cursor	    */
#define CGACRSR 0x0607			/* CGA cursor		    */
#define EGACRSR 0x0709			/* EGA cursor		    */

#define CDCGA	0			/* color graphics card		*/
#define CDMONO	1			/* monochrome text card 	*/
#define CDVGA28 2			/* VGA adapter 28 rows		*/
#define CDEGA35 3			/* EGA/VGA color adapter 35/40 rows	*/
#define CDEGA43 4			/* EGA/VGA color adapter 43/50 rows	*/
#define CDSENSE 9			/* detect the card type 	*/

#define NDRIVE	5			/* number of screen drivers	*/

#define CHSENSE -1

int oldtype = -1;			/* display type before initializetion */
int dtype = -1; 			/* current display type 	*/
char drvname[][8] = {			/* screen resolution names	*/
	"CGA", "MONO", "VGA28", "EGA35", "EGA43"
};
long scadd;				/* address of screen ram	*/
int *scptr[NROW];			/* pointer to screen lines	*/
unsigned int sline[NCOL];		/* screen line image		*/
int egaexist = FALSE;			/* is an EGA card available?	*/
int cheight = 2;			// cursor height
extern union REGS rg;			/* cpu register for use of DOS calls */

extern	int	ttopen();		/* Forward references.		*/
extern	int	ttputc();
extern	int	ttflush();
extern	int	ttclose();
extern	int	ibmmove();
extern	int	ibmeeol();
extern	int	ibmeeop();
extern	int	ibmbeep();
extern	int	ibmopen();
extern	int	ibmrev();
extern	int	ibmcres();
extern	int	ibmclose();
extern	int	ibmputc();
extern	int	ibmkopen();
extern	int	ibmkclose();
extern	int	ibmgetc(), ttgetc();

#if	COLOR
extern	int	ibmfcol();
extern	int	ibmbcol();

int	cfcolor = -1;		/* current foreground color */
int	cbcolor = -1;		/* current background color */
int	ctrans[] =		/* ansi to ibm color translation table */
	{0, 4, 10, 6, 1, 5, 3, 7};
#endif

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
TERM	term	= {
	NROW-1,
	NROW-1,
	NCOL,
	80,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	ibmopen,
	ibmclose,
	ibmkopen,
	ibmkclose,
#if	MSC | TURBO
	ibmgetc,
#else
	ttgetc,
#endif
	ibmputc,
	ttflush,
	ibmmove,
	ibmeeol,
	ibmeeop,
	ibmbeep,
	ibmrev,
	ibmcres
#if	COLOR
      , ibmfcol,
	ibmbcol
#endif
};

void flickwait()  {

	if ( dtype == CDCGA )  {
		/* wait for vertical retrace to be off */
		while ( inp( 0x3DA ) & 8 )
			;
		/* and to be back on */
		while ( !( inp( 0x3DA ) & 8 ) )
			;
	}
}

void intr( intno )  {

	static union REGS *rgp = &rg;

	int86( intno, rgp, rgp );
}

void intv()  {  intr( 0x10 );  }

void intk()  {  intr( 0x16 );  }

void intd()  {  intr( 0x21 );  }


#if	COLOR
ibmfcol(color)		/* set the current output color */
int color;	/* color to set */
{
	cfcolor = ctrans[color];
}

ibmbcol(color)		/* set the current background color */
int color;	/* color to set */
{
	cbcolor = ctrans[color];
}
#endif

ibmmove(row, col)
{
	rg.h.ah = 2;		/* set cursor position function code */
	rg.h.dl = col;
	rg.h.dh = row;
	rg.h.bh = 0;		/* set screen page number */
	intv();
}

ibmeeol()	/* erase to the end of the line */
{
	unsigned int attr;	/* attribute byte mask to place in RAM */
	unsigned int *lnptr;	/* pointer to the destination line */
	int i;
	int ccol;	/* current column cursor lives */
	int crow;	/*	   row	*/

	/* find the current cursor position */
	rg.h.ah = 3;		/* read cursor position function code */
	rg.h.bh = 0;		/* current video page */
	intv();
	ccol = rg.h.dl; 	/* record current column */
	crow = rg.h.dh; 	/* and row */

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	if (dtype != CDMONO)
		attr = (((cbcolor & 15) << 4) | (cfcolor & 15)) << 8;
	else
		attr = 0x0700;
#else
	attr = 0x0700;
#endif
	lnptr = &sline[0];
	for (i=0; i < term.t_ncol; i++)
		*lnptr++ = SPACE | attr;

	if (flickcode) flickwait();

	/* and send the string out */
	movmem(&sline[0], scptr[crow]+ccol, (term.t_ncol-ccol)*2);

}

ibmputc(ch)	/* put a character at the current position in the
int ch; 	   current colors */
{
	rg.h.ah = 14;		/* write char to screen with current attrs */
	rg.h.al = ch;
#if	COLOR
	if (dtype != CDMONO)
		rg.h.bl = cfcolor;
	else
		rg.h.bl = 0x07;
#else
	rg.h.bl = 0x07;
#endif
	intv();
}

ibmeeop()
{
	int attr;		/* attribute to fill screen with */

	rg.x.ax = 0x0600;	/* clear window function code */
	rg.x.cx = 0;		/* upper left corner of scroll */
	rg.x.dx = (term.t_nrow << 8) | (term.t_ncol - 1);
				/* lower right corner of scroll */
#if	COLOR
	if (dtype != CDMONO)
		attr = ((ctrans[gbcolor] & 15) << 4) | (ctrans[gfcolor] & 15);
	else
		attr = 0;
#else
	attr = 0;
#endif
	rg.h.bh = attr;
	intv();
}

ibmrev(state)		/* change reverse video state */
int state;		/* TRUE = reverse, FALSE = normal */
{
	/* This never gets used under the IBM-PC driver */
}

ibmcres(res)	/* change screen resolution */
char *res;	/* resolution to change to */
{
	int i;		/* index */

	for (i = 0; i < NDRIVE; i++)
		if (stricmp(res, drvname[i]) == 0) {
			scinit(i);
			return(TRUE);
		}
	return(FALSE);
}

spal()	/* reset the palette registers */
{
	/* nothin here now..... */
}

ibmbeep()
{
#if	MWC86
	putcnb(BEL);
#else
	ibmputc(BEL);
#endif
}

ibmopen()
{
	scinit(CDSENSE);
	oldtype = dtype;
	revexist = TRUE;
	cheight = curheight(CHSENSE);

	ttopen();
}

ibmclose()
{
#if	COLOR
	ibmfcol(7);
	ibmbcol(0);
#endif
	/* if we have changed the display type... restore it */
	if (dtype != oldtype)
		scinit(oldtype);

	curheight(cheight);

	ttclose();
}


int ibm_xkeyb;

ibmkopen()	/* open the keyboard */
{
#if MSC | TURBO
	/* test for presence of an extended keyboard BIOS:
	 * get extended shift key status with AL preset to
	 * a highly unprobable return value:
	 * both shift keys, ctrl, alt pressed and all toggles on
	 */
	rg.x.ax = 0x12FF;
	intk();
	/* if AL is unchanged, then we probably have an old BIOS because
	 * an old BIOS wouldn't try to handle function call 0x12
	 */
	ibm_xkeyb = ( rg.h.al != 0xFF ) ? 0x10 : 0;
	/* see also typahead() in termio.c */
#else
	ibm_xkeyb = 0;
#endif
}

ibmkclose()	/* close the keyboard */
{
}

#if MSC | TURBO
ibmgetc()	// get a character
{
	int c;

	rg.h.ah = ibm_xkeyb;
	intk();

	// map duplicate cursor keys to the standard ones
	if ( ( unsigned char ) rg.h.al == 0xE0 && rg.h.ah )
		rg.h.al = 0;

	if ( rg.x.ax == 0x0300 )	// is this Ctrl-@ (ASCII NUL) ?
		c = 0;
	else if ( !rg.h.al )		// extended key code ?
		c = SPEC | ( unsigned char ) rg.h.ah;
	else
		c = ( unsigned char ) rg.h.al;

	// check for Ctrl-any unless Alt-nnn was typed
	if ( rg.h.ah && ( unsigned char ) c < ' ' )	// assumes ASCII!
		c |= CTRL | '@';

	return c;
}
#endif

scinit(type)	/* initialize the screen head pointers */
int type;	/* type of adapter to init for */
{
	union {
		long laddr;	/* long form of address */
		int *paddr;	/* pointer form of address */
	} addr;
	int i;

	/* if asked...find out what display is connected */
	if (type == CDSENSE)
		type = getboard();

	/* if we have nothing to do....don't do it */
	if (dtype == type)
		return(TRUE);

	/* if we try to switch to EGA and there is none, don't */
	if (type >= CDVGA28 && egaexist != TRUE)
		return(FALSE);

	/* if we had the EGA open... close it */
	if (dtype >= CDVGA28 && type < CDVGA28)
		egaclose();

	/* and set up the various parameters as needed */
	switch (type) {
		case CDMONO:	/* Monochrome adapter */
			scadd = SCADM;
			break;
		case CDCGA:	/* Color graphics adapter */
			scadd = SCADC;
			break;
		case CDEGA35:	/* Enhanced graphics adapter */
		case CDEGA43:
		case CDVGA28:	/* Video Graphics Array */
			scadd = SCADE;
			if (oldtype != -1)	/* we're not initializing */
				egaopen(type);	/* pass type (35/43/28 rows) */
			break;
	}

	/* determine the number of rows from the BIOS data area */
	if ( ( i = *(char far *) 0x0484L ) == 0 )
		i = 25;
	else if ( ++i > NROW )	/* failsave for VERY small fonts */
		i = NROW;
	newsize( TRUE, i );

	/* reset the $sres environment variable */
	strcpy(sres, drvname[type]);
	dtype = type;

	/* initialize the screen pointer array */
	for (i = 0; i < NROW; i++) {
		addr.laddr = scadd + (long)(term.t_ncol * i * 2);
		scptr[i] = addr.paddr;
	}
	return(TRUE);
}

/* getboard:	Determine which type of display board is attached.
		Current known types include:

		CDMONO	Monochrome graphics adapter
		CDCGA	Color Graphics Adapter/Enhanced Graphics Adapter 25 rows
		CDEGA43 Enhanced Graphics Adapter 43 rows
		CDEGA35 Enhanced Graphics Adapter 35 rows
		CDVGA28 Video Graphics Array 28 rows
*/
int getboard()
{
	int type;	/* board type to return */
	int nrows;	/* #screen rows from BIOS data area */

	nrows = * (unsigned char far *) 0x0484L + 1;
	term.t_ncol = * (unsigned char far *) 0x044AL;

	switch ( nrows )  {
		case 43:
		case 50:
			type = CDEGA43;  break;
		case 35:
		case 40:
			type = CDEGA35;  break;
		case 28:
			type = CDVGA28;  break;
		default:
			type = (* (char far *) 0x0449L == 7) ? CDMONO : CDCGA;	break;
	}

	/* test if EGA/VGA present */
	egaexist = (nrows > 1);

	return type;
}

egaopen(type)	/* init the computer to work with the EGA */
{
	if ( type == CDEGA35 )	{
		execprog( "NCC /40" );	// NCC must be on the PATH!
		return;
	}

	/* put the beast into EGA 43 row mode */
	rg.x.ax = 0x0003;
	intv();

	rg.x.ax = (type == CDEGA43) ? 0x1112 : 0x1111;
				/* set char. generator function code	*/
				/*  to 8 by 8 or 8 by 14 char. set	*/
	rg.h.bl = 0;		/* block 0				*/
	intv();

	rg.x.ax = 0x1200;	/* alternate select function code	*/
				/* clear AL for no good reason		*/
	rg.h.bl = 32;		/* alt. print screen routine		*/
	intv();

	rg.h.ah = 1;		/* set cursor size function code	*/
	rg.x.cx = 0x0607;	/* turn cursor on code			*/
	intv();

	outp(0x03D4, 10);	/* video bios bug patch 		*/
	outp(0x03D5, 6);
}

egaclose()  {

	/* put the beast back into 25 line mode */
	rg.x.ax = 3;
	intv();

	* ( char far * ) 0x0487L &= ~0x01;	/* turn on cursor emulation */
}

int curheight(height)  {

	rg.h.ah = 0x03; 		/* get cursor shape */
	rg.h.bl = 0;			/* page 0 */
	intv();
	if (height >= 0) {
		rg.h.ah = 0x01; 	/* set cursor shape */
		rg.h.ch = rg.h.cl - height + 1;
		intv();
	}
	return rg.h.cl - rg.h.ch + 1;
}

scwrite(row, outstr, forg, bacg)	/* write a line out */
int row;	/* row of screen to place outstr on */
char *outstr;	/* string to write out (must be term.t_ncol long) */
int forg;	/* foreground color of string to write */
int bacg;	/* background color */
{
	unsigned int attr;	/* attribute byte mask to place in RAM */
	unsigned int *lnptr;	/* pointer to the destination line */
	int i;

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	if (dtype != CDMONO)
		attr = (((ctrans[bacg] & 15) << 4) | (ctrans[forg] & 15)) << 8;
	else
		attr = (((bacg & 15) << 4) | (forg & 15)) << 8;
#else
	attr = (((bacg & 15) << 4) | (forg & 15)) << 8;
#endif
	lnptr = &sline[0];
	for ( i = 0; i < term.t_ncol; i++ )
		*lnptr++ = ( unsigned char ) *outstr++ | attr;

	if (flickcode) flickwait();

	/* and send the string out */
	movmem( &sline[0], scptr[ row ], term.t_ncol * 2 );
}

#if	FLABEL
fnclabel(f, n)		/* label a function key */
int f,n;	/* default flag, numeric argument [unused] */
{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif

#else

ibmhello()  {}

#endif

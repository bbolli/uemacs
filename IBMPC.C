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
#define CDEGA   2			/* EGA/VGA color 		*/
#define CDSENSE 9			/* detect the card type 	*/

#define NDRIVE	5			/* number of screen drivers	*/

#define CHSENSE -1

int oldtype = -1;			/* display type before initializetion */
int dtype = -1; 			/* current display type 	*/
char drvname[][8] = {			/* screen resolution names	*/
	"CGA", "MONO", "EGA"
};
long scadd;				/* address of screen ram	*/
int *scptr[NROW];			/* pointer to screen lines	*/
unsigned int sline[NCOL];		/* screen line image		*/
int egaexist = FALSE;			/* is an EGA card available?	*/
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
	ttgetc,
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
	int86(0x10, &rg, &rg);
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
	int86(0x10, &rg, &rg);
	ccol = rg.h.dl;		/* record current column */
	crow = rg.h.dh;		/* and row */

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
	int86(0x10, &rg, &rg);
}

ibmeeop()
{
	int attr;		/* attribute to fill screen with */

	rg.h.ah = 6;		/* scroll page up function code */
	rg.h.al = 0;		/* # lines to scroll (clear it) */
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
	int86(0x10, &rg, &rg);
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


ibmkopen()	/* open the keyboard */
{
}

ibmkclose()	/* close the keyboard */
{
}

{
}

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
	if (type == CDEGA && egaexist != TRUE)
		return(FALSE);

	/* if we had the EGA open... close it */
	if (dtype == CDEGA)
		egaclose();

	/* and set up the various parameters as needed */
	switch (type) {
		case CDMONO:	/* Monochrome adapter */
			scadd = SCADM;
			break;
		case CDCGA:	/* Color graphics adapter */
			scadd = SCADC;
			break;
		case CDEGA:	/* Enhanced graphics adapter */
			scadd = SCADE;
			if (oldtype != -1)	/* we're not initializing */
				egaopen();
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
		CDCGA	Color Graphics Adapter
		CDEGA	Extended graphics Adapter
*/

/* getbaord:	Detect the current display adapter
		if MONO		set to MONO
		   CGA		set to CGA	EGAexist = FALSE
		   EGA		set to CGA	EGAexist = TRUE
*/

int getboard()

{
	int type;	/* board type to return */

	type = CDCGA;
	int86(0x11, &rg, &rg);
	if ((((rg.x.ax >> 4) & 3) == 3))
		type = CDMONO;

	/* test if EGA present */
	rg.x.ax = 0x1200;
	rg.x.bx = 0xff10;
	int86(0x10,&rg, &rg);		/* If EGA, bh=0-1 and bl=0-3 */
	egaexist = !(rg.x.bx & 0xfefc);	/* Yes, it's EGA */
	return(type);
}

egaopen()	/* init the computer to work with the EGA */

{
	/* put the beast into EGA 43 row mode */
	rg.x.ax = 3;
	int86(16, &rg, &rg);

	rg.h.ah = 17;		/* set char. generator function code */
	rg.h.al = 18;		/*  to 8 by 8 double dot ROM         */
	rg.h.bl = 0;		/* block 0                           */
	int86(16, &rg, &rg);

egaclose()  {

	/* put the beast back into 25 line mode */
	rg.x.ax = 3;
	intv();

}

egaclose()  {

	/* put the beast back into 25 line mode */
	rg.x.ax = 3;
	int86(16, &rg, &rg);
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
	for (i=0; i<term.t_ncol; i++)
		*lnptr++ = (outstr[i] & 255) | attr;

	if (flickcode && (dtype == CDCGA)) {
		/* wait for vertical retrace to be off */
		while ((inp(0x3da) & 8))
			;
	
		/* and to be back on */
		while ((inp(0x3da) & 8) == 0)
			;
	}

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

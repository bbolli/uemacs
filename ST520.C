/*

The routines in this file provide support for the Atari 520 or 1040ST
using VT52 emulation.  The I/O services are provided here as well.  It
compiles into nothing if not a 520ST style device. The only compiler
supported directly is Mark Williams C

Additional code and ideas from:

		James Turner
		Jeff Lomicka
		J. C. Benoist

*/

#define	termdef	1			/* don't define "term" external */

#include        <stdio.h>
#include        "estruct.h"
#include	"edef.h"

#if	ATARI & ST520

/*
	These routines provide support for the ATARI 1040ST and 520ST
using the virtual VT52 Emulator

*/

#include	<osbind.h>
#include	<aline.h>
#include	<linea.h>

#define NROW	50	/* Screen size. 		*/
#define NCOL	80	/* Edit if you want to. 	*/
#define	MARGIN	8	/* size of minimim margin and	*/
#define	SCRSIZ	64	/* scroll size for extended lines */
#define	NPAUSE	300	/* # times thru update to pause */
#define BIAS	0x20	/* Origin 0 coordinate bias.	*/
#define ESC	0x1B	/* ESC character.		*/
#define SCRFONT 2	/* index of 8x16 monochrome system default font */
#define DENSIZE	50	/* # of lines in a dense screen	*/

/****	ST Internals definitions		*****/

/*	BIOS calls */

#define	BCONSTAT	1	/* return input device status */
#define	CONIN		2	/* read character from device */
#define	BCONOUT		3	/* write character to device */

/*	XBIOS calls */

#define	INITMOUS	0	/* initialize the mouse */
#define	GETREZ		4	/* get current resolution */
#define	SETSCREEN	5	/* set screen resolution */
#define	SETPALETTE	6	/* set the color pallette */
#define	SETCOLOR	7	/* set or read a color */
#define	CURSCONF	21	/* set cursor configuration */
#define	IKBDWS		25	/* intelligent keyboard send command */
#define	KBDVBASE	34	/* get keyboard table base */

/*	GEMDOS calls */

#define	EXEC		0x4b	/* Exec off a process */

#define	CON		2	/* CON: Keyboard and screen device */

/*	Palette color definitions	*/

#define	LOWPAL	"000700070770007707077777"
#define	MEDPAL	"000700007777"
#define	HIGHPAL	"111000"

/*	ST Global definitions		*/

/* keyboard vector table */
struct KVT {
	long midivec;		/* midi input */
	long vkbderr;		/* keyboard error */
	long vmiderr;		/* MIDI error */
	long statvec;		/* IKBD status */
	int (*mousevec)();	/* mouse vector */
	long clockvec;		/* clock vector */
	long joyvec;		/* joystict vector */
} *ktable;

int (*sysmint)();			/* system mouse interupt handler */

/* mouse parameter table */
struct Param {
	char topmode;
	char buttons;
	char xparam;
	char yparam;
	int xmax,ymax;
	int xinitial,yinitial;
} mparam;

int initrez;			/* initial screen resolution */
int currez;			/* current screen resolution */
char resname[][8] = {		/* screen resolution names */
	"LOW", "MEDIUM", "HIGH", "DENSE"
};
short spalette[16];		/* original color palette settings */
short palette[16];		/* current palette settings */

LINEA *aline;	/* Pointer to line a parameter block returned by init */

NLINEA *naline;	/* Pointer to line a parameters at negative offsets  */

FONT  **fonts;	/* Array of pointers to the three system font headers */
		/* returned by init (in register A1)                  */

WORD  (**foncs)();    /* Array of pointers to the 15 line a functions      */
                     /* returned by init (in register A2)                 */
                     /* only valid in ROM'ed TOS                          */

FONT *system_font;	/* pointer to default system font */
FONT *small_font;	/* pointer to small font */

extern  int     ttopen();               /* Forward references.          */
extern  int     ttgetc();
extern  int     ttputc();
extern  int     ttflush();
extern  int     ttclose();
extern  int     stmove();
extern  int     steeol();
extern  int     steeop();
extern  int     stbeep();
extern  int     stopen();
extern	int	stclose();
extern	int	stgetc();
extern	int	stputc();
extern	int	strev();
extern	int	strez();
extern	int	stkopen();
extern	int	stkclose();

#if	COLOR
extern	int	stfcol();
extern	int	stbcol();
#endif

/*
 * Dispatch table. All the
 * hard fields just point into the
 * terminal I/O code.
 */
TERM    term    = {
	NROW-1,
        NROW-1,
        NCOL,
        NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
        &stopen,
        &stclose,
	&stkopen,
	&stkclose,
        &stgetc,
	&stputc,
        &ttflush,
        &stmove,
        &steeol,
        &steeop,
        &stbeep,
        &strev,
	&strez
#if	COLOR
	, &stfcol,
	&stbcol
#endif
};

void init_aline()
{
       linea0();
       aline = (LINEA *)(la_init.li_a0);
       fonts = (FONT **)(la_init.li_a1);
       foncs = la_init.li_a2;
       naline = ((NLINEA *)aline) - 1;
}

init()
{
       init_aline();
       system_font = fonts[SCRFONT];        /* save it */
	small_font = fonts[1];
}


switch_font(fp)

FONT *fp;

{
       /* See aline.h for description of fields */
	/* these definitions are temporary...too many cooks!!! */
#undef	V_CEL_HT
#undef	V_CEL_WR
#undef	V_CEL_MY
#undef	V_CEL_MX
#undef	V_FNT_ST
#undef	V_FNT_ND
#undef	V_FNT_AD
#undef	V_FNT_WR
#undef	V_OFF_AD
#undef	VWRAP
#undef	V_Y_MAX
#undef	V_X_MAX

       naline->V_CEL_HT = fp->form_height;
       naline->V_CEL_WR = aline->VWRAP * fp->form_height;
       naline->V_CEL_MY = (naline->V_Y_MAX / fp->form_height) - 1;
       naline->V_CEL_MX = (naline->V_X_MAX / fp->max_cell_width) - 1;
       naline->V_FNT_WR = fp->form_width;
       naline->V_FNT_ST = fp->first_ade;
       naline->V_FNT_ND = fp->last_ade;
       naline->V_OFF_AD = fp->off_table;
       naline->V_FNT_AD =  fp->dat_table;
}

stmove(row, col)
{
        stputc(ESC);
        stputc('Y');
        stputc(row+BIAS);
        stputc(col+BIAS);
}

steeol()
{
        stputc(ESC);
        stputc('K');
}

steeop()
{
#if	COLOR
	stfcol(gfcolor);
	stbcol(gbcolor);
#endif
        stputc(ESC);
        stputc('J');
}

strev(status)	/* set the reverse video state */

int status;	/* TRUE = reverse video, FALSE = normal video */

{
	if (currez > 1) {
		stputc(ESC);
		stputc(status ? 'p' : 'q');
	}
}

#if	COLOR
mapcol(clr)	/* medium rez color translation */

int clr;	/* emacs color number to translate */

{
	static int mctable[] = {0, 1, 2, 3, 2, 1, 2, 3};

	if (currez != 1)
		return(clr);
	else
		return(mctable[clr]);
}

stfcol(color)	/* set the forground color */

int color;	/* color to set forground to */

{
	if (currez < 2) {
		stputc(ESC);
		stputc('b');
		stputc(mapcol(color));
	}
}

stbcol(color)	/* set the background color */

int color;	/* color to set background to */


{
	if (currez < 2) {
		stputc(ESC);
		stputc('c');
		stputc(mapcol(color));
	}
}
#endif

static char beep[] = {
	0x00, 0x00,
	0x01, 0x01,
	0x02, 0x01,
	0x03, 0x01,
	0x04, 0x02,
	0x05, 0x01,
	0x07, 0x38,
	0x08, 0x10,
	0x09, 0x10,
	0x0A, 0x10,
	0x0B, 0x00,
	0x0C, 0x30,
	0x0D, 0x03,
	0xFF, 0x00
};

stbeep()
{
	Dosound(beep);
}

domouse()	/* mouse interupt handler */

{
	return((*sysmint)());
}

stkopen()	/* open the keyboard (and mouse) */

{
	/* grab the keyboard vector table */
	ktable = (struct KVT *)xbios(KBDVBASE);
	sysmint = ktable->mousevec;	/* save mouse vector */

	/* initialize the mouse */
	mparam.topmode = 0;
	mparam.buttons = 4;
	mparam.xparam = 8;
	mparam.yparam = 10;
	mparam.xmax = 79;
	mparam.ymax = 23;
	mparam.xinitial = 0;
	mparam.yinitial = 0;
	xbios(INITMOUS, 4, &mparam, &domouse);
}

stopen()	/* open the screen */

{
	int i;

        ttopen();
	eolexist = TRUE;
	init();

	/* switch to a steady cursor */
	xbios(CURSCONF, 3);

	/* save the current color palette */
	for (i=0; i<16; i++)
		spalette[i] = xbios(SETCOLOR, i, -1);

	/* and find the current resolution */
	initrez = currez = xbios(GETREZ);
	strcpy(sres, resname[currez]);

	/* set up the screen size and palette */
	switch (currez) {
		case 0:	term.t_mrow = 25 - 1;
			term.t_nrow = 25 - 1;
			term.t_ncol = 40;
			strcpy(palstr, LOWPAL);
			break;

		case 1: term.t_mrow = 25 - 1;
			term.t_nrow = 25 - 1;
			strcpy(palstr, MEDPAL);
			break;

		case 2: term.t_mrow = DENSIZE - 1;
			term.t_nrow = 25 - 1;
			strcpy(palstr, HIGHPAL);
	}

	/* and set up the default palette */
	spal(palstr);

	stputc(ESC);	/* automatic overflow off */
	stputc('w');
	stputc(ESC);	/* turn cursor on */
	stputc('e');
}

stkclose()	/* close the keyboard (and mouse) */

{
	static char resetcmd[] = {0x80, 0x01};	/* keyboard reset command */

	/* restore the mouse interupt routines */
	xbios(INITMOUS, 2, &mparam, (long)sysmint);

	/* and reset the keyboard controller */
	xbios(IKBDWS, 1, &resetcmd[0]);
}

stclose()

{
	stputc(ESC);	/* auto overflow on */
	stputc('v');

	/* switch to a flashing cursor */
	xbios(CURSCONF, 2);

	/* restore the original screen resolution */
	if (currez == 3)
		switch_font(system_font);
	strez(resname[initrez]);

	/* restore the original palette settings */
	xbios(SETPALETTE, spalette);

	ttclose();
}

/* 	spal(pstr):	reset the current palette according to a
			"palette string" of the form

	000111222333444555666777

	which contains the octal values for the palette registers
*/

spal(pstr)

char *pstr;	/* palette string */

{
	int pal;	/* current palette position */
	int clr;	/* current color value */
	int i;

	for (pal = 0; pal < 16; pal++) {
		if (*pstr== 0)
			break;

		/* parse off a color */
		clr = 0;
		for (i = 0; i < 3; i++)
			if (*pstr)
				clr = clr * 16 + (*pstr++ - '0');
		palette[pal] = clr;
	};

	/* and now set it */
	xbios(SETPALETTE, palette);
}

stgetc()	/* get a char from the keyboard */

{
	register long rval;	/* return value from BIOS call */
	static int funkey = 0;	/* held fuction key scan code */
	static long sh;		/* shift/alt key on held function? */
	long bios();

	/* if there is a pending function key, return it */
	if (funkey) {
		if (sh) {	/* alt or cntrl */
			if (funkey >= 0x3B && funkey <= 0x44) {
				rval = funkey + '^' - ';';
				if (sh & 0x08)	/* alt */
					rval += 10;
				funkey = 0;
				return(rval & 255);
			}
		}
		rval = funkey;
		funkey = 0;
	} else {
		/* waiting... flash the cursor */
		xbios(CURSCONF, 2);

		/* get the character */
		rval = bios(CONIN, CON);
		sh = Getshift(-1) & 0x0cL; /* see if alt or cntrl depressed */
		if ((rval & 255L) == 0L) {
			funkey = (rval >> 16L) & 255;
			rval = 0;
		}

	}

	/* and switch to a steady cursor */
	xbios(CURSCONF, 3);

	return(rval & 255);
}

stputc(c)	/* output char c to the screen */

char c;		/* character to print out */

{
	bios(BCONOUT, CON, c);
}

strez(newrez)	/* change screen resolution */

char *newrez;	/* requested resolution */

{
	int nrez;	/* requested new resolution */

	/* first, decode the resolution name */
	for (nrez = 0; nrez < 4; nrez++)
		if (strcmp(newrez, resname[nrez]) == 0)
			break;
	if (nrez == 4) {
		mlwrite("%%No such resolution");
		return(FALSE);
	}

	/* next, make sure this resolution is legal for this monitor */
	if ((currez < 2 && nrez > 1) || (currez > 1 && nrez < 2)) {
		mlwrite("%%Resolution illegal for this monitor");
		return(FALSE);
	}

	/* eliminate non-changes */
	if (currez == nrez)
		return(TRUE);

	/* finally, make the change */
	switch (nrez) {
		case 0:	/* low resolution - 16 colors */
			newwidth(TRUE, 40);
			strcpy(palstr, LOWPAL);
			xbios(SETSCREEN, -1L, -1L, 0);
			break;

		case 1:	/* medium resolution - 4 colors */
			newwidth(TRUE, 80);
			strcpy(palstr, MEDPAL);
			xbios(SETSCREEN, -1L, -1L, 1);
			break;

		case 2:	/* High resolution - 2 colors - 25 lines */
			newsize(TRUE, 25);
			strcpy(palstr, HIGHPAL);
			switch_font(system_font);
			break;

		case 3:	/* Dense resolution - 2 colors - 40 lines */
			newsize(TRUE, DENSIZE);
			strcpy(palstr, HIGHPAL);
			switch_font(small_font);
			break;
	}

	/* and set up the default palette */
	spal(palstr);
	currez = nrez;
	strcpy(sres, resname[currez]);

	stputc(ESC);	/* automatic overflow off */
	stputc('w');
	stputc(ESC);	/* turn cursor on */
	stputc('e');

	return(TRUE);
}

#if	LATTICE
system(cmd)	/* call the system to execute a new program */

char *cmd;	/* command to execute */

{
	char *pptr;			/* pointer into program name */
	char pname[NSTRING];		/* name of program to execute */
	char tail[NSTRING];		/* command tail */

	/* scan off program name.... */
	pptr = pname;
	while (*cmd && (*cmd != ' ' && *cmd != '\t'))
		*pptr++ = *cmd++;
	*pptr = 0;

	/* create program name length/string */
	tail[0] = strlen(cmd);
	strcpy(&tail[1], cmd);

	/* go do it! */
	return(gemdos(		(int)EXEC,
				(int)0,
				(char *)pname,
				(char *)tail,
				(char *)NULL));
}
#endif

#if	TYPEAH
typahead()

{
	int rval;	/* return value from BIOS call */

	/* get the status of the console */
	rval = bios(BCONSTAT, CON);

	/* end return the results */
	if (rval == 0)
		return(FALSE);
	else
		return(TRUE);
}
#endif

#if	FLABEL
fnclabel(f, n)		/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif
#else
sthello()
{
}
#endif

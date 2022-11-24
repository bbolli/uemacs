/*
 * The routines in this file move the cursor around on the screen. They
 * compute a new value for the cursor, then adjust ".". The display code
 * always updates the cursor location, so only moves between lines, or
 * functions that adjust the top line in the window and invalidate the
 * framing, are hard.
 */
#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

#define OVERLAP 2	/* overlap at page up/down (like ITS EMACS) */

/*
 * Move the cursor to the beginning of the current line. Trivial.
 */
gotobol(f, n)
{
	curwp->w_doto  = 0;
	return (TRUE);
}

/*
 * Move the cursor backwards by "n" characters. If "n" is less than zero call
 * "forwchar" to actually do the move. Otherwise compute the new cursor
 * location. Error if you try and move out of the buffer. Set the flag if the
 * line pointer for dot changes.
 */
backchar(f, n)
register int	n;
{
	register LINE	*lp;

	if (n < 0)
		return (forwchar(f, -n));
	while (n--) {
		if (curwp->w_doto == 0) {
			if ((lp=lback(curwp->w_dotp)) == curbp->b_linep)
				return (FALSE);
			curwp->w_dotp  = lp;
			curwp->w_doto  = llength(lp);
			curwp->w_flag |= WFMOVE;
		} else
			curwp->w_doto--;
	}
	return (TRUE);
}

/*
 * Move the cursor to the end of the current line. Trivial. No errors.
 */
gotoeol(f, n)
{
	curwp->w_doto  = llength(curwp->w_dotp);
	return (TRUE);
}

/*
 * Move the cursor forwards by "n" characters. If "n" is less than zero call
 * "backchar" to actually do the move. Otherwise compute the new cursor
 * location, and move ".". Error if you try and move off the end of the
 * buffer. Set the flag if the line pointer for dot changes.
 */
forwchar(f, n)
register int	n;
{
	if (n < 0)
		return (backchar(f, -n));
	while (n--) {
		if (curwp->w_doto == llength(curwp->w_dotp)) {
			if (curwp->w_dotp == curbp->b_linep)
				return (FALSE);
			curwp->w_dotp  = lforw(curwp->w_dotp);
			curwp->w_doto  = 0;
			curwp->w_flag |= WFMOVE;
		} else
			curwp->w_doto++;
	}
	return (TRUE);
}

gotoline(f, n)		/* move to a particular line.
			   argument (n) must be a positive integer for
			   this to actually do anything 	*/

{
	register int status;	/* status return */
	char arg[NSTRING];	/* buffer to hold argument */

	/* get an argument if one doesnt exist */
	if (f == FALSE) {
		if ((status = mlreply("Line to GOTO: ", arg, NSTRING)) != TRUE) {
			mlwrite("[Aborted]");
			return(status);
		}
		n = atoi(arg);
	}

	if (n < 1)		/* if a bogus argument...then leave */
		return(FALSE);

	/* first, we go to the start of the buffer */
	curwp->w_dotp  = lforw(curbp->b_linep);
	curwp->w_doto  = 0;
	return(forwline(f, n-1));
}

/*
 * Goto the beginning of the buffer. Massive adjustment of dot. This is
 * considered to be hard motion; it really isn't if the original value of dot
 * is the same as the new value of dot. Normally bound to "M-<".
 */
gotobob(f, n)
{
	curwp->w_dotp  = lforw(curbp->b_linep);
	curwp->w_doto  = 0;
	curwp->w_flag |= WFHARD;
	return (TRUE);
}

/*
 * Move to the end of the buffer. Dot is always put at the end of the file
 * (ZJ). The standard screen code does most of the hard parts of update.
 * Bound to "M->".
 */
gotoeob(f, n)
{
	curwp->w_dotp  = curbp->b_linep;
	curwp->w_doto  = 0;
	curwp->w_flag |= WFHARD;
	return (TRUE);
}

/*
 * Move forward by full lines. If the number of lines to move is less than
 * zero, call the backward line function to actually do it. The last command
 * controls how the goal column is set. Bound to "C-N". No errors are
 * possible.
 */
forwline(f, n)
{
	register LINE	*dlp;

	if (n < 0)
		return (backline(f, -n));

	/* if we are on the last line as we start....fail the command */
	if (curwp->w_dotp == curbp->b_linep)
		return(FALSE);

	/* if the last command was not note a line move,
	   reset the goal column */
	if ((lastflag&CFCPCN) == 0)
		curgoal = getccol(FALSE);

	/* flag this command as a line move */
	thisflag |= CFCPCN;

	/* and move the point down */
	dlp = curwp->w_dotp;
	while (n-- && dlp!=curbp->b_linep)
		dlp = lforw(dlp);

	/* reseting the current position */
	curwp->w_dotp  = dlp;
	curwp->w_doto  = getgoal(dlp);
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}

/*
 * This function is like "forwline", but goes backwards. The scheme is exactly
 * the same. Check for arguments that are less than zero and call your
 * alternate. Figure out the new line and call "movedot" to perform the
 * motion. No errors are possible. Bound to "C-P".
 */
backline(f, n)
{
	register LINE	*dlp;

	if (n < 0)
		return (forwline(f, -n));


	/* if we are on the first line as we start....fail the command */
	if (lback(curwp->w_dotp) == curbp->b_linep)
		return(FALSE);

	/* if the last command was not note a line move,
	   reset the goal column */
	if ((lastflag&CFCPCN) == 0)
		curgoal = getccol(FALSE);

	/* flag this command as a line move */
	thisflag |= CFCPCN;

	/* and move the point up */
	dlp = curwp->w_dotp;
	while (n-- && lback(dlp)!=curbp->b_linep)
		dlp = lback(dlp);

	/* reset the current position */
	curwp->w_dotp  = dlp;
	curwp->w_doto  = getgoal(dlp);
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}

#if	WORDPRO
gotobop(f, n)	/* go back to the beginning of the current paragraph
		   here we look for a <NL><NL> or <NL><TAB> or <NL><SPACE>
		   combination to delimit the beginning of a paragraph	*/

int f, n;	/* default Flag & Numeric argument */

{
	register int suc;	/* success of last backchar */

	if (n < 0)	/* the other way...*/
		return(gotoeop(f, -n));

	while (n-- > 0) {	/* for each one asked for */

		/* first scan back until we are in a word */
		suc = backchar(FALSE, 1);
		while (!inword(-1) && suc)
			suc = backchar(FALSE, 1);
		curwp->w_doto = 0;	/* and go to the B-O-Line */

		/* and scan back until we hit a <NL><NL> or <NL><TAB>
		   or a <NL><SPACE>					*/
		while (lback(curwp->w_dotp) != curbp->b_linep)
			if (llength(curwp->w_dotp) != 0 &&
			    lgetc(curwp->w_dotp, curwp->w_doto) != TAB &&
			    lgetc(curwp->w_dotp, curwp->w_doto) != ' ')
				curwp->w_dotp = lback(curwp->w_dotp);
			else
				break;

		/* and then forward until we are in a word */
		suc = forwchar(FALSE, 1);
		while (suc && !inword(-1))
			suc = forwchar(FALSE, 1);
	}
	curwp->w_flag |= WFMOVE;	/* force screen update */
	return(TRUE);
}

gotoeop(f, n)	/* go forword to the end of the current paragraph
		   here we look for a <NL><NL> or <NL><TAB> or <NL><SPACE>
		   combination to delimit the beginning of a paragraph	*/

int f, n;	/* default Flag & Numeric argument */

{
	register int suc;	/* success of last backchar */

	if (n < 0)	/* the other way...*/
		return(gotobop(f, -n));

	while (n-- > 0) {	/* for each one asked for */

		/* first scan forward until we are in a word */
		suc = forwchar(FALSE, 1);
		while (!inword(-1) && suc)
			suc = forwchar(FALSE, 1);
		curwp->w_doto = 0;	/* and go to the B-O-Line */
		if (suc)	/* of next line if not at EOF */
			curwp->w_dotp = lforw(curwp->w_dotp);

		/* and scan forword until we hit a <NL><NL> or <NL><TAB>
		   or a <NL><SPACE>					*/
		while (curwp->w_dotp != curbp->b_linep) {
			if (llength(curwp->w_dotp) != 0 &&
			    lgetc(curwp->w_dotp, curwp->w_doto) != TAB &&
			    lgetc(curwp->w_dotp, curwp->w_doto) != ' ')
				curwp->w_dotp = lforw(curwp->w_dotp);
			else
				break;
		}

		/* and then backward until we are in a word */
		suc = backchar(FALSE, 1);
		while (suc && !inword(-1)) {
			suc = backchar(FALSE, 1);
		}
		curwp->w_doto = llength(curwp->w_dotp); /* and to the EOL */
	}
	curwp->w_flag |= WFMOVE;	/* force screen update */
	return(TRUE);
}
#endif

/*
 * This routine, given a pointer to a LINE, and the current cursor goal
 * column, return the best choice for the offset. The offset is returned.
 * Used by forw/backline and forw/backpage.
 */
getgoal(dlp)
register LINE	*dlp;
{
	register int	c;
	register int	col;
	register int	newcol;
	register int	dbo;

	col = 0;
	dbo = 0;
	while (dbo != llength(dlp)) {
		c = lgetc(dlp, dbo);
		newcol = col;
		if (c == '\t')
			newcol |= 0x07;
		else if (c<0x20 || c==0x7F)
			++newcol;
		++newcol;
		if (newcol > curgoal)
			break;
		col = newcol;
		++dbo;
	}
	return (dbo);
}

/*
 * Scroll forward by a specified number of lines, or by a full page if no
 * argument. Bound to "C-V".
 * This was changed to work like most editors do, i.e. the top window line
 * moves as much as the "." line, to the effect that "." stays at about the
 * same screen position. The offset logic is from forwline.
 */
forwpage(f, n)
register int	n;
{
	register LINE *lp, *lt;

	if (f == FALSE) {
		n = curwp->w_ntrows - OVERLAP;	/* Default scroll.	*/
		if (n <= 0)			/* Forget the overlap	*/
			n = 1;			/* if tiny window.	*/
	} else if (n < 0)
		return (backpage(f, -n));
#if	CVMVAS
	else					/* Convert from pages	*/
		n *= curwp->w_ntrows;		/* to lines.		*/
#endif
	/* if we are on the last line as we start....fail the command */
	if (curwp->w_dotp == curbp->b_linep)
		return FALSE;

	/* if the last command was not a line move, reset the goal column */
	if ((lastflag & CFCPCN) == 0)
		curgoal = getccol(FALSE);

	/* flag this command as a line move */
	thisflag |= CFCPCN;

	/* and move the point and the top window line down */
	lp = curwp->w_dotp;
	lt = curwp->w_linep;
	while (n-- && lp!=curbp->b_linep)
		lp = lforw(lp),  lt = lforw(lt);

	/* reset the current position */
	curwp->w_linep = lt;
	curwp->w_dotp  = lp;
	curwp->w_doto  = getgoal(lp);
	curwp->w_flag |= WFHARD;
	return TRUE;
}

/*
 * This command is like "forwpage", but it goes backwards. Bound to "M-V".
 */
backpage(f, n)
register int	n;
{
	register LINE *lp, *lt;

	if (f == FALSE) {
		n = curwp->w_ntrows - OVERLAP;	/* Default scroll.	*/
		if (n <= 0)			/* Don't blow up if the */
			n = 1;			/* window is tiny.	*/
	} else if (n < 0)
		return (forwpage(f, -n));
#if	CVMVAS
	else					/* Convert from pages	*/
		n *= curwp->w_ntrows;		/* to lines.		*/
#endif

	/* if we are on the first line as we start, try to move "." to BOL,
	 * else fail the command
	 */
	if (lback(curwp->w_dotp) == curbp->b_linep)
		if ( curwp->w_doto != 0 ) {
			curwp->w_doto = 0;
			curwp->w_flag |= WFMOVE;
			return TRUE;
		} else
			return FALSE;

	/* if the last command was not a line move, reset the goal column */
	if ((lastflag & CFCPCN) == 0)
		curgoal = getccol(FALSE);

	/* flag this command as a line move */
	thisflag |= CFCPCN;

	/* and move the point up */
	lp = curwp->w_dotp;
	lt = curwp->w_linep;
	while ( n-- && lback( lp ) != curbp->b_linep ) {
		lp = lback( lp );
		if ( lback( lt ) != curbp->b_linep )
			lt = lback( lt );
	}

	/* reset the current position */
	curwp->w_linep = lt;
	curwp->w_dotp  = lp;
	curwp->w_doto  = getgoal(lp);
	curwp->w_flag |= WFHARD;
	return TRUE;
}

/*
 * Set the mark in the current window to the value of "." in the window. No
 * errors are possible. Bound to "M-.".
 */
setmark(f, n)
{
	curwp->w_markp = curwp->w_dotp;
	curwp->w_marko = curwp->w_doto;
	mlwrite("[Mark set]");
	return (TRUE);
}

/*
 * clear the mark in the current window. Trivial.
 */
resetmark(f, n)
{
	curwp->w_markp = NULL;
	curwp->w_marko = 0;
	mlwrite("[Mark cleared]");
	return TRUE;
}

/*
 * Swap the values of "." and "mark" in the current window. This is pretty
 * easy, bacause all of the hard work gets done by the standard routine
 * that moves the mark about. The only possible error is "no mark". Bound to
 * "C-X C-X".
 */
swapmark(f, n)
{
	register LINE	*odotp;
	register int	odoto;

	if (curwp->w_markp == NULL) {
		mlwrite("No mark in this window");
		return (FALSE);
	}
	odotp = curwp->w_dotp;
	odoto = curwp->w_doto;
	curwp->w_dotp  = curwp->w_markp;
	curwp->w_doto  = curwp->w_marko;
	curwp->w_markp = odotp;
	curwp->w_marko = odoto;
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}

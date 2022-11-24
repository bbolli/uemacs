/*	Spawn:	various DOS access commands
		for MicroEMACS
*/

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

#if	AMIGA
#define  NEW   1006L
#endif

#if	VMS
#define EFN	0				/* Event flag.		*/

#include	<ssdef.h>			/* Random headers.	*/
#include	<stsdef.h>
#include	<descrip.h>
#include	<iodef.h>

extern	int	oldmode[3];			/* In "termio.c"	*/
extern	int	newmode[3];			/* In "termio.c"	*/
extern	short	iochan;				/* In "termio.c"	*/
#endif

#if	V7 | USG | BSD
#include	<signal.h>
extern int vttidy();
#endif

#if	MSDOS & (MSC | TURBO)
#include	<process.h>
#endif

/*
 * Create a subjob with a copy of the command interpreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C". The message at the start in VMS puts out a newline.
 * Under some (unknown) condition, you don't get one free when DCL starts up.
 */
spawncli(f, n)
{
#if	AMIGA
	long newcli;

#endif

#if	V7 | USG | BSD
	register char *cp;
	char	*getenv();
#endif

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

#if	AMIGA
	mlwrite("[Starting new CLI]");
	sgarbf = TRUE;
	Execute("NEWCLI \"CON:0/0/640/200/MicroEMACS Subprocess\"", 0L, 0L);
	return(TRUE);
#endif

#if	VMS
	movecursor(term.t_nrow, 0);		/* In last line.	*/
	mlputs("[Starting DCL]\r\n");
	TTflush(); 				 /* Ignore "ttcol".	 */
	sgarbf = TRUE;
	return (sys(NULL));			/* NULL => DCL.	*/
#endif
#if	CPM
	mlwrite("Not in CP/M-86");
#endif
#if	MSDOS & (AZTEC | MSC | TURBO)
	movecursor(term.t_nrow, 0);		/* Seek to last line.	*/
	TTflush();
	TTkclose();
	shellprog("");
	TTkopen();
	sgarbf = TRUE;
	return(TRUE);
#endif
#if	ST520 & MWC
	mlerase();	/* clear the message line */
	TTflush();
	TTkclose();
	system("msh.prg");
	TTkopen();
	sgarbf = TRUE;
	return(TRUE);
#endif
#if	MSDOS & LATTICE
	movecursor(term.t_nrow, 0);		/* Seek to last line.	*/
	TTflush();
	TTkclose();
	sys("\\command.com", "");		/* Run CLI.		*/
	TTkopen();
	sgarbf = TRUE;
	return(TRUE);
#endif
#if	V7 | USG | BSD
	movecursor(term.t_nrow, 0);		/* Seek to last line.	*/
	TTflush();
	TTclose();				/* stty to old settings */
	if ((cp = getenv("SHELL")) != NULL && *cp != '\0')
		system(cp);
	else
#if	BSD
		system("exec /bin/csh");
#else
		system("exec /bin/sh");
#endif
	sgarbf = TRUE;
	sleep(2);
	TTopen();
	return(TRUE);
#endif
}

#if	BSD

bktoshell()		/* suspend MicroEMACS and wait to wake up */
{
	int pid;

	vttidy();
	pid = getpid();
	kill(pid,SIGTSTP);
}

rtfrmshell()
{
	TTopen();
	curwp->w_flag = WFHARD;
	sgarbf = TRUE;
}
#endif

/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "^X!".
 */
spawn(f, n)
{
	register int	s;
	char		line[NLINE];

#if	AMIGA
	long newcli;
#endif

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

#if	AMIGA
	if ((s=mlreply("! ", line, NLINE)) != TRUE)
		return (s);
	newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
	Execute(line, 0L, newcli);
	Close(newcli);
	tgetc();     /* Pause.		     */
	sgarbf = TRUE;
	return(TRUE);
#endif

#if	VMS
	if ((s=mlreply("! ", line, NLINE)) != TRUE)
		return (s);
	TTputc('\n');		     /* Already have '\r'    */
	TTflush();
	s = sys(line);				/* Run the command.	*/
	ttputs("\r\n[End]");			/* Pause.		*/
	TTflush();
	tgetc();
	sgarbf = TRUE;
	return (s);
#endif
#if	CPM
	mlwrite("Not in CP/M-86");
	return (FALSE);
#endif
#if	MSDOS
	if ((s=mlreply("! ", line, NLINE)) != TRUE)
		return(s);
	movecursor(term.t_nrow, 0);
	TTkclose();
	s = shellprog(line);
	TTkopen();
	/* if we are interactive, pause here */
	if (s == TRUE) {
		if (clexec == FALSE) {
			ttputs("\r\n[End]");
			tgetc();
		}
		sgarbf = TRUE;
	}
	return s;
#endif
#if	ST520 & MWC
	if ((s=mlreply("! ", line, NLINE)) != TRUE)
		return(s);
	mlerase();
	TTkclose();
	system(line);
	TTkopen();
	/* if we are interactive, pause here */
	if (clexec == FALSE) {
		ttputs("\r\n[End]");
		tgetc();
	}
	sgarbf = TRUE;
	return (TRUE);
#endif
#if	V7 | USG | BSD
	if ((s=mlreply("! ", line, NLINE)) != TRUE)
		return (s);
	TTputc('\n');		     /* Already have '\r'    */
	TTflush();
	TTclose();				/* stty to old modes	*/
	system(line);
	TTopen();
	ttputs("[End]");			/* Pause.		*/
	TTflush();
	while ((s = tgetc()) != '\r' && s != ' ')
		;
	sgarbf = TRUE;
	return (TRUE);
#endif
}

/*
 * Run an external program with arguments. When it returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X $".
 */

execprg(f, n)

{
	register int	s;
	char		line[NLINE];

#if	AMIGA
	long newcli;
#endif

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

#if	AMIGA
	if ((s=mlreply("$ ", line, NLINE)) != TRUE)
		return (s);
	newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
	Execute(line, 0L, newcli);
	Close(newcli);
	tgetc();     /* Pause.		     */
	sgarbf = TRUE;
	return(TRUE);
#endif

#if	VMS
	if ((s=mlreply("$ ", line, NLINE)) != TRUE)
		return (s);
	TTputc('\n');		     /* Already have '\r'    */
	TTflush();
	s = sys(line);				/* Run the command.	*/
	ttputs("\r\n[End]");			/* Pause.		*/
	TTflush();
	tgetc();
	sgarbf = TRUE;
	return (s);
#endif
#if	CPM
	mlwrite("Not in CP/M-86");
	return (FALSE);
#endif
#if	MSDOS
	if ((s=mlreply("$ ", line, NLINE)) != TRUE)
		return(s);
	movecursor(term.t_nrow, 0);
	TTkclose();
	s = execprog(line);
	TTkopen();
	/* if we are interactive, pause here */
	if (s == TRUE) {
		if (clexec == FALSE) {
			ttputs("\r\n[End]");
			tgetc();
		}
		sgarbf = TRUE;
	}
	return (s);
#endif
#if	ST520 & MWC
	if ((s=mlreply("$ ", line, NLINE)) != TRUE)
		return(s);
	mlerase();
	TTkclose();
	system(line);
	TTkopen();
	/* if we are interactive, pause here */
	if (clexec == FALSE) {
		ttputs("\r\n[End]");
		tgetc();
	}
	sgarbf = TRUE;
	return (TRUE);
#endif
#if	V7 | USG | BSD
	if ((s=mlreply("$ ", line, NLINE)) != TRUE)
		return (s);
	TTputc('\n');		     /* Already have '\r'    */
	TTflush();
	TTclose();				/* stty to old modes	*/
	system(line);
	TTopen();
	mlputs("[End]");			/* Pause.		*/
	TTflush();
	while ((s = tgetc()) != '\r' && s != ' ')
		;
	sgarbf = TRUE;
	return (TRUE);
#endif
}

/*
 * Pipe a one line command into a window
 * Bound to ^X @
 */
pipecmd(f, n)
{
	register int	s;	/* return status from CLI */
	register WINDOW *wp;	/* pointer to new window */
	register BUFFER *bp;	/* pointer to buffer to zot */
	char	line[NLINE];	/* command line send to shell */
	static char bname[] = "command";

#if	AMIGA
	static char filnam[] = "ram:command";
	long newcli;
#else
	static char filnam[NSTRING] = "command";
#endif

#if	MSDOS | (ST520 & MWC)
	char *tmp;
	char *getenv();
#endif

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

#if	MSDOS
	if ((tmp = getenv("TMP")) == NULL)
		strcpy(filnam, "command");
	else {
		strcpy(filnam, tmp);
		if (filnam[strlen(filnam) - 1] != '\\')
		    strcat(filnam,"\\");
		strcat(filnam,"command");
	}
#endif

#if	VMS
	mlwrite("Not available under VMS");
	return(FALSE);
#endif
#if	CPM
	mlwrite("Not available under CP/M-86");
	return(FALSE);
#endif

	/* get the command to pipe in */
	if ((s=mlreply("@ ", line, NLINE)) != TRUE)
		return(s);

	/* get rid of the command output buffer if it exists */
	if ((bp=bfind(bname, FALSE, 0)) != FALSE) {
		/* try to make sure we are off screen */
		wp = wheadp;
		while (wp != NULL) {
			if (wp->w_bufp == bp) {
				onlywind(FALSE, 1);
				break;
			}
			wp = wp->w_wndp;
		}
		if (zotbuf(bp) != TRUE)

			return(FALSE);
	}

#if	AMIGA
	newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
	strcat(line, " >");
	strcat(line, filnam);
	Execute(line, 0L, newcli);
	s = TRUE;
	Close(newcli);
	sgarbf = TRUE;
#endif
#if	MSDOS | (ST520 & MWC)
	strcat(line," >>");
	strcat(line,filnam);
	movecursor(term.t_nrow, 0);
	TTkclose();
#if	MSDOS
	shellprog(line);
#else
	system(line);
#endif
	TTkopen();
	sgarbf = TRUE;
	s = fexist(filnam);
#endif
#if	V7 | USG | BSD
	TTputc('\n');		     /* Already have '\r'    */
	TTflush();
	TTclose();				/* stty to old modes	*/
	strcat(line,">");
	strcat(line,filnam);
	system(line);
	TTopen();
	TTflush();
	sgarbf = TRUE;
	s = TRUE;
#endif

	if (s != TRUE)
		return(s);

	/* split the current window to make room for the command output */
	if (splitwind(FALSE, 1) == FALSE)
		return(FALSE);

	/* and read the stuff in */
	if (getfile(filnam, FALSE) == FALSE)
		return(FALSE);

	/* make this window in VIEW mode, update all mode lines */
	curwp->w_bufp->b_mode |= MDVIEW;
	wp = wheadp;
	while (wp != NULL) {
		wp->w_flag |= WFMODE;
		wp = wp->w_wndp;
	}

	/* and get rid of the temporary file */
	unlink(filnam);
	return(TRUE);
}

/*
 * filter a buffer through an external DOS program
 * Bound to ^X #
 */
filter(f, n)

{
	register int	s;	/* return status from CLI */
	register BUFFER *bp;	/* pointer to buffer to zot */
	char line[NLINE];	/* command line send to shell */
	char tmpnam[NFILEN];	/* place to store real file name */
	static char bname1[] = "fltinp";

#if	AMIGA
	static char filnam1[] = "ram:fltinp";
	static char filnam2[] = "ram:fltout";
	long newcli;
#else
	static char filnam1[] = "fltinp";
	static char filnam2[] = "fltout";
#endif

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

#if	VMS
	mlwrite("Not available under VMS");
	return(FALSE);
#endif
#if	CPM
	mlwrite("Not available under CP/M-86");
	return(FALSE);
#endif

	/* get the filter name and its args */
	if ((s=mlreply("# ", line, NLINE)) != TRUE)
		return(s);

	/* setup the proper file names */
	bp = curbp;
	strcpy(tmpnam, bp->b_fname);	/* save the original name */
	strcpy(bp->b_fname, bname1);	/* set it to our new one */

	/* write it out, checking for errors */
	if (writeout(filnam1) != TRUE) {
		mlwrite("[Cannot write filter file]");
		strcpy(bp->b_fname, tmpnam);
		return(FALSE);
	}

#if	AMIGA
	newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
	strcat(line, " <ram:fltinp >ram:fltout");
	Execute(line,0L,newcli);
	s = TRUE;
	Close(newcli);
	sgarbf = TRUE;
#endif
#if	MSDOS | (ST520 & MWC)
	strcat(line," <fltinp >fltout");
	movecursor(term.t_nrow, 0);
	TTkclose();
#if	MSDOS
	shellprog(line);
#else
	system(line);
#endif
	TTkopen();
	s = TRUE;
	sgarbf = TRUE;
#endif
#if	V7 | USG | BSD
	TTputc('\n');		     /* Already have '\r'    */
	TTflush();
	TTclose();				/* stty to old modes	*/
	strcat(line," <fltinp >fltout");
	system(line);
	TTopen();
	TTflush();
	sgarbf = TRUE;
	s = TRUE;
#endif

	/* on failure, escape gracefully */
	if (s != TRUE || (readin(filnam2,FALSE) == FALSE)) {
		mlwrite("[Execution failed]");
		strcpy(bp->b_fname, tmpnam);
		unlink(filnam1);
		unlink(filnam2);
		return(s);
	}

	/* reset file name */
	strcpy(bp->b_fname, tmpnam);	/* restore name */
	bp->b_flag |= BFCHG;		/* flag it as changed */

	/* and get rid of the temporary file */
	unlink(filnam1);
	unlink(filnam2);
	return(TRUE);
}

#if	VMS
/*
 * Run a command. The "cmd" is a pointer to a command string, or NULL if you
 * want to run a copy of DCL in the subjob (this is how the standard routine
 * LIB$SPAWN works). You have to do weird stuff with the terminal on the way in
 * and the way out, because DCL does not want the channel to be in raw mode.
 */
sys(cmd)
register char	*cmd;
{
	struct	dsc$descriptor	cdsc;
	struct	dsc$descriptor	*cdscp;
	long	status;
	long	substatus;
	long	iosb[2];

	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  oldmode, sizeof(oldmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		return (FALSE);
	cdscp = NULL;				/* Assume DCL.		*/
	if (cmd != NULL) {			/* Build descriptor.	*/
		cdsc.dsc$a_pointer = cmd;
		cdsc.dsc$w_length  = strlen(cmd);
		cdsc.dsc$b_dtype   = DSC$K_DTYPE_T;
		cdsc.dsc$b_class   = DSC$K_CLASS_S;
		cdscp = &cdsc;
	}
	status = LIB$SPAWN(cdscp, 0, 0, 0, 0, 0, &substatus, 0, 0, 0);
	if (status != SS$_NORMAL)
		substatus = status;
	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  newmode, sizeof(newmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		return (FALSE);
	if ((substatus&STS$M_SUCCESS) == 0)	/* Command failed.	*/
		return (FALSE);
	return (TRUE);
}
#endif

#if	~AZTEC & ~MSC & ~TURBO & MSDOS

/*
 * This routine, once again by Bob McNamara, is a C translation of the "system"
 * routine in the MWC-86 run time library. It differs from the "system" routine
 * in that it does not unconditionally append the string ".exe" to the end of
 * the command name. We needed to do this because we want to be able to spawn
 * off "command.com". We really do not understand what it does, but if you don't
 * do it exactly "malloc" starts doing very very strange things.
 */
sys(cmd, tail)
char	*cmd;
char	*tail;
{
#if MWC
	register unsigned n;
	extern char *__end;

	n = __end + 15;
	n >>= 4;
	n = ((n + dsreg() + 16) & 0xFFF0) + 16;
	return(execall(cmd, tail, n));
#endif

#if LATTICE
	return(forklp(cmd, tail, (char *)NULL));
#endif
}
#endif

#if	MSDOS & LATTICE
/*	System: a modified version of lattice's system() function
		that detects the proper switchar and uses it
		written by Dana Hogget				*/

system(cmd)

char *cmd;	/*  Incoming command line to execute  */

{
	char *getenv();
	static char *swchar = "/C";	/*  Execution switch  */
	union REGS inregs;	/*	parameters for dos call  */
	union REGS outregs;	/*  Return results from dos call  */
	char *shell;		/*  Name of system command processor  */
	char *p;		/*  Temporary pointer  */
	int ferr;		/*	Error condition if any	*/

	/*  get name of system shell  */
	if ((shell = getenv("COMSPEC")) == NULL)
		return (-1);		/*  No shell located  */

	p = cmd;
	while (isspace(*p))		/*  find out if null command */
		p++;

	/**  If the command line is not empty, bring up the shell  **/
	/**  and execute the command.  Otherwise, bring up the     **/
	/**  shell in interactive mode.   **/

	if (p && *p) {
		/**  detect current switch character and us it  **/
		inregs.h.ah = 0x37;	/*  get setting data  */
		inregs.h.al = 0x00;	/*  get switch character	*/
		intdos(&inregs, &outregs);
		*swchar = outregs.h.dl;
		ferr = forkl(shell, "command", swchar, cmd, (char *)NULL);
	} else {
		ferr = forkl(shell, "command", (char *)NULL);
	}

	return (ferr ? ferr : wait());
}
#endif

#if	MSDOS & LATTICE
extern int _oserr;
#endif

#if	MSDOS & AZTEC
extern int errno;
#endif

#if	MSDOS & (TURBO | LATTICE | AZTEC)
/*	SHELLPROG: Execute a command in a subshell		*/

shellprog(cmd)

char *cmd;	/*  Incoming command line to execute  */

{
	char *shell;		/* Name of system command processor */
	char swchar;		/* switch character to use */
#if ~TURBO
	union REGS rg;	/* parameters for dos call */
#endif
	char comline[NSTRING];	/* constructed command line */
	char *getenv();

	/*  detect current switch character and set us up to use it */
	rg.h.ah = 0x37;	/*  get setting data  */
	rg.h.al = 0x00;	/*  get switch character  */
#if TURBO
	intd();
#else
	intdos(&rg, &rg);
#endif
	swchar = (char)rg.h.dl;

	/*  get name of system shell  */
	if ((shell = getenv("COMSPEC")) == NULL)  {
		mlwrite("[Shell not found]");
		return(FALSE);		/*  No shell located	*/
	}

	/*  trim leading whitespace off the command  */
	while (*cmd == ' ' || *cmd == '\t')	/*  find out if null command */
		cmd++;

	/**  If the command line is not empty, bring up the shell  **/
	/**  and execute the command.  Otherwise, bring up the     **/
	/**  shell in interactive mode.				   **/

	if (*cmd) {
		strcpy(comline, shell);
		strcat(comline, " ");
		comline[strlen(comline) + 1] = 0;
		comline[strlen(comline)] = swchar;
		strcat(comline, "c ");
		strcat(comline, cmd);
		return(execprog(comline));
	} else
		return(execprog(shell));
}

/*	EXECPROG:	A function to execute a named program
			with arguments
*/

execprog(cmd)

char *cmd;	/*  Incoming command line to execute  */

{
	char *sp;		/* temporary string pointer */
	char f1[38];		/* FCB1 area (not initialized) */
	char f2[38];		/* FCB2 area (not initialized) */
	char prog[NSTRING];	/* program filespec */
	char tail[NSTRING];	/* command tail with length byte */
#if ~TURBO
	union REGS rg;		/* parameters for dos call  */
#endif
	struct SREGS segreg;	/* segment registers for dos call */
	struct pblock {		/* EXEC parameter block */
		short envptr;	/* 2 byte pointer to environment string */
		char *cline;	/* 4 byte pointer to command line */
		char *fcb1;	/* 4 byte pointer to FCB at PSP+5Ch */
		char *fcb2;	/* 4 byte pointer to FCB at PSP+6Ch */
	} pblock;
	char *flook();

	/* parse the command name from the command line */
	sp = prog;
	while (*cmd && *cmd != ' ' && *cmd != '\t' && *cmd != '/')
		*sp++ = *cmd++;
	*sp = 0;

	/* take the remainder as command tail */
	tail[0] = (char)(strlen(cmd)); /* record the byte length */
	strcpy(&tail[1], cmd);
	tail[tail[0]+1] = '\r';

	/* look up the program on the path trying various extensions */
	if ((sp = flook(strcat(prog, ".com"), TRUE)) == NULL) {
		strcpy(&prog[strlen(prog)-4], ".exe");
		if ((sp = flook(prog, TRUE)) == NULL) {
			prog[strlen(prog)-4] = 0;	/* trim extension */
			if ((sp = flook(prog, TRUE)) == NULL)  {
				mlwrite("[Program not found]");
				return(FALSE);
			}
		}
	}
	strcpy(prog, sp);

	/* get a pointer to this PSPs environment segment number */
	segread(&segreg);

	/* set up the EXEC parameter block */
	pblock.envptr = 0;	/* make the child inherit the parents env */
	pblock.fcb1 = f1;	/* point to a blank FCB */
	pblock.fcb2 = f2;	/* point to a blank FCB */
	pblock.cline = tail;	/* parameter line pointer */

	/* and make the call */
	rg.h.ax = 0x4b00;	/* EXEC Execute a Program */
#if	AZTEC
	rg.x.ds = ((unsigned long)(prog) >> 16);	/* program name ptr */
#else
	segreg.ds = ((unsigned long)(prog) >> 16);	/* program name ptr */
#endif
	rg.x.dx = (unsigned int)(prog);
#if	AZTEC
	rg.x.es = rg.x.ds;
	/*rg.x.es = ((unsigned long)(&pblock) >> 16);	* set up param block ptr */
#else
	segreg.es = ((unsigned long)(&pblock) >> 16);	/* set up param block ptr */
#endif
	rg.x.bx = (unsigned int)(&pblock);
#if	LATTICE
#define	CFLAG	1
	if ((intdosx(&rg, &rg, &segreg) & CFLAG) == 0) {
		rg.h.ah = 0x4d;	/* get child process return code */
		intdos(&rg, &rg);	/* go do it */
		rval = rg.x.ax;	/* save child's return code */
	} else
		rval = -_oserr;		/* failed child call */
#endif
#if	AZTEC
#define	CFLAG	1
	if ((sysint(0x21, &rg, &rg) & CFLAG) == 0) {
		rg.h.ah = 0x4d;	/* get child process return code */
		sysint(0x21, &rg, &rg);	/* go do it */
		rval = rg.x.ax;	/* save child's return code */
	} else		
		rval = -errno;		/* failed child call */
#endif
#if	TURBO
	int86x(0x21, &rg, &rg, &segreg);
	if (rg.x.cflag == 0) {
		rg.h.ah = 0x4d;	/* get child process return code */
		intd();		/* go do it */
		rval = rg.x.ax;	/* save child's return code */
	} else
		rval = -_doserrno;	/* failed child call */
#endif

	if (rval < 0)  {
		mlwrite("[Execution failed]");
		return FALSE;
	}
	return TRUE;
}
#endif

#if	MSDOS & MSC
/*	SHELLPROG: Execute a command in a subshell		*/

shellprog(cmd)

char *cmd;	/*  Incoming command line to execute  */

{
	char *shell;		/* Name of system command processor */
	char *p;		/* Temporary pointer */
	char swchar;		/* switch character to use */
	union REGS regs;	/* parameters for dos call */
	char comline[NSTRING];	/* constructed command line */
	char *getenv();

	/*  detect current switch character and set us up to use it */
	regs.h.ah = 0x37;	/*  get setting data  */
	regs.h.al = 0x00;	/*  get switch character  */
	intdos(&regs, &regs);
	swchar = (char)regs.h.dl;

	/*  get name of system shell  */
	if ((shell = getenv("COMSPEC")) == NULL)
		return(FALSE);		/*  No shell located	*/

	/* trim leading whitespace off the command */
	while (*cmd == ' ' || *cmd == '\t')	/*  find out if null command */
		cmd++;

	/**  If the command line is not empty, bring up the shell  **/
	/**  and execute the command.	Otherwise, bring up the     **/
	/**  shell in interactive mode.   **/

	if (*cmd) {
		strcpy(comline, shell);
		strcat(comline, " ");
		comline[strlen(comline) + 1] = 0;
		comline[strlen(comline)] = swchar;
		strcat(comline, "c ");
		strcat(comline, cmd);
		return(execprog(comline));
	} else
		return(execprog(shell));
}

/*	EXECPROG:	A function to execute a named program
			with arguments
*/

execprog(cmd)

char *cmd;	/*  Incoming command line to execute  */

{
    system(cmd);
    return(TRUE);
}
#endif

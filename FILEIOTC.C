/*
 * The routines in this file read and write ASCII files from the disk. All of
 * the knowledge about files are here.
 *
 ******** This is a special Turbo C version of fileio.c ********
 */

#include	<stdio.h>
#include	<io.h>
#include	<fcntl.h>
#include	"estruct.h"
#include	"edef.h"

#define open	_open
#define read	_read
#define write	_write
#define close	_close
#define creat	_creat


int ffp;		/* File handle, all functions. */
int eofflag;		/* end-of-file flag */
int errflag;		/* error flag */

/*
 * Open a file for reading.
 */
ffropen(fn)
char	*fn;
{
	if ((ffp=open(fn, O_RDONLY)) == -1)
		return (FIOFNF);
	eofflag = FALSE;
	return (FIOSUC);
}

/*
 * Open a file for writing. Return TRUE if all is well, and FALSE on error
 * (cannot create).
 */
ffwopen(fn)
char	*fn;
{
	if ((ffp=creat(fn, 0)) == -1) {
		mlwrite("Cannot open file for writing");
		return (FIOERR);
	}
	return (FIOSUC);
}

/*
 * Close a file. Should look at the status in all systems.
 */
ffclose()
{
#if	MSDOS & CTRLZ
	char ctrlz = 26;
	write(ffp, &ctrlz, 1);		/* add a ^Z at the end of the file */
#endif
	
	/* free this since we do not need it anymore */
	if (fline) {
		free(fline);
		fline = NULL;
	}

	if (close(ffp) != 0) {
		mlwrite("Error closing file");
		return(FIOERR);
	}
	return(FIOSUC);
}

/*
 * Write a line to the already opened file. The "buf" points to the buffer,
 * and the "nbuf" is its length, less the free newline. Return the status.
 * Check only at the newline.
 */
ffputline(buf, nbuf, binary, lmode)
char	*buf;
{
	static char cr[2] = "\r\n";
	int sizecr;
#if	CRYPT
	char cline[NLINE+1];	/* encrypted line */
	int n;

	if (cryptflag)
		while ( !errflag && nbuf > 0 ) {
			n = ( nbuf >= NLINE ) ? NLINE : nbuf;
			strncpy(cline, buf, n);
			crypt(cline, n);
			if ( write( ffp, cline, n ) < n )
				errflag = TRUE;
			buf += n;
			nbuf -= n;
		}
	else
#endif
		if ( write(ffp, buf, nbuf) < nbuf )
			errflag = TRUE;

	sizecr = binary ? 1 : 2;
	if ( errflag || lmode & LMEOL && write(ffp, cr, sizecr) < sizecr) {
		mlwrite("Write I/O error");
		errflag = FALSE;
		return (FIOERR);
	}

	return (FIOSUC);
}

/*
 * Provide buffered input for low-level reads
 */
#define IOBUFSIZ	2048
static near getbufc( int eol )  {

	static unsigned char buf[IOBUFSIZ];
	static unsigned char *bufp;
	static int buff = 0, bufs = EOF;
	int c;

	if ( bufs != EOF ) 
		return ( c = bufs,  bufs = EOF,  c );

	if ( buff <= 0 )	/* read another chunk */
		if ( (buff = read(ffp, buf, IOBUFSIZ)) <= 0 )  {
			if ( buff < 0 )
				errflag = TRUE;
			return EOF;
		}
		else
			bufp = buf;

	if ( eol == '\n' && *bufp == '\r' )  {	/* convert "\r\n" to '\n' */

		/* does bufp+1 point into the buffer? */
		if ( bufp + 1 < &buf[ IOBUFSIZ ] )  {
			if ( bufp[ 1 ] == '\n' )  {
				buff -= 2;	/* eat 2 chars */
				bufp += 2;
				return '\n';	/* drop '\r' */
			}

		} else {

			buff = 0;	/* make sure a new chunk is read */
			if ( ( bufs = getbufc( eol ) ) == '\n' )
				return ( bufs = EOF,  '\n' );
			else
				return '\r';
		}
	}

	--buff;
	return *bufp++;
}

/*
 * Read a line from a file, and store the bytes in the supplied buffer. The
 * "nbuf" is the length of the buffer. Complain about long lines and lines
 * at the end of the file that don't have a newline present. Check for I/O
 * errors too. Return status.
 */
ffgetline(lengthp, eol, lmodep)
int *lengthp;
int eol;
char *lmodep;
{
	register int c;		/* current character read */
	register int i;		/* current index into fline */
	extern char *realloc( char *, unsigned );

	/* if we are at the end...return it */
	if (eofflag)
		return(FIOEOF);

	/* dump fline if it ended up too big */
	if (flen > NSTRING) {
		free(fline);
		fline = NULL;
	}

	/* if we don't have an fline, allocate one */
	if (fline == NULL)
		if ((fline = malloc(flen = NSTRING)) == NULL)
			return(FIOMEM);

	/* read the line in */
	*lmodep = 0;
	i = 0;
	while ((c = getbufc(eol)) != EOF && c != eol) {
		fline[i] = c;
		if ( ++i >= MAXLINE )
			break;
		/* if it's longer, get more room */
		if (i >= flen) {
			if ((fline = realloc(fline, flen+NSTRING)) == NULL)
				return(FIOMEM);
			flen += NSTRING;
		}
	}
	/* test for any errors that might have occurred */
	if (c == EOF) {
		if (errflag) {
			mlwrite("File read error");
			errflag = FALSE;
			return(FIOERR);
		}

		if (i != 0)
			*lmodep |= LMEOF,  eofflag = TRUE;
		else
			return(FIOEOF);
	}
	else if (c == eol)
		*lmodep |= LMEOL;

	/* terminate and decrypt the string */
	fline[i] = 0;
#if	CRYPT
	if (cryptflag)
		crypt(fline, i);
#endif
	*lengthp = i;
	return(FIOSUC);
}


int fexist(fname)	/* does <fname> exist on disk? */
char *fname;		/* file to check for existence */

{
	int fd;

	/* try to open the file for reading */
	fd = open(fname, O_RDONLY);

	/* if it fails, just return false! */
	if (fd == EOF)
		return(FALSE);

	/* otherwise, close it and report true */
	close(fd);
	return(TRUE);
}

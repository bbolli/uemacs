
               /***********************************************\
               *                                               *
               *                    aline.h                    *
               *       Common include file for C interface     *
               *       to low level Line A calls               *
               *                                               *
               *       J.R. Bammi                              *
               *         decvax!cwruecmp!bammi                 *
               *         bammi%cwru.edu.CSNET                  *
               *         bammi@cwru.edu.ARPA                   *
               *         CIS: 71515,155                        *
               *                                               *
               \***********************************************/

#define	WORD	int
#define	VOID	void

/*****************************************************************************\
*                                                                            *
*                                 Defines                                     *
*                                                                            *
\*****************************************************************************/

/*
 *  Object colors (default pallette)
 *
 */
#define WHITE    0
#define BLACK    1
#define RED      2
#define GREEN    3
#define BLUE     4
#define CYAN     5
#define YELLOW   6
#define MAGENTA  7
#define LWHITE   8
#define LBLACK   9
#define LRED     10
#define LGREEN   11
#define LBLUE    12
#define LCYAN    13
#define LYELLOW  14
#define LMAGENTA 15


/* 
 * Vdi writing modes
 *
 */
#define MD_REPLACE 1
#define MD_TRANS   2
#define MD_XOR     3
#define MD_ERASE   4


/*
 * Raster Op Codes
 *
 */
#define ALL_WHITE  0
#define S_AND_D    1
#define	S_AND_NOTD 2
#define S_ONLY     3
#define NOTS_AND_D 4
#define	D_ONLY     5
#define S_XOR_D    6
#define S_OR_D     7
#define	NOT_SORD   8
#define	NOT_SXORD  9
#define D_INVERT  10
#define	NOT_D     11
#define	S_OR_NOTD 12
#define NOTS_OR_D 13
#define	NOT_SANDD 14
#define ALL_BLACK 15

/*
 * Sprite formats
 *
 */
#define SP_VDI     0
#define SP_XOR     1

/*
 * Line A Opcodes
 *
 */
#define INIT		0
#define PUTPIXEL	1
#define GETPIXEL	2
#define LINE		3
#define HLINE		4
#define RECTANGLE	5
#define FPOLYGON	6
#define BITBLT		7
#define TEXTBLT		8
#define SHOWMOUSE	9
#define HIDEMOUSE	10
#define TRANMOUSE	11
#define USPRITE		12
#define DSPRITE		13
#define CPYRASTER	14
#define FSEEDFILL	15      /* ROM TOS only */


/*****************************************************************************\
*                                                                            *
*                                 Types                                       *
*                                                                            *
\*****************************************************************************/

       /*
        * Global Variables at negative offsets from the Line A parameter
        * block address returned by init. (I have no way of telling if this
        * list is complete).
        *
        */
/* Name   Offset  Type Description                                          */
/* --------------------------------------------------------------------------*/
/* V_Y_MAX    -4   W   Max Y pixel value of the screen                      */
/* V_STATUS   -6   W   Text Status byte                                     */
/*                       Bit   Field           Zero            One          */
/*                       0     cursor flash    disabled        enabled      */
/*                       1     flash state     off             on           */
/*                       2     cursor visible  no              yes          */
/*                       3     end of line     no-wrap         wrap         */
/*                       4     inverse video   on              off          */
/*                       5     cursor saved    false           true         */
/* V_OFF_AD  -10   L   Font offset table address                            */
/* V_X_MAX   -12   W   Max X pixel value                                    */
/* V_FNT_WR  -14   W   Width of Font Form in bytes (see type FONT below)    */
/* V_FNT_ST  -16   W   First font ASCII code (first_ade)                    */
/* V_FNT_ND  -18   W   Last  font ASCII code (last_ade )                    */
/* V_FNT_AD  -22   L   Font Form address                                    */
/*                     Mono Spaced, 8 pixels wide and byte aligned, any ht. */
/* V_CUR_TIM -23   B   Cursor countdown timer                               */
/* V_CUR_CNT -24   B   Cursor flash interval( in frames)                    */
/* V_CUR_CY  -26   W   Y cursor position                                    */
/* V_CUR_CX  -28   W   X cursor position                                    */
/* V_CUR_OFF -30   W   Offset from screen base to first cell (bytes)        */
/* V_CUR_AD  -34   L   Current cursor address                               */
/* V_COL_FG  -36   W   Foreground color index                               */
/* V_COL_BG  -38   W   Background color index                               */
/* V_CEL_WR  -40   W   Offset to next vertical cell (bytes)                 */
/* V_CEL_MY  -42   W   Max cells high - 1                                   */
/* V_CEL_MX  -44   W   Max cells across - 1                                 */
/* V_CEL_HT  -46   W   Cell height in pixels (font form's height)           */
/* --------------------------------------------------------------------------*/

/*
 * Atari finally named these variables
 * so here they are
 *
 */
typedef struct {
       WORD    V_CEL_HT;    /* *((WORD  *)((char  *)aline - (char  *)46L)) */
       WORD    V_CEL_MX;    /* *((WORD  *)((char  *)aline - (char  *)44L)) */
       WORD    V_CEL_MY;    /* *((WORD  *)((char  *)aline - (char  *)42L)) */
       WORD    V_CEL_WR;    /* *((WORD  *)((char  *)aline - (char  *)40L)) */
       WORD    V_COL_BG;    /* *((WORD  *)((char  *)aline - (char  *)38L)) */
       WORD    V_COL_FG;    /* *((WORD  *)((char  *)aline - (char  *)36L)) */
       char    *V_CUR_AD;   /* *((char **)((char **)aline - (char **)34L)) */
       WORD    V_CUR_OFF;   /* *((WORD  *)((char  *)aline - (char  *)30L)) */
       WORD    V_CUR_CX;    /* *((WORD  *)((char  *)aline - (char  *)28L)) */
       WORD    V_CUR_CY;    /* *((WORD  *)((char  *)aline - (char  *)26L)) */
       WORD    V_CUR_CNT;   /* *((char  *)((char  *)aline - (char  *)24L)) */
/*     char    V_CUR_TIM;    *((char  *)((char  *)aline - (char  *)23L))   */
       char    **V_FNT_AD;  /* *((char **)((char **)aline - (char **)22L)) */
       WORD    V_FNT_ND;    /* *((WORD  *)((char  *)aline - (char  *)18L)) */
       WORD    V_FNT_ST;    /* *((WORD  *)((char  *)aline - (char  *)16L)) */
       WORD    V_FNT_WR;    /* *((WORD  *)((char  *)aline - (char  *)14L)) */
       WORD    V_X_MAX;     /* *((WORD  *)((char  *)aline - (char  *)12L)) */
       char    **V_OFF_AD;  /* *((char **)((char **)aline - (char **)10L)) */
       WORD    V_STATUS;    /* *((WORD  *)((char  *)aline - (char  *) 6L)) */
       WORD    V_Y_MAX;     /* *((WORD  *)((char  *)aline - (char  *) 4L)) */
       WORD    xxdummy;     /* *((WORD  *)((char  *)aline - (char  *) 2L)) */
} NLINEA;

       /* A pointer to the type LINEA is returned by the Line A init call
        * ($A000), in registers A0 and D0.
         * This pointer is saved in the global variable 'aline'.
        *
        */
typedef struct {

/* Type    Name       Offset   Function                    Comments                 */
/* ------------------------------------------------------------------------- */
   WORD   VPLANES;    /*  0  # of Planes        Also see CurrRez             */
   WORD   VWRAP;      /*  2  Bytes / scan line    "    "    "                */
                      /*     VWRAP can be changed to implement special effect*/
                      /*     Doubling VWRAP will skip every other scan line  */
                      /*                                                     */
                      /*                                                     */
   WORD   *CONTRL;    /*  4  Ptr to CONTRL Array  Contrl gets set to this    */
   WORD   *INTIN;     /*  8  Ptr to INTIN  Array  Intin  gets set to this    */
   WORD   *PTSIN;     /* 12  Ptr to PTSIN  Array  Ptsin  gets set to this    */
   WORD   *INTOUT;    /* 16  Ptr to INTOUT Array  Intout gets set to this    */
   WORD   *PTSOUT;    /* 20  Ptr to PTSOUT Array  Ptsout gets set to this    */
                      /*     CONTRL is the control array                     */
                      /*     INTIN is the array of input parameters          */
                      /*     PTSIN is the array of input coordinates         */
                      /*          Even entrys are X coordinate               */
                      /*          Odd  entrys are corresponding Y coodinates */
                      /*     INTOUT is the array of output parameters        */
                      /*     PTSOUT is the array of output coordinates       */
                      /*        organizes like PTSIN.                        */
                      /*                                                     */
   WORD   COLBIT0;    /* 24  Plane 0 Color Value  All Three Rez's            */
   WORD   COLBIT1;    /* 26  Plane 1 Color Value  Med and Low Rez only       */
   WORD   COLBIT2;    /* 28  Plane 2 Color Value  Low Rez only               */
   WORD   COLBIT3;    /* 30  Plane 3 Color Value  Low Rez Only               */
                      /*     Foreground color COLBIT0 + 2*COLBIT1 + 4*COLBIT2*/
                      /*                      + 8*COLBIT3                    */
                      /*                                                     */
                      /*                                                     */
   WORD   LSTLIN;     /* 32  Always set to -1, Done for you in init_aline()  */
                      /*     Does anyone know what it is supposed to be?     */
                      /*                                                     */
   WORD   LNMASK;     /* 34  Linemask used when drawing lines, same as Vdi's */
                      /*     line style                                      */
                      /*                                                     */
   WORD   WMODE;      /* 36  Writing mode                                    */
                      /*     0=Replace Mode-Replace all bits in Dest with src*/
                      /*     1=Trans. Mode-Only additional bits in src set(OR*/
                      /*     2=Xor Mode- Src XOR Dest                        */
                      /*     3=Inverse Trans.- (NOT src) Or Dest             */
                      /*     Values upto 16 are permitted                    */
                      /*                                                     */
   WORD   X1;         /* 38  X1 coordinate used in various calls             */
                      /*                                                     */
   WORD   Y1;         /* 40  Y1 coordinate used in various calls             */
                      /*                                                     */
   WORD   X2;         /* 42  X2 coordinate used in various calls             */
                      /*                                                     */
   WORD   Y2;         /* 44  Y2 coordinate used in various calls             */
                      /*                                                     */
                      /*                                                     */
   WORD   *PATPTR;    /* 46  Pointer to current fill pattern                 */
                      /*     Must be integral power of 2 (words) in length   */
   WORD   PATMSK;     /* 50  I don't know why they call it a mask. It is in  */
                      /*     reality the length in words of the current patt.*/
   WORD   MFILL;      /* 52  Multi Plane fill flag 1 == Current fill Pattern */
                      /*     is for Muti Plane.                              */
                      /*                                                     */
                      /*                                                     */
   WORD   CLIP;       /* 54  Clipping Flag 1 == TRUE                         */
   WORD   XMINCL;     /* 56  Min X of clipping window                        */
   WORD   YMINCL;     /* 58  Min Y of clipping window                        */
   WORD   XMAXCL;     /* 60  Max X of clipping window                        */
   WORD   YMAXCL;     /* 62  Max Y of clipping window                        */
                      /*                                                     */
                      /*                                                     */
   WORD   XDDA;       /* 64  Accumulator for Scaling, Must be set to 0x08000 */
                      /*     before each call to Text Blt. Done for you in   */
                      /*     in aline_text()                                 */
   WORD   DDAINC;     /* 66  Scaling factor - Fractional amount to scale char*/
                      /*     When scaling up = 256 *(Size-Textsize)/Textsize */
                      /*     When scaling down = 256*(Size)/Textsize         */
                      /*     scaling down does not work                      */
   WORD   SCALDIR;    /* 68  Scaling direction 0 == down                     */
   WORD   MONO;       /* 70  Mono flag 0== current font is a propotional font*/
                      /*     Its Ok for Thickening to increase the width of  */
                      /*     the current character.                          */
                      /*     1 == current font is mono spaced, so thickening */
                      /*     may not increase the width of the current char  */
                      /*                                                     */
   WORD   SOURCEX;    /* 72  X coordinate of character in the font form      */
                      /*     SOURCEX is caluclated from info in the font     */
                      /*     header for the current font (see FONT type)     */
                      /*     SOURCEX = off_table[char-first_ade]             */
                      /*     SOURCEX is calculated for you in aline_text()   */
                      /*     The pointer to a table of font header for the   */
                      /*     internal fonts is returned in A2 on init (A000) */
   WORD   SOURCEY;    /* 74  Y coodinate of character in the font form       */
                      /*     Typically set to 0 (top line of font form)      */
   WORD   DESTX;      /* 76  X coordinate of character on screen             */
   WORD   DESTY;      /* 78  Y coordinate of character on screen             */
   WORD   DELX;       /* 80  Width of Character                              */
                      /*     Difference between two SOURCEX's                */
   WORD   DELY;       /* 82  Height of Character                             */
                      /*     form_height field of FONT_HEAD of current font  */
   WORD   *FBASE;     /* 84  Pointer to start of font form                   */
   WORD   FWIDTH;     /* 88  Width of the current font's form                */
                      /*                                                     */
   WORD   STYLE;      /* 90  Vector of style flags                           */
                      /*     Bit 0 = Thicken Flag                            */
                      /*     Bit 1 = Lighten Flag                            */
                      /*     Bit 2 = Skewing Flag                            */
                      /*     Bit 3 = Underline Flag (ignored)                */
                      /*     Bit 4 = Outline Flag                            */
                      /*                                                     */
   WORD   LITEMASK;   /* 92  Mask used for lightening text                   */
                      /*     The Mask is picked up from the font header      */
   WORD   SKEWMASK;   /* 94  Mask used for skewing text                      */
                      /*     The Mask is picked up from the font header      */
   WORD   WEIGHT;     /* 96  The number of bits by which to thicken text     */
                      /*     The number is picked up from the font header    */
   WORD   ROFF;       /* 98  Offset above baseline when skewing              */
                      /*     Again picked up from the font header            */
                      /*                                                     */
   WORD   LOFF;       /* 100 Offset below character baseline when skewing    */
                      /*     Again picked up from the font header            */
                      /*                                                     */
   WORD   SCALE;      /* 102 Scaling Flag 1 == true                          */
                      /*                                                     */
   WORD   CHUP;       /* 104 Character rotation vector.                      */
                      /*     0 = normal (0 degrees)                          */
                      /*     1800 = 180 degrees                              */
                      /*     2700 = 270 degrees                              */
                      /*                                                     */
   WORD   TEXTFG;     /* 106 Text foreground color                           */
                      /*                                                     */
   char   *SCRTCHP;   /* 108 Address of buffer required for creating special */
                      /*     text effects. The size of this buffer should be */
                      /*     1K according the Internals. The Atari document  */
                      /*     of course does not talk about such things :-)   */
                      /*                                                     */
   WORD   SCRPT2;     /* 112 The offset of the scaling buffer buffer in above*/
                      /*     buffer. Internals suggests an offset of 0x0040  */
                      /*     As usual the Atari document does'nt say a thing */
                      /*                                                     */
   WORD   TEXTBG;     /* 114 Text background color (Ram Vdi only)            */
                      /*     used for the BitBlt writing modes (4-19) only   */
                      /*                                                     */
   WORD   COPYTRAN;   /* 116 Copy raster form type flag (Ram vdi only)       */
                      /*     0 => Opaque type                                */
                      /*          n-plane source  ->  n-plane dest           */
                      /*              BitBlt writing modes (4-19)            */
                      /*    ~0 => Transparent type                           */
                      /*          1-plane source  ->  n-plane dest           */
                      /*              Vdi writing modes (1-3)                */
                      /*                                                     */
   WORD(*SEEDABORT)();/* 118 Pointer to function returning int, which is     */
                      /*     called by the fill logic to allow the fill to   */
                      /*     be aborted. If the routine returns FALSE (0)    */
                      /*     the fill is not aborted. If it returns TRUE (~0)*/
                      /*     the fill is aborted                             */
/* ------------------------------------------------------------------------- */

} LINEA;             /*       P H E W !!!!!                                  */



 /* A pointer to array of type FONT is returned by the Line A init call
  * ($A000), in regsister A1.
  * This pointer is saved in the global array variable 'fonts[]'.
  *
  */

typedef struct _font {

/* Type    Name       Offset   Function                    Comments          */
/* ------------------------------------------------------------------------- */
   WORD   font_id;    /*  0 Font face identifier  1 == system font           */
                      /*                                                     */
   WORD   size;       /*  2 Font size in points                              */
                      /*                                                     */
   char   name[32];   /*  4 Face name                                        */
                      /*                                                     */
   WORD   first_ade;  /* 36 Lowest ADE value in the face (lowest ASCII value */
                      /*    of displayable character).                       */
                      /*                                                     */
   WORD   last_ade;   /* 38 Highest ADE value in the face (highest ASCII valu*/
                      /*    of displayable character).                       */
                      /*                                                     */
   WORD   top;        /* 40 Distance of top line relative to baseline        */
                      /*                                                     */
   WORD   ascent;     /* 42 Distance of ascent line relative to baseline     */
                      /*                                                     */
   WORD   half;       /* 44 Distance of half line relative to baseline       */
                      /*                                                     */
   WORD   descent;    /* 46 Distance of decent line relative to baseline     */
                      /*                                                     */
   WORD   bottom;     /* 48 Distance of bottom line relative to baseline     */
                      /*    All distances are measured in absolute values    */
                      /*    rather than as offsets. They are always +ve      */
                      /*                                                     */
 WORD max_char_width; /* 50 Width of the widest character in font            */
                      /*                                                     */
 WORD max_cell_width; /* 52 Width of the widest cell character cell in face  */
                      /*                                                     */
   WORD left_offset;  /* 54 Left Offset see Vdi appendix G                   */
                      /*                                                     */
   WORD right_offset; /* 56 Right offset   "      "     "                    */
                      /*                                                     */
   WORD   thicken;    /* 58 Number of pixels by which to thicken characters  */
                      /*                                                     */
   WORD   ul_size;    /* 60 Width in  pixels of the underline                */
                      /*                                                     */
   WORD   lighten;    /* 62 The mask used to lighten characters              */
                      /*                                                     */
   WORD   skew;       /* 64 The mask used to determine when to perform       */
                      /*    additional rotation on the character to perform  */
                      /*    skewing                                          */
                      /*                                                     */
   WORD   flags;      /* 66 Flags                                            */
                      /*      bit 0 set if default system font               */
                      /*      bit 1 set if horiz offset table should be used */
                      /*      bit 2 byte-swap flag (thanks to Intel idiots)  */
                      /*      bit 3 set if mono spaced font                  */
                      /*                                                     */
   char   *h_table;   /* 68 Pointer to horizontal offset table               */
                      /*                                                     */
   WORD   *off_table; /* 72 Pointer to character offset table                */
                      /*                                                     */
   char   *dat_table; /* 76 Pointer to font data                             */
                      /*                                                     */
   WORD   form_width; /* 80 Form width (#of bytes /scanline in font data)    */
                      /*                                                     */
   WORD   form_height;/* 82 Form height (#of scanlines in font data)         */
                      /*                                                     */
 struct _font *next_font;  /* 84 Pointer to next font in face                */
                      /*                                                     */
/* ------------------------------------------------------------------------- */
} FONT;

       
       /*
        * OP_TAB type required for Bit Blt parameter block.
        * each entry defines the logic operation to apply for
        * the 4 Fore/Back ground bit combinations
        */
typedef struct {

/* Type    Name       Offset   Function             Comments                 */
/* ------------------------------------------------------------------------- */
   char   fg0bg0;     /* 0     Logic op to employ when both FG and BG are 0  */
   char   fg0bg1;     /* 1     Logic op to employ when FG = 0 and BG = 1     */
   char   fg1bg0;     /* 2     Logic op to employ when FG = 1 and BG = 0     */
   char   fg1bg1;     /* 3     Logic op to employ when both FG and BG are 1  */
/* ------------------------------------------------------------------------- */
} OP_TAB;


/*
 * Source and destination description blocks
 */
typedef struct  {
       WORD    bl_xmin;                /* Minimum x                    */
       WORD    bl_ymin;                /* Minimum y                    */
       char    *bl_form;               /* Word aligned memory form     */
       WORD    bl_nxwd;                /* Offset to next word in line  */
       WORD    bl_nxln;                /* Offset to next line in plane */
       WORD    bl_nxpl;                /* Offset to next plane         */
}SDDB;

       /* Offsets to next word in plane */
#define HI_NXWD        2
#define MED_NXWD       4
#define LOW_NXWD       8

       /* Scan line widths of the screen */
#define HI_NXLN        80
#define MED_NXLN       160
#define LOW_NXLN       160

       /*
        * Offsets between planes - always the same due to
        * the way the STs video memory is laid out
         */
#define NXPL           2

       /* 
        * Bit Blt Parameter Block Type (for function $A007)
        *
        */

typedef struct {

/* Type           Name           Offset   Function          Comments         */
/* ------------------------------------------------------------------------- */
   WORD           bb_b_wd;     /*       width of block in pixels             */
   WORD           bb_b_ht;     /*       height of block in pixels            */
   WORD           bb_plane_ct; /*       number of planes to blit             */
   WORD           bb_fg_col;   /*       foreground color                     */
   WORD           bb_bg_col;   /*       back   ground color                  */
   OP_TAB  bb_op_tab;          /*       logic for fg x bg combination        */
   SDDB           bb_s;        /*       source info block                    */
   SDDB           bb_d;        /*       destination info block               */
   WORD           *bb_p_addr;  /*       pattern buffer address               */
   WORD           bb_p_nxln;   /*       offset to next line in pattern       */
   WORD           bb_p_nxpl;   /*       offset to next plane in pattern      */
   WORD           bb_p_mask;   /*       pattern index mask                   */
   char           bb_fill[24]; /*       work space                           */
/* ------------------------------------------------------------------------- */
} BBPB;


/*
 * Memory Form Definition Block
 *
 */
typedef struct
{
       char            *fd_addr;    /* Addrerss of upper left corner of firs*/
                                    /* plane of raster area. If NULL then   */
                                    /* MFDB is for a physical device        */
       WORD            fd_w;        /* Form Width in Pixels                 */
       WORD            fd_h;        /* Form Height in Pixels                */
       WORD            fd_wdwidth;  /* Form Width in words (fd_w/sizeof(int)*/
       WORD            fd_stand;    /* Form format 0= device spec 1=standard*/
       WORD            fd_nplanes;  /* Number of memory planes              */
       WORD            fd_r1;       /* Reserved                             */
       WORD            fd_r2;       /* Reserved                             */
       WORD            fd_r3;       /* Reserved                             */
} MFDB;




/*
 * Sprite definition block
 *
 */
typedef struct
{
       WORD    sp_xhot;                /* Offset to X hot spot           */
       WORD    sp_yhot;                /* Offset to Y hot spot           */
       WORD    sp_format;              /* Format SP_VDI or SP_XOR        */
       WORD    sp_bg;                  /* Background color               */
       WORD    sp_fg;                  /* Foregroud color                */
       WORD    sp_data[32];            /* Sprite data -                  */
                                       /* Alternating words of back/fore */
                                       /* ground data                    */
                                       /* Note that:                     */
                                       /*   sprite save block is         */
                                       /*  10+VPLANES*64 bytes long      */

} SFORM;

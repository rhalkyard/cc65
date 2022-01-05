/*****************************************************************************/
/*                                                                           */
/*				   bbc.h			       	     */
/*                                                                           */
/*		    BBC Micro system specific definitions		     */
/*                                                                           */
/*                                                                           */
/* (C) 2005 Dominic Beesley <cc65@brahms.demon.co.uk>			     */
/* based up the atari.h which was the work of...			     */
/*                                                                           */
/*          Mark Keates <markk@dendrite.co.uk>                               */
/*          Freddy Offenga <taf_offenga@yahoo.com>                           */
/*          Christian Groessler <cpg@aladdin.de>                             */
/*                                                                           */
/*                                                                           */
/* This software is provided 'as-is', without any expressed or implied       */
/* warranty.  In no event will the authors be held liable for any damages    */
/* arising from the use of this software.                                    */
/*                                                                           */
/* Permission is granted to anyone to use this software for any purpose,     */
/* including commercial applications, and to alter it and redistribute it    */
/* freely, subject to the following restrictions:                            */
/*                                                                           */
/* 1. The origin of this software must not be misrepresented; you must not   */
/*    claim that you wrote the original software. If you use this software   */
/*    in a product, an acknowledgment in the product documentation would be  */
/*    appreciated but is not required.                                       */
/* 2. Altered source versions must be plainly marked as such, and must not   */
/*    be misrepresented as being the original software.                      */
/* 3. This notice may not be removed or altered from any source              */
/*    distribution.                                                          */
/*                                                                           */
/*****************************************************************************/



#ifndef _BBC_H
#define _BBC_H



/* Check for errors */
#if !defined(__BBC__)
#  error This module may only be used when compiling for the BBC/Master Series
#endif

#define CLOCKS_PER_SEC 100

/* Character codes */
#define CH_DELCHR	0x32   /* delete char under the cursor */
#define CH_ESC 	    	0x1B
#define CH_CURS_UP  	0x8B
#define CH_CURS_DOWN 	0x8A
#define CH_CURS_LEFT    0x88
#define CH_CURS_RIGHT   0x89

#define CH_TAB          0x09   /* tabulator */
#define CH_EOL          0x0D   /* end-of-line marker */
#define CH_CLR          0x0C   /* clear screen */
#define CH_BEL          0x07   /* bell */
#define CH_DEL          0x7F   /* back space (delete char to the left) */
#define CH_RUBOUT       0x7F   /* back space (old, deprecated) */
#define CH_DELLINE      0x00   /* delete line */
#define CH_INSLINE      0x00   /* insert line */
#define CH_ENTER	    0x0D   /* RETURN is 13 */

/* These are actually CTRL+F? */
#define CH_F1 	    	0x91
#define CH_F2 	    	0x92
#define CH_F3 	    	0x93
#define CH_F4 	    	0x94
#define CH_F5 	    	0x95
#define CH_F6 	    	0x96
#define CH_F7 	    	0x97
#define CH_F8 	    	0x98
#define CH_F9 	    	0x99
#define CH_F10 	    	0x90

#define CH_ULCORNER 	42
#define CH_URCORNER 	42
#define CH_LLCORNER 	42
#define CH_LRCORNER 	42
#define CH_TTEE     	42
#define CH_BTEE     	42
#define CH_LTEE     	42
#define CH_RTEE     	42
#define CH_CROSS    	42
#define CH_HLINE        45
#define CH_VLINE        58

/* color defines */

/* colour does work with conio for bbc */
#define COLOR_BLACK  	       	0
#define COLOR_WHITE  	       	7
#define COLOR_RED    	       	1
#define COLOR_CYAN      	    6
#define COLOR_VIOLET 	       	5
#define COLOR_GREEN  	        2
#define COLOR_BLUE   	       	4
#define COLOR_YELLOW 	       	3
#define COLOR_FLASH(color)      (8 + color)

/* BBC doesn't have these colours, map to an approximation */
#define COLOR_ORANGE 	       	COLOR_YELLOW
#define COLOR_BROWN  	       	COLOR_RED
#define COLOR_LIGHTRED       	COLOR_RED
#define COLOR_GRAY1  	       	COLOR_WHITE
#define COLOR_GRAY2  	        COLOR_YELLOW
#define COLOR_LIGHTGREEN     	COLOR_CYAN
#define COLOR_LIGHTBLUE      	COLOR_CYAN
#define COLOR_GRAY3  	       	COLOR_VIOLET

/* TGI colours */
#define TGI_COLOR_BLACK  	       	COLOR_BLACK
#define TGI_COLOR_WHITE  	       	COLOR_WHITE
#define TGI_COLOR_RED    	       	COLOR_RED
#define TGI_COLOR_CYAN      	    COLOR_CYAN
#define TGI_COLOR_VIOLET 	       	COLOR_VIOLET
#define TGI_COLOR_GREEN  	        COLOR_GREEN
#define TGI_COLOR_BLUE   	       	COLOR_BLUE
#define TGI_COLOR_YELLOW 	       	COLOR_YELLOW
#define TGI_COLOR_ORANGE 	       	COLOR_ORANGE
#define TGI_COLOR_BROWN  	       	COLOR_BROWN
#define TGI_COLOR_LIGHTRED       	COLOR_LIGHTRED
#define TGI_COLOR_GRAY1  	       	COLOR_GRAY1
#define TGI_COLOR_GRAY2  	       	COLOR_GRAY2
#define TGI_COLOR_LIGHTGREEN     	COLOR_LIGHTGREEN
#define TGI_COLOR_LIGHTBLUE      	COLOR_LIGHTBLUE
#define TGI_COLOR_GRAY3  	        COLOR_GRAY3

#endif 

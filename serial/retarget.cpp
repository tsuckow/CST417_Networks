/******************************************************************************/
/* RETARGET.C: 'Retarget' layer for target-dependent low level functions      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

struct __FILE { int handle; /* Add whatever you need here */ };
struct __FILE __stdout;
struct __FILE __stdin;
struct __FILE __stderr;

#include <stdio.h>
#include <rt_misc.h>

#pragma import(__use_no_semihosting_swi)

#include "serial.h"


int std::putc(int ch, FILE *f) {
	return fputc(ch,f);
}
int std::fputc(int ch, FILE *f) {
	int tmp;
	if (f->handle == __stdout.handle || f->handle == __stderr.handle) {
		if( ch == '\n' ) sendchar('\r');
		tmp = sendchar(ch);
		return (tmp);
	} else {
		return 1;
	}
}
int std::getc( FILE *f) {
	return fgetc(f);
}
int std::fgetc( FILE *f) {
	int tmp;
	if (f->handle == __stdin.handle) {
		tmp = getkey();
		if( tmp == '\r' ) tmp = getkey();
		return (tmp);
	} else {
		return EOF;
	}
}


int std::ferror(FILE *f) {
	/* Your implementation of ferror */
	return EOF;
}


extern "C" {
void _ttywrch(int ch) {
	if( ch == '\n' ) sendchar('\r');
	sendchar(ch);
}

void _sys_exit(int return_code) {
label:	goto label;  /* endless loop */
}
}

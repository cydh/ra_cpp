// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>  // for va_start, etc
#include <memory>    // for std::unique_ptr

#include "cbasetypes.h"
#include "showmsg.h"
#include "core.h" //[Ind] - For SERVER_TYPE

#include "libconfig.h"

using namespace std;

#ifdef WIN32
	#include "../common/winapi.h"

	#ifdef DEBUGLOGMAP
		#define DEBUGLOGPATH "log\\map-server.log"
	#endif
	#ifdef DEBUGLOGCHAR
		#define DEBUGLOGPATH "log\\char-server.log"
	#endif
	#ifdef DEBUGLOGLOGIN
		#define DEBUGLOGPATH "log\\login-server.log"
	#endif
#else
	#include <unistd.h>

	#ifdef DEBUGLOGMAP
		#define DEBUGLOGPATH "log/map-server.log"
	#endif
	#ifdef DEBUGLOGCHAR
		#define DEBUGLOGPATH "log/char-server.log"
	#endif
	#ifdef DEBUGLOGLOGIN
		#define DEBUGLOGPATH "log/login-server.log"
	#endif
#endif

///////////////////////////////////////////////////////////////////////////////
/// behavioral parameter.
/// when redirecting output:
/// if true prints escape sequences
/// if false removes the escape sequences
int stdout_with_ansisequence = 0;

int msg_silent = 0; //Specifies how silent the console is.
int console_msg_log = 0;//[Ind] msg error logging


#define is_console(file) (0!=isatty(fileno(file)))

std::string string_format(const std::string fmt_str, ...) {
    int final_n, n = ((int)fmt_str.size()) * 2; /* reserve 2 times as much as the length of the fmt_str */
    std::string str;
    std::unique_ptr<char[]> formatted;
    va_list ap;
    while(1) {
        formatted.reset(new char[n]); /* wrap the plain char array into the unique_ptr */
        strcpy(&formatted[0], fmt_str.c_str());
        va_start(ap, fmt_str);
        final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    return std::string(formatted.get());
}

//vprintf_without_ansiformats
int	VFPRINTF(FILE *file, const string fmt, va_list argptr)
{
	char *p, *q;
	string tempbuf;

	if(fmt.empty() )
		return 0;

	if( is_console(file) || stdout_with_ansisequence )
	{
		vfprintf(file, fmt.c_str(), argptr);
		return 0;
	}

	// Print everything to the buffer
	tempbuf = string_format(fmt,argptr);

	// start with processing
	p = tempbuf.front();
	while ((q = strchr(p, 0x1b)) != NULL)
	{	// find the escape character
		fprintf(file, "%.*s", (int)(q-p), p); // write up to the escape
		if( q[1]!='[' )
		{	// write the escape char (whatever purpose it has) 
			fprintf(file, "%.*s", 1, q);
			p=q+1;	//and start searching again
		}
		else
		{	// from here, we will skip the '\033['
			// we break at the first unprocessible position
			// assuming regular text is starting there

			// skip escape and bracket
			q=q+2;
			while(1)
			{
				if( ISDIGIT(*q) ) 
				{
					++q;
					// and next character
					continue;
				}
				else if( *q == ';' )
				{	// delimiter
					++q;
					// and next number
					continue;
				}
				else if( *q == 'm' )
				{	// \033[#;...;#m - Set Graphics Rendition (SGR)
					// set the attributes
				}
				else if( *q=='J' )
				{	// \033[#J - Erase Display (ED)
				}
				else if( *q=='K' )
				{	// \033[K  : clear line from actual position to end of the line
				}
				else if( *q == 'H' || *q == 'f' )
				{	// \033[#;#H - Cursor Position (CUP)
					// \033[#;#f - Horizontal & Vertical Position
				}
				else if( *q=='s' )
				{	// \033[s - Save Cursor Position (SCP)
				}
				else if( *q=='u' )
				{	// \033[u - Restore cursor position (RCP)
				}
				else if( *q == 'A' )
				{	// \033[#A - Cursor Up (CUU)
					// Moves the cursor UP # number of lines
				}
				else if( *q == 'B' )
				{	// \033[#B - Cursor Down (CUD)
					// Moves the cursor DOWN # number of lines
				}
				else if( *q == 'C' )
				{	// \033[#C - Cursor Forward (CUF)
					// Moves the cursor RIGHT # number of columns
				}
				else if( *q == 'D' )
				{	// \033[#D - Cursor Backward (CUB)
					// Moves the cursor LEFT # number of columns
				}
				else if( *q == 'E' )
				{	// \033[#E - Cursor Next Line (CNL)
					// Moves the cursor down the indicated # of rows, to column 1
				}
				else if( *q == 'F' )
				{	// \033[#F - Cursor Preceding Line (CPL)
					// Moves the cursor up the indicated # of rows, to column 1.
				}
				else if( *q == 'G' )
				{	// \033[#G - Cursor Horizontal Absolute (CHA)
					// Moves the cursor to indicated column in current row.
				}
				else if( *q == 'L' || *q == 'M' || *q == '@' || *q == 'P')
				{	// not implemented, just skip
				}
				else
				{	// no number nor valid sequencer
					// something is fishy, we break and give the current char free
					--q;
				}
				// skip the sequencer and search again
				p = q+1; 
				break;
			}// end while
		}
	}
	if (*p)	// write the rest of the buffer
		fprintf(file, "%s", p);
	return 0;
}
int	FPRINTF(FILE *file, const string fmt, ...)
{
	int ret;
	va_list argptr;
	va_start(argptr, fmt);
	ret = VFPRINTF(file,fmt,argptr);
	va_end(argptr);
	return ret;
}

#define FFLUSH fflush

#define STDOUT stdout
#define STDERR stderr


char timestamp_format[20] = ""; //For displaying Timestamps

int _vShowMessage(enum msg_type flag, const string string, va_list ap)
{
	va_list apcopy;
	string prefix;
#if defined(DEBUGLOGMAP) || defined(DEBUGLOGCHAR) || defined(DEBUGLOGLOGIN)
	FILE *fp;
#endif
	
	if (string.empty() ) {
		ShowError("Empty string passed to _vShowMessage().\n");
		return 1;
	}
	/**
	 * For the buildbot, these result in a EXIT_FAILURE from core.c when done reading the params.
	 **/
#if defined(BUILDBOT)
	if( flag == MSG_WARNING ||
	    flag == MSG_ERROR ||
	    flag == MSG_SQL ) {
		buildbotflag = 1;
	}
#endif
	if(
		( flag == MSG_WARNING && console_msg_log&1 ) ||
		( ( flag == MSG_ERROR || flag == MSG_SQL ) && console_msg_log&2 ) ||
		( flag == MSG_DEBUG && console_msg_log&4 ) ) {//[Ind]
		FILE *log = NULL;
		if( (log = fopen(SERVER_TYPE == ATHENA_SERVER_MAP ? "./log/map-msg_log.log" : "./log/unknown.log","a+")) ) {
			char timestring[255];
			time_t curtime;
			time(&curtime);
			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(log,"(%s) [ %s ] : ",
				timestring,
				flag == MSG_WARNING ? "Warning" :
				flag == MSG_ERROR ? "Error" :
				flag == MSG_SQL ? "SQL Error" :
				flag == MSG_DEBUG ? "Debug" :
				"Unknown");
			va_copy(apcopy, ap);
			vfprintf(log,string,apcopy);
			va_end(apcopy);
			fclose(log);
		}
	}
	if(
	    (flag == MSG_INFORMATION && msg_silent&1) ||
	    (flag == MSG_STATUS && msg_silent&2) ||
	    (flag == MSG_NOTICE && msg_silent&4) ||
	    (flag == MSG_WARNING && msg_silent&8) ||
	    (flag == MSG_ERROR && msg_silent&16) ||
	    (flag == MSG_SQL && msg_silent&16) ||
	    (flag == MSG_DEBUG && msg_silent&32)
	)
		return 0; //Do not print it.

	if (timestamp_format[0] && flag != MSG_NONE)
	{	//Display time format. [Skotlex]
		time_t t = time(NULL);
		strftime(prefix, 80, timestamp_format, localtime(&t));
	} else prefix[0]='\0';

	switch (flag) {
		case MSG_NONE: // direct printf replacement
			break;
		case MSG_STATUS: //Bright Green (To inform about good things)
			strcat(prefix,CL_GREEN"[Status]"CL_RESET":");
			break;
		case MSG_SQL: //Bright Violet (For dumping out anything related with SQL) <- Actually, this is mostly used for SQL errors with the database, as successes can as well just be anything else... [Skotlex]
			strcat(prefix,CL_MAGENTA"[SQL]"CL_RESET":");
			break;
		case MSG_INFORMATION: //Bright White (Variable information)
			strcat(prefix,CL_WHITE"[Info]"CL_RESET":");
			break;
		case MSG_NOTICE: //Bright White (Less than a warning)
			strcat(prefix,CL_WHITE"[Notice]"CL_RESET":");
			break;
		case MSG_WARNING: //Bright Yellow
			strcat(prefix,CL_YELLOW"[Warning]"CL_RESET":");
			break;
		case MSG_DEBUG: //Bright Cyan, important stuff!
			strcat(prefix,CL_CYAN"[Debug]"CL_RESET":");
			break;
		case MSG_ERROR: //Bright Red  (Regular errors)
			strcat(prefix,CL_RED"[Error]"CL_RESET":");
			break;
		case MSG_FATALERROR: //Bright Red (Fatal errors, abort(); if possible)
			strcat(prefix,CL_RED"[Fatal Error]"CL_RESET":");
			break;
		default:
			ShowError("In function _vShowMessage() -> Invalid flag passed.\n");
			return 1;
	}

	if (flag == MSG_ERROR || flag == MSG_FATALERROR || flag == MSG_SQL)
	{	//Send Errors to StdErr [Skotlex]
		FPRINTF(STDERR, "%s ", prefix);
		va_copy(apcopy, ap);
		VFPRINTF(STDERR, string, apcopy);
		va_end(apcopy);
		FFLUSH(STDERR);
	} else {
		if (flag != MSG_NONE)
			FPRINTF(STDOUT, "%s ", prefix);
		va_copy(apcopy, ap);
		VFPRINTF(STDOUT, string, apcopy);
		va_end(apcopy);
		FFLUSH(STDOUT);
	}

#if defined(DEBUGLOGMAP) || defined(DEBUGLOGCHAR) || defined(DEBUGLOGLOGIN)
	if(strlen(DEBUGLOGPATH) > 0) {
		fp=fopen(DEBUGLOGPATH,"a");
		if (fp == NULL)	{
			FPRINTF(STDERR, CL_RED"[ERROR]"CL_RESET": Could not open '"CL_WHITE"%s"CL_RESET"', access denied.\n", DEBUGLOGPATH);
			FFLUSH(STDERR);
		} else {
			fprintf(fp,"%s ", prefix);
			va_copy(apcopy, ap);
			vfprintf(fp,string,apcopy);
			va_end(apcopy);
			fclose(fp);
		}
	} else {
		FPRINTF(STDERR, CL_RED"[ERROR]"CL_RESET": DEBUGLOGPATH not defined!\n");
		FFLUSH(STDERR);
	}
#endif

	return 0;
}

void ClearScreen(void)
{
#ifndef _WIN32
	ShowMessage(CL_CLS);	// to prevent empty string passed messages
#endif
}
int _ShowMessage(enum msg_type flag, const string string, ...)
{
	int ret;
	va_list ap;
	va_start(ap, string);
	ret = _vShowMessage(flag, string, ap);
	va_end(ap);
	return ret;
}

// direct printf replacement
void ShowMessage(const string string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_NONE, string, ap);
	va_end(ap);
}
void ShowStatus(const string string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_STATUS, string, ap);
	va_end(ap);
}
void ShowSQL(const string string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_SQL, string, ap);
	va_end(ap);
}
void ShowInfo(const string string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_INFORMATION, string, ap);
	va_end(ap);
}
void ShowNotice(const string string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_NOTICE, string, ap);
	va_end(ap);
}
void ShowWarning(const string string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_WARNING, string, ap);
	va_end(ap);
}
void ShowConfigWarning(config_setting_t *config, const string string, ...)
{
	std::string buf;
	va_list ap;
	buf.append(string);
	buf.append(" ("+config_setting_source_file(config)+":"+config_setting_source_line(config)+")\n");
	va_start(ap, string);
	_vShowMessage(MSG_WARNING, buf, ap);
	va_end(ap);
}
void ShowDebug(const string string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_DEBUG, string, ap);
	va_end(ap);
}
void ShowError(const string string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_ERROR, string, ap);
	va_end(ap);
}
void ShowFatalError(const string string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_FATALERROR, string, ap);
	va_end(ap);
}



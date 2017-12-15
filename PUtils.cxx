//==============================================================================
// File:        PUtils.cxx
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
// Assorted utility routines
#include <Xm/Text.h>
#include <string.h>
#include "PUtils.h"

/*---------------------------------------------------------------------------
*/
/* set the string in a text widget */
void setTextString(Widget text, char *string)
{
//  Arg         wargs[1];
    
    XmTextSetString(text,string);
    
    /* this must be done after we set the text */
    /* or the ugly cursor will be visible */
//this was causing crashes for some reason,
//so use XmTextSetInsertionPosition() instead (PH 10/08/99)
//  XtSetArg(wargs[0], XmNcursorPosition, -1);
//  XtSetValues(text, wargs, 1);
// still causes crashes
// for a long time I was setting the insertion position to -1
// to hide the ugly cursor, but that causes crashes (on linux anyway),
// so change it to the end of the string
    XmTextSetInsertionPosition(text,strlen(string));
}

/* convert Xm string to char* of maximum length n */
/* returns string length */
int strncvtXm(char *out,XmString in,int n)
{
    XmStringContext     contxt;
    XmStringCharSet     charset;
    XmStringDirection   dir;
    Boolean             separator;
    char                *text;
    int                 len = 0;
    
    XmStringInitContext(&contxt, in);
    for (;;) {
        if (XmStringGetNextSegment(contxt,&text,&charset,&dir,&separator)) {
            strncpy(out+len,text,n-len);
            len += strlen(text);
            XtFree(text);
            if (len >= n) len = n;
            else if (!separator) continue;
        }
        break;
    }
    XmStringFreeContext(contxt);
    return (len);
}

/* set the string in a label widget */
void setLabelString(Widget label, char *string)
{
    int         n;
    Arg         wargs[2];
    XmString    new_str;
    
    // XmStringCreateLtoR translates '\n' into string separators
    new_str = XmStringCreateLtoR(string,XmFONTLIST_DEFAULT_TAG);
    if (!new_str) quit("no mem");
    
    /* set the label string */
    n = 0;
    XtSetArg(wargs[n], XmNlabelString, new_str);  ++n;
    XtSetValues(label,wargs,n);

    // must free our local XmString after setting the label
    XmStringFree(new_str);
}

// get string from a label
// - Note: You must call XtFree to free returned string when finished
char *getLabelString(Widget label)
{
    char        buff[256];
    Arg         wargs[1];
    XmString    str;

    XtSetArg(wargs[0], XmNlabelString, &str);
    XtGetValues(label,wargs,1);
    
    int len = strncvtXm(buff,str,256);
    
    char *out = XtMalloc(len + 1);
    if (out) {
        memcpy(out, buff, len+1);
    }
    return(out);
}


void setToggle(Widget toggle, int on)
{
    Arg wargs[1];
    
    XtSetArg(wargs[0], XmNset, on ? TRUE : FALSE);
    XtSetValues(toggle,wargs,1);
}




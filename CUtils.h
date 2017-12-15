//==============================================================================
// File:        CUtils.h
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
#ifndef __Utils_h__
#define __Utils_h__

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef VAX
#define Printf vaxPrintf    // avoid Printf/printf name conflict
#endif

void    setProgname(char *aFilename, char *aPathname);
double  double_time(void);
void    usleep_(unsigned long usec);
void    quit(char *msg);
int     Printf(char *,...);
void    SetPrintfOutput(char *buff, int size);

#ifdef  __cplusplus
}
#endif

#endif /* __Utils_h__ */

#ifndef _UTILS_H
#define _UTILS_H

/*#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"*/

char ** divideString (char *string, char separator);
char *strlwr (char *a);
int compareString (char *srt1, char *str2);

long charToInt (char * characterizedNumber);
char *intToChar (int number);

double charToDouble (char * characterizedNumber);
char *doubleToChar (double number);

int voidToInt (void *voidValue);
long voidToLong (void *voidValue);
double voidToDouble (void *voidValue);
void *intToVoid (int intValue);
void *longToVoid (long longValue);
void *doubleToVoid (double doubleValue);

unsigned long charToUnLong (char * characterizedNumber);
char *unLongToChar (unsigned long number);

void *unLongToVoid (unsigned long longValue);
unsigned long voidToUnLong (void *voidValue);
unsigned long intToLTA (int value);

char *twistedChar (char *straightChar);

#endif

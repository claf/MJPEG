#ifndef _UTILS_C
#define _UTILS_C

#include "embera.h"

char ** divideString (char *string, char separator) {

	char **strarr;
	int start = 0, i, k = 1, count = 2;

	for (i = 0; string[i] != '\0'; i++) {

		if (string[i] == separator) {
			count++;
		}
	}

	strarr = malloc(count * sizeof(char*));
	i = 0;

	while (*string++ != '\0') {
		if (*string == separator) {

			strarr[i] = malloc(k - start + 2);
			strncpy(strarr[i], string - k + start, k - start + 1);
			strarr[i][k - start + 1] = '\0';
			start = k;
			i++;
		}
		k++;
	}
	strarr[i] = malloc(k - start);
	strncpy(strarr[i], string - k + start, k - start - 1);
	strarr[i][k - start - 1] = '\0';
	strarr[++i] = NULL;

	return strarr;
}

char *strlwr (char *a) {

	char *ret = a;
	while (*a != '\0') {
		//char *_a;
		if (isupper (*a))
			*a = tolower (*a);
			//a = _a;
      		++a;
    	}

	return ret;
}

int compareString (char *srt1, char *str2){

	int flag = -1;
	if (!strcmp(srt1, str2))
		flag = 0;
	return flag;
}

double charToDouble (char * characterizedNumber) {

	double result =0;
	//char *str = (char *)malloc(strlen(characterizedNumber));
	//str = characterizedNumber;
	result = atof(characterizedNumber);
	//free(str);
	return result;
}

long charToInt (char * characterizedNumber) {

	int result = 0;
	//char *str = (char *)malloc(strlen(characterizedNumber));
	//str = characterizedNumber;
	//printf("%s\n", characterizedNumber);
	result = atoi(characterizedNumber);
	//free(str);
	return  result;
}

unsigned long charToUnLong (char * characterizedNumber) {

	unsigned long result = 0;
	//char *str = (char *)malloc(strlen(characterizedNumber));
	//str = characterizedNumber;
	result = atoi(characterizedNumber);
	//free(str);
	return  result;
}

char *unLongToChar (unsigned long number) {

	char *characterizedNumber = (char *)malloc(sizeof(unsigned long));
	sprintf (characterizedNumber, "%ld", number);
	return  characterizedNumber;
}

char *doubleToChar (double number) {

	char *characterizedNumber = (char *)malloc(sizeof(double));
	sprintf (characterizedNumber, "%.12f", number);
	return  characterizedNumber;
}

char *intToChar (int number) {

	char *characterizedNumber = (char *)malloc(sizeof(int));
	sprintf (characterizedNumber, "%d", number);
	return  characterizedNumber;
}

int voidToInt (void *voidValue) {

	int value;
	memcpy(&value, voidValue, sizeof(int));
	return value;
}

long voidToLong (void *voidValue) {

	long value;
	memcpy(&value, voidValue, sizeof(long));
	return value;
}

unsigned long voidToUnLong (void *voidValue) {

	unsigned long value;
	memcpy(&value, voidValue, sizeof(unsigned long));
	return value;
}

double voidToDouble (void *voidValue) {

	double value;
	memcpy(&value, voidValue, sizeof(double));
	return value;
}

unsigned char voidToUnChar (void *voidValue) {

	unsigned char value;
	memcpy(&value, voidValue, sizeof(unsigned char));
	return value;

}

void *intToVoid (int intValue) {

	void *voidValue = (void *)malloc(sizeof(int));
	memcpy(voidValue, &intValue, sizeof(int));
	return voidValue;
}

void *longToVoid (long longValue) {

	void *voidValue = (void *)malloc(sizeof(long));
	memcpy(voidValue, &longValue, sizeof(long));
	return voidValue;
}

void *unLongToVoid (unsigned long longValue) {

	void *voidValue = (void *)malloc(sizeof(unsigned long));
	memcpy(voidValue, &longValue, sizeof(unsigned long));
	return voidValue;
}

void *doubleToVoid (double doubleValue) {

	void *voidValue = (void *)malloc(sizeof(double));
	memcpy(voidValue, &doubleValue, sizeof(double));
	return voidValue;
}

void *unCharToVoid (unsigned char unChar) {

	void *voidValue = (void *)malloc(sizeof(unsigned char));
	memcpy(voidValue, &unChar, sizeof(unsigned char));
	return voidValue;
}


unsigned long intToLTA (int value) {

	unsigned long resultValue = 0;
	resultValue << value;
	return resultValue;
}


/*
void **divideBuffer (void *buffer, int sectionNumber){

	void **sectionedBuffer;
	int i, j, k=0;
	int subBufferSize = (int)(sizeof(*buffer)/sectionNumber);

	for (i=0; i<sectionNumber; i++) {

		for (j=0; j<subBufferSize; j++) {
			sectionedBuffer [i][j] = buffer[k];
			k++;
		}
	}
	return sectionedBuffer;
}
*/

char splitExecName (char *path, char c) {

//	char *execName = (char *)malloc(strlen(path));
	int i = 0, j = 0;
	printf("Original Path 0 = %s \n", path);
//	printf("size of path %d\n", strlen(path));

/*
	for (i=0; i<strlen(path); i++){

		execName[i] = path[(strlen(path)-1)-i];
    }

	char *temp1 = strtok(execName, "/");

	int ltemp1 = strlen(temp1);

	char *temp2 = (char *)malloc(ltemp1);
	memcpy (temp2, temp1, strlen(temp1));

	for (j = 0; j<ltemp1; j++){

		temp1[j] = temp2[(ltemp1-1)-j];
    }
*/
	path = "toto";
	return path;
}

char *twistedChar (char *straightChar) {

        char *twisted = (char *)malloc(strlen(straightChar));
        int i=0;
        for (i=0; i<strlen(straightChar); i++){

                twisted[i] = straightChar[(strlen(straightChar)-1)-i];
        }
#ifdef DEBUG
        printf("Twisted = %s\n", twisted);
#endif
        return twisted;
}

#endif

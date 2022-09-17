/*  Copyright (C) 2007--2009 Tensilica, Inc. */
char *
strdup(s)
     char *s;
{
    char *result = (char*)malloc(strlen(s) + 1);
    if (result == (char*)0)
	return (char*)0;
    strcpy(result, s);
    return result;
}

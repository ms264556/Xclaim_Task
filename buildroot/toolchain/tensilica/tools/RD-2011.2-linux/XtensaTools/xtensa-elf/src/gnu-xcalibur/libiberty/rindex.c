/*  Copyright (C) 2007--2009 Tensilica, Inc. */
/* Stub implementation of (obsolete) rindex(). */

extern char *strrchr ();

char *
rindex (s, c)
  char *s;
  int c;
{
  return strrchr (s, c);
}

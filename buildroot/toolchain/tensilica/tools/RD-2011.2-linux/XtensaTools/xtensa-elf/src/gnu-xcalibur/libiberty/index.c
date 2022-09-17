/*  Copyright (C) 2007--2009 Tensilica, Inc. */
/* Stub implementation of (obsolete) index(). */

extern char * strchr();

char *
index (s, c)
  char *s;
  int c;
{
  return strchr (s, c);
}

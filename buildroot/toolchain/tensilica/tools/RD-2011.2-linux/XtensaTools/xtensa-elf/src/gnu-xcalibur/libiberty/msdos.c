/*  Copyright (C) 2007--2009 Tensilica, Inc. */
char msg[] = "No vfork available - aborting\n";
vfork()
{
  write(1, msg, sizeof(msg));
}

sigsetmask()
{
  /* no signals support in go32 (yet) */
}

waitpid()
{
  return -1;
}

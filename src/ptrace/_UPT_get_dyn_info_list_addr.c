/* libunwind - a platform-independent unwind library
   Copyright (C) 2003-2004 Hewlett-Packard Co
	Contributed by David Mosberger-Tang <davidm@hpl.hp.com>

This file is part of libunwind.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#include "_UPT_internal.h"

#ifdef UNW_TARGET_IA64
# include "elf64.h"
# include "os-linux.h"
#endif

int
_UPT_get_dyn_info_list_addr (unw_addr_space_t as, unw_word_t *dil_addr,
			     void *arg)
{
#if UNW_TARGET_IA64
  unsigned long lo, hi, off;
  struct UPT_info *ui = arg;
  struct map_iterator mi;
  char path[PATH_MAX];
  unw_dyn_info_t *di;
  unw_word_t res;
#endif
  int count = 0;

  Debug (12, "looking for dyn_info list\n");

#if UNW_TARGET_IA64
  maps_init (&mi, ui->pid);
  while (maps_next (&mi, &lo, &hi, &off, path))
    {
      if (off)
	continue;

      if (ui->ei.image)
	{
	  munmap (ui->ei.image, ui->ei.size);
	  /* invalidate the cache: */
	  ui->di_cache.start_ip = ui->di_cache.end_ip = 0;
	}

      if (elf_map_image (&ui->ei, path) < 0)
	return -UNW_ENOINFO;

      Debug (16, "checking object %s\n", path);

      di = _UPTi_find_unwind_table (ui, as, path, lo, off);
      if (di)
	{
	  res = _Uia64_find_dyn_list (as, di, arg);
	  if (res && count++ == 0)
	    {
	      Debug (12, "dyn_info_list_addr = 0x%lx\n", (long) res);
	      *dil_addr = res;
	    }
	}
    }
  maps_close (&mi);
#else
# warning Implement me, please.
#endif

  /* If multiple dynamic-info list addresses are found, we would have
     to determine which was is the one actually in use (since the
     dynamic name resolution algorithm will pick one "winner").
     Perhaps we'd have to track them all until we find one that's
     non-empty.  Hopefully, this case simply will never arise, since
     only libunwind defines the dynamic info list head. */
  assert (count <= 1);

  return (count > 0) ? 0 : -UNW_ENOINFO;
}
/////////////////////////////////////////////////////////////////////////
// $Id: usage.cc 12415 2014-07-10 11:14:49Z vruppert $
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2001-2014  The Bochs Project
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


unsigned char bios[65536];

  int
main(int argc, char *argv[])
{
  int bios_file;
  FILE * org_file;
  unsigned org, last_org, offset;
  int retval;
  unsigned int to_read, index;
  double elements, ratio;

  if (argc !=3 ) {
    fprintf(stderr, "Usage: usage bios-file org-file\n");
    exit(1);
    }

  bios_file = open(argv[1], O_RDONLY);
  org_file = fopen(argv[2], "r");

  if ( (bios_file<0) | (org_file==NULL) ) {
    fprintf(stderr, "problems opening files.\n");
    exit(1);
    }

  printf("files opened OK\n");

  to_read = 65536;
  index   = 0;
  while (to_read > 0) {
    retval = read(bios_file, &bios[index], to_read);
    if (retval <= 0) {
      fprintf(stderr, "problem reading bios file\n");
      exit(1);
      }
    to_read -= retval;
    index   += retval;
    }
  printf("bios file read in OK\n");

  last_org = 0;

  while (1) {
    retval = fscanf(org_file, "0x%x\n", &org);
    if (retval <= 0) break;
    printf("%04x .. %04x ", last_org, org-1);
    for (offset=org-1; offset>last_org; offset--) {
      if (bios[offset] != 0) break;
      }
    if (offset > last_org) {
      elements = (1.0 + double(offset) - double(last_org));
      }
    else {
      if (bios[last_org] == 0)
        elements = 0.0;
      else
        elements = 1.0;
      }

    ratio = elements / (double(org) - double(last_org));
    ratio *= 100.0;
    printf("%6.2lf\n", ratio);
    last_org = org;
    }
}

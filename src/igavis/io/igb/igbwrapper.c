/* wrapper for Header_Read from IGB library, that avoids to pass
 * a file pointer. The file pointer cannot be obtained from Python
 * in a robust way.
 */
#include "header.h"
#include <stdlib.h>
#include <stdio.h>

// Here we pass the filename instead.
int igb_header_read(
    char* fname,
    Header* header,
    Bool_Header* bool_header)
{
  FILE* fh;
  if ((fh=fopen(fname,"r"))==NULL)
    return EXIT_FAILURE;

  if (!Header_Read(fh,header,bool_header))
    return EXIT_FAILURE;

  fclose(fh);
  return EXIT_SUCCESS;
}

int igb_header_write(
    char* fname,
    Header* header,
    Bool_Header* bool_header)
{
  FILE* fh;

  if ((fh=fopen(fname,"wb"))==NULL)
    return EXIT_FAILURE;

  if (!Header_Write(fh,header,bool_header))
    return EXIT_FAILURE;

  fclose(fh);
  return EXIT_SUCCESS;
}


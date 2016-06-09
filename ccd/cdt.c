/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : cdt.c
+----------------------------------------------------------------------------- 
|  Copyright 2002 Texas Instruments Berlin, AG 
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Berlin, AG 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Berlin, AG. 
+----------------------------------------------------------------------------- 
|  Purpose :  Ccddata tool. Delivers informations from ccddata_dll.dll.
+----------------------------------------------------------------------------- 
*/ 

/*==== INCLUDES =============================================================*/
#include "typedefs.h"
#include "ccdtable.h"
#include "ccddata.h"

#include <stdio.h>
#include <string.h>

static char*
#include "ccddata_version.h"
;

/*==== CONSTS ================================================================*//*==== TYPES =================================================================*/
/*==== LOCALS ================================================================*/
static int       ccddata_version;
static int       ccddata_table_version;
static char*     cdt_ccddata_dllname;
/*==== PRIVATE FUNCTIONS =====================================================*/
/*
+------------------------------------------------------------------------------
|  Function     :  parse_cmdline
+------------------------------------------------------------------------------
|  Description  :  Checks options and parameters.
|
|  Parameters   :  Argc and argv from main.
|
|  Return       :  0 if correct cmdline, -1 otherwise.
|                  
+------------------------------------------------------------------------------
*/
static int parse_cmdline (int argc, char* argv[])
{
  int ac = 1;

  if (argc < 2)
    return -1;

  while (ac < argc)
  {
    char* av;
    av = argv[ac];
    switch (av[0])
    {
      case '-':
        if (!strcmp (&av[1], "cdv"))          /* Ccddata version */
        {
          ccddata_version = 1;
        }
        else if (!strcmp (&av[1], "tv"))     /* Table version */
        {
          ccddata_table_version = 1;
        }
        else if (!strcmp (&av[1], "l"))     /* Cccdata dll */
        {
          ++ac;
          cdt_ccddata_dllname = argv[ac];
        }
        else
        {
          fprintf (stderr, "Unknown option: %s\n", av);
          return -1;
        }
        break;
      default:
        return -1;
    }
    ac++;
  }
  return 0;
}


/*
+------------------------------------------------------------------------------
|  Function     :  usage
+------------------------------------------------------------------------------
|  Description  :  Print usage information.
|
|  Parameters   :  tapname - The name of the tap executable.
|
|  Return       :  -
|                  
+------------------------------------------------------------------------------
*/

static void usage (char* cdtname)
{
  fprintf (stderr, "Usage: %s [options]\n", cdtname);
  fprintf (stderr, " Options:\n");
  fprintf (stderr, "  -cdv: print version of ccddata dll\n");
  fprintf (stderr, "  -tv: print version ccddata tables\n");
  fprintf (stderr, "  -l <ccddata-dll>: select dedicated ccddata dll\n");
}

/*==== PUBLIC FUNCTIONS ======================================================*/

/*
+------------------------------------------------------------------------------
|  Function     :  main
+------------------------------------------------------------------------------
|  Description  :  The start into happiness.
|
|  Parameters   :  As usual. See parse_cmdline() and usage() for details. 
|
|  Return       :  0
|                  
+------------------------------------------------------------------------------
*/

int main (int argc, char** argv)
{
  printf ("Ccddata tool %s\n", CCDDATA_VERSION);

  if (parse_cmdline (argc, argv) < 0)
  {
    char *cdtbase;
    cdtbase = strrchr (argv[0], '\\');
    if (!cdtbase)
      cdtbase = strrchr (argv[0], '/');
    usage (cdtbase ? cdtbase+1 : argv[0]);
    return -1;
  }

  if (ccddata_init (cdt_ccddata_dllname, 0, NULL, NULL) != CCDDATA_DLL_OK)
  {
    fprintf (stderr, "Cannot load ccddata dll");
    return -1;
  }

  if (ccddata_version)
    printf ("Version: %s\n", ccddata_get_version());
  if (ccddata_table_version)
    printf ("Table version: %d\n", ccddata_get_table_version());

  ccddata_exit ();

  return 0;
}

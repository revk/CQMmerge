// Merge CQM SVG files

#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <syslog.h>
#include <netdb.h>
#include <stdlib.h>
#include <err.h>
#include <axl.h>

int debug = 0;

int
main (int argc, const char *argv[])
{
   {                            // POPT
      poptContext optCon;       // context for parsing command-line options
      const struct poptOption optionsTable[] = {
         {"debug", 'v', POPT_ARG_NONE, &debug, 0, "Debug"},
         POPT_AUTOHELP {}
      };

      optCon = poptGetContext (NULL, argc, argv, optionsTable, 0);
      poptSetOtherOptionHelp (optCon, "{svgfiles}");

      int c;
      if ((c = poptGetNextOpt (optCon)) < -1)
         errx (1, "%s: %s\n", poptBadOption (optCon, POPT_BADOPTION_NOALIAS), poptStrerror (c));

                               // Load svg files (merge if necessary) and output
      if (!poptPeekArg (optCon))
      {
         poptPrintUsage (optCon, stderr, 0);
         return -1;
      }
      xml_t t = NULL;
      const char *fn;
      while ((fn = poptGetArg (optCon)))
      {
         xml_t n = NULL;
         if (!strcmp (fn, "-"))
            n = xml_tree_read (stdin);
         else
            n = xml_tree_read_file (fn);
         if (!n)
            err (1, "Cannot load %s", fn);
         if (debug)
            fprintf (stderr, "Loaded %s\n", fn);
         if (!t)
         {
            t = n;
            continue;
         }
         // Merge
         xml_t find (xml_t x, const char *tag)
         {
            xml_t findin (xml_t x)
            {
               xml_t e = NULL;
               const char *t;
               while ((e = xml_element_next (x, e)) && (!(t = xml_get (e, "@id")) || strcmp (t, tag)))
               {
                  xml_t s = findin (e);
                  if (s)
                     return s;
               }
               return e;
            }
            return findin (x);
         }
         xml_t a = find (t, "cqm");
         if (a)
         {
            xml_t b = find (n, "cqm");
            if (b)
            {
               const char *tags[] = { "tx", "rx", "fail", "sent", "max", "ave", "min", "hi", "lo" };
               int n;
               for (n = 0; n < sizeof (tags) / sizeof (*tags); n++)
               {
                  xml_t A = find (a, tags[n]);
                  xml_t B = find (b, tags[n]);
                  if (A && B)
                     xml_element_append (A, B);
               }
            }
         }
         xml_tree_delete (n);
      }
      if (t)
      {
#if 0
         if (!defs)
         {                      // Use --defs to only add defs assuming style is in page
            char *style = getstyle ("");
            if (style)
            {                   // User style
               xml_t s = xml_element_add_ns_after (t, NULL, "style", t);        // At start
               xml_element_set_content (s, style);
               free (style);
            }
         }
#endif
#if 0
         if (strchr (format, '+') && !xml_find (t, "defs/marker"))
         {                      // Markers are needed
            void addmarker (char *id, int s)
            {
               xml_t m = xml_add (t, "defs/+marker@id", id);
               xml_add (m, "@markerWidth", "4");
               xml_add (m, "@markerHeight", "4");
               xml_add (m, "@refX", "2");
               xml_add (m, "@refY", "2");
               xml_add (m, "@markerUnits", "userSpaceOnUse");
               m = xml_addf (m, "path@class", "s%d m", s);
               xml_add (m, "@d", "M1,2L3,2M2,1L2,3");
            }
            addmarker ("t", 18);
            addmarker ("r", 21);
         }
#endif
         xml_namespace (t, ":svg", "http://www.w3.org/2000/svg");
         xml_write (stdout, t);
         xml_tree_delete (t);
      }
   }

   return 0;
}

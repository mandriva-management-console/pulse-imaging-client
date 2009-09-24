/*
 * (c) 2003-2007 Linbox FAS, http://linbox.com
 * (c) 2008-2009 Mandriva, http://www.mandriva.com
 *
 * $Id$
 *
 * This file is part of Pulse 2, http://pulse2.mandriva.org
 *
 * Pulse 2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Pulse 2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pulse 2; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

  for (i = 0; i < nb; i++)
    {
      used = 0;

      //debug("- Block %d : O",i+1);
#ifdef BENCH
      sprintf (filename, "/dev/null");
#else
      sprintf (filename, "%s%03d", nameprefix, i);
#endif
      fo = fopen (filename, "wb");

      //debug("H");

      if (remaining > ALLOCLG)
        lg = ALLOCLG;
      else
        lg = remaining;

      sprintf (numline, "L%03d:", i);
      filestring = empty;
      if ((fs = fopen ("options", "rt")))
        {
          while (fgets (line, 400, fs))
            {
              line[strlen (line) - 1] = '|';
              if (strstr (line, numline))
                {
                  filestring = line + 5;
                  break;
                }
            }
          fclose (fs);
        }

      bzero (header.header, HEADERLG);
      sprintf (header.header, "%s%sALLOCTABLELG=%d\n", firststring, filestring, lg);
      //debug("%s",header.header);
      firststring[0] = 0;
      bzero (header.bitmap, ALLOCLG);
      memcpy (header.bitmap, ptr, lg);

      remaining -= lg;
      ptr += lg;

      compress_init (&c, i, bytes, index);
      compress_data (c, (unsigned char *) &header, TOTALLG, fo, 0);

      //debug("D");
      dataptr = buffer;
      datalg = 0;

      for (j = 0; j < lg; j++)
        {
          //debug("%3d\b\b\b",(100*j)/lg);
          if (j % 200 == 0)
            {
#ifndef BENCH
                char tmp[32];
                sprintf(tmp, "%llu", done);
                ui_send("refresh_backup_progress", 1, tmp);
#endif
            }
          for (k = 1; k < 256; k += k)
            {
              if (!(header.bitmap[j] & k))
                skip += 512;
              else
                {
                  if (skip)
                    {
                      if (lseek (fi, skip, SEEK_CUR) == -1) UI_READ_ERROR;
                      c->cbytes += skip;
                    }
                  if (read (fi, dataptr, 512) != 512) UI_READ_ERROR;
                  skip = 0;
                  dataptr += 512;
                  datalg += 512;
                  used++;

                  if (datalg == TOTALLG)
                    {
                      compress_data (c, buffer, TOTALLG, fo, 0);
                      dataptr = buffer;
                      datalg = 0;
                    }
                }
            }
        }

      //debug("F");

      if (datalg > 0)
        compress_data (c, buffer, datalg, fo, 1);

      //debug("C");

      if (skip)
        {
          /* do not check this seek because of trailing 0 at the bitmap's end */
//        fseek (fi, skip, SEEK_CUR);
          lseek (fi, skip, SEEK_CUR);
          c->cbytes += skip;
          skip = 0;
        }
      bytes = compress_end (c, fo);
      fclose (fo);

      //debug(". (used : %ld)\n",used);
    }

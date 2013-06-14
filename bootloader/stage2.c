/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1996  Erich Boleyn  <erich@uruk.org>
 *  Copyright (C) 2000  Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "shared.h"
#include <term.h>
#include "etherboot.h"
#include "deffunc.h"
grub_jmp_buf restart_env;

char pulse2name[32];
extern int nosecurity;
extern int modifietimeout;
char *desc_entries;

static char *
get_entry (char *list, int num, int nested)
{
  int i;

  for (i = 0; i < num; i++)
    {
      do
        {
          while (*(list++));
        }
      while (nested && *(list++));
    }

  return list;
}

#if 0
static void
print_entries (int y, int size, int first, char *menu_entries)
{
  int i;

  gotoxy (60, y + 1);
  grub_putchar (first?DISP_UP:' ');

  menu_entries = get_entry (menu_entries, first, 0);

  for (i = 1; i <= size; i++)
    {
      int j = 0;

      gotoxy (20, y + i);

      while (*menu_entries)
        {
          if (j < 40)
            { grub_putchar (*menu_entries); j++; }
          menu_entries++;
        }

      if (*(menu_entries - 1)) menu_entries++;

      for (; j < 40; j++) grub_putchar (' ');
    }

  gotoxy (60, y + size);
  grub_putchar (*menu_entries?DISP_DOWN:' ');
}

#endif

/* Print an entry in a line of the menu box.  */
static void
print_entry (int y, int highlight, char *entry)
{
  int x;

  if (current_term->setcolorstate)
    current_term->setcolorstate (COLOR_STATE_NORMAL);

  if (highlight && current_term->setcolorstate)
    current_term->setcolorstate (COLOR_STATE_HIGHLIGHT);

  gotoxy (20, y);
  grub_putchar (' ');
  for (x = 20; x < 59; x++)
    {
      if (*entry && x <= 57)
        {
          if (x == 57)
            grub_putchar (DISP_RIGHT);
          else
            grub_putchar (*entry++);
        }
      else
        grub_putchar (' ');
    }
  gotoxy (58, y);

  if (current_term->setcolorstate)
    current_term->setcolorstate (COLOR_STATE_STANDARD);
}


static void show_desc(int entry)
{
  int x;
  char *ptr;

  if (current_term->setcolorstate)
  current_term->setcolorstate (COLOR_STATE_HIGHLIGHT);

  ptr=get_entry(desc_entries, entry, 0);

  gotoxy(9, 21); grub_putchar(' ');
  gotoxy(70, 21); grub_putchar(' ');
  for(x=10;x<69;x++)
   {
     gotoxy(x,21);
     if (ptr[x-10]) {
       grub_printf("%c",ptr[x-10]);

     }
     else {
       x++;
       for(; x<70; x++) grub_putchar(' ');
       break;
     }
   }

  if (grub_strcmp (current_term->name, "graphics") != 0) {
    for(x=9;x<70;x++) {
      gotoxy(x,21);
      set_attrib(highlight_color);
    }
  }

  if (current_term->setcolorstate)
                current_term->setcolorstate (COLOR_STATE_NORMAL);

}

/* Print entries in the menu box.  */
static void
print_entries (int y, int size, int first, int entryno, char *menu_entries)
{
  int i;

  gotoxy (60, y + 1);

  if (first)
    grub_putchar (DISP_UP);
  else
    grub_putchar (' ');

  menu_entries = get_entry (menu_entries, first, 0);

  for (i = 0; i < size; i++)
    {
      print_entry (y + i + 1, entryno == i, menu_entries);
      if (entryno == i)  show_desc(entryno);

      while (*menu_entries)
        menu_entries++;

      if (*(menu_entries - 1))
        menu_entries++;
    }

  gotoxy (60, y + size);

  if (*menu_entries)
    grub_putchar (DISP_DOWN);
  else
    grub_putchar (' ');

  gotoxy (57, y + entryno + 1);
}


static void
print_border (int y, int size)
{
  int i,j,s;

  /* Color the menu. The menu is 75 * 14 characters.  */

  if (grub_strcmp (current_term->name, "graphics") != 0) {
    for (i = 0; i < 14; i++)
      { for (j = 0; j < 44; j++)
        { gotoxy (j + 18, i + y);
        set_attrib (normal_color);
        }
      }
  }

  gotoxy (18, y);
  grub_putchar (DISP_UL);
  for (i = 0; i < 42; i++) grub_putchar (DISP_HORIZ);
  grub_putchar (DISP_UR);

  i = 1;
  while (1)
    {
      gotoxy (18, y + i);

      if (i > size)
        break;

      grub_putchar (DISP_VERT);
      gotoxy (61, y + i);
      grub_putchar (DISP_VERT);
      i++;
    }

  grub_putchar (DISP_LL);
  for (i = 0; i < 42; i++) grub_putchar (DISP_HORIZ);
  grub_putchar (DISP_LR);

  s = strlen(pulse2name);
  if (s && strcmp(pulse2name, "ERROR") != 0) {
    gotoxy((80-s-2)/2, 3);
    grub_printf(" %s ", pulse2name);
    gotoxy (18, y + size + 1);
  }
}

static void
set_line (int y, int attr)
{
  /* check if we are in gfx mode */
  if (grub_strcmp (current_term->name, "graphics") == 0) return;

  int x;
    { for (x = 19; x < 60; x++)
        {
          gotoxy (x, y);
          set_attrib (attr);
        }
    }
  gotoxy (74, y);
}

#if 0
/* Set the attribute of the line Y to normal state.  */
static void
set_line_normal (int y, int entryno, char *menu_entries)
{
  set_line (y, entryno, normal_color, menu_entries);
}

/* Set the attribute of the line Y to highlight state.  */
static void
set_line_highlight (int y, int entryno, char *menu_entries)
{
  set_line (y, entryno, highlight_color, menu_entries);
}
#endif

//"utilisation int 10h function 01h asm.S implicit declaration"
static void
run_menu (char *menu_entries, char *config_entries, int num_entries,
          char *heap, int entryno)
{
  int c, time1, time2 = -1, first_entry = 0;
  char *cur_entry = 0;
  char log[3] = "L1 ";
  struct term_entry *prev_term = NULL;

  /*
   *  Main loop for menu UI.
   */

restart:
  while (entryno > 11)
    {
      first_entry++;
      entryno--;
    }

  /* If the timeout was expired or wasn't set, force to show the menu
     interface. If the terminal is dumb and the timeout is set, hide
     the menu to boot the default entry automatically when the timeout
     is expired.  */
  if (grub_timeout < 0)
    show_menu = 1;
  else if (terminal & TERMINAL_DUMB)
    show_menu = 0;

  /* If SHOW_MENU is false, don't display the menu until ESC is pressed.  */
  if (! show_menu)
    {
      /* Get current time.  */
      while ((time1 = getrtsecs ()) == 0xFF)
        ;

      while (1)
        {
          /* Check if ESC is pressed.  */
//      if (checkkey () != -1 && ASCII_CHAR (getkey ()) == '\e')
        if ( ((console_getkbdstatuskey()&0x0F)==0x0B) )
            {
              grub_timeout = -1;
              show_menu = 1;
              break;
            }

          /* If GRUB_TIMEOUT is expired, boot the default entry.  */
          if (grub_timeout >=0
              && (time1 = getrtsecs ()) != time2
              && time1 != 0xFF)
            {
              if (grub_timeout <= 0)
                {
                  grub_timeout = -1;
                  goto boot_entry;
                }

              time2 = time1;
              grub_timeout--;

              /* Print a message.  */
              if (terminal & TERMINAL_DUMB)
                grub_printf ("\rPress `ESC' to enter the command-line... %d   ",
                             grub_timeout);
              else
                grub_printf ("\rHidden menu mode, boot default in %d sec  ",
                             grub_timeout);
            }
        }
    }

  /* If the terminal is dumb, enter the command-line interface instead.  */
  if (show_menu && (terminal & TERMINAL_DUMB))
    {
      enter_cmdline (heap, 1);
    }

  /* Only display the menu if the user wants to see it. */
  if (show_menu)
    {
      /* Disable the auto fill mode.  */
      auto_fill = 0;

      init_page ();
     
      nocursor ();

      print_border (3, 12);

      grub_printf ("\n\
                    Use the %c and %c keys to select.\n",
                   DISP_UP, DISP_DOWN);

          if (config_entries)
            printf ("\
                    Press enter to boot.\n");
          else
            printf ("\
      Keys : \'b\' to boot, \'e\' to edit, \'c\' for command-line,\n\
             \'o\' for a new line after selected, (\'O\' for before),\n\
             \'d\' to remove, ESC to go back.");

      print_entries (3, 12, first_entry, entryno, menu_entries);
      set_line (4+entryno, highlight_color);
    }

  /* XX using RT clock now, need to initialize value */
  while ((time1 = getrtsecs()) == 0xFF);

  while (1)
    {
      /* Initialize to NULL just in case...  */
      cur_entry = NULL;

      if (grub_timeout >= 0 && (time1 = getrtsecs()) != time2 && time1 != 0xFF)
        {
          if (grub_timeout <= 0)
            {
              grub_timeout = -1;
              break;
            }

          /* else not booting yet! */
          time2  = time1;
          gotoxy (3, 23);
          printf ("Default boot in %d seconds.    ", grub_timeout);
          gotoxy (80, 4 + entryno);
          grub_timeout--;
        }

      /* Check for a keypress, however if TIMEOUT has been expired
         (GRUB_TIMEOUT == -1) relax in GETKEY even if no key has been
         pressed.
         This avoids polling (relevant in the grub-shell and later on
         in grub if interrupt driven I/O is done).  */
      if ((checkkey () != -1) || (grub_timeout == -1))
        {
          c = translate_keycode (getkey ());

          if (grub_timeout >= 0)
            {
              gotoxy (3, 22);
              printf ("                                                                    ");
              grub_timeout = -1;
              fallback_entry = -1;
              gotoxy (74, 4 + entryno);
            }

          if (c == 'r')
            {
              /* reload the menu */
              printf ("\n");
              init_bios_info();
              /* should never return */
            }

          /* We told them above (at least in SUPPORT_SERIAL) to use
             '^' or 'v' so accept these keys.  */
          if (c == 16 || c == '^')
            {
              if (entryno > 0)
                {
                  set_line (4+entryno, normal_color);
                  print_entry (4 + entryno, 0,
                               get_entry (menu_entries,
                                          first_entry + entryno,
                                          0));
                  entryno--;
                  set_line (4+entryno, highlight_color);
                  print_entry (4 + entryno, 1,
                               get_entry (menu_entries,
                                          first_entry + entryno,
                                          0));
                  show_desc(entryno);

                }
              else if (first_entry > 0)
                {
                  first_entry--;
                  print_entries (3, 12, first_entry, entryno,
                                 menu_entries);
                }
            }
          if ((c == 14 || c == 'v') && first_entry + entryno + 1 < num_entries)
            {
              if (entryno < 11)
                {
                  set_line (4+entryno, normal_color);
                  print_entry (4 + entryno, 0,
                               get_entry (menu_entries,
                                          first_entry + entryno,
                                          0));
                  entryno++;
                  set_line (4+entryno, highlight_color);
                  print_entry (4 + entryno, 1,
                               get_entry (menu_entries,
                                          first_entry + entryno,
                                          0));
                  show_desc(entryno);

                }
              else if (num_entries > 12 + first_entry)
                {
                  first_entry++;
                  print_entries (3, 12, first_entry, entryno, menu_entries);
                  show_desc(entryno);
                }
            }

          if (config_entries)
            {
              if ((c == '\n') || (c == '\r'))
                break;
            }
          else
            {
              if (((c == 'd') || (c == 'o') || (c == 'O')) && nosecurity)
                {
                  print_entry (4 + entryno, 0,
                                 get_entry (menu_entries,
                                            first_entry + entryno,
                                            0));

                  /* insert after is almost exactly like insert before */
                  if (c == 'o')
                    {
                      /* But `o' differs from `O', since it may causes
                         the menu screen to scroll up.  */
                      if (entryno < 11)
                        entryno++;
                      else
                        first_entry++;

                      c = 'O';
                    }

                  cur_entry = get_entry (menu_entries,
                                         first_entry + entryno,
                                         0);

                  if (c == 'O')
                    {
                      memmove (cur_entry + 2, cur_entry,
                               ((int) heap) - ((int) cur_entry));

                      cur_entry[0] = ' ';
                      cur_entry[1] = 0;

                      heap += 2;

                      num_entries++;
                    }
                  else if (num_entries > 0)
                    {
                      char *ptr = get_entry(menu_entries,
                                            first_entry + entryno + 1,
                                            0);

                      memmove (cur_entry, ptr, ((int) heap) - ((int) ptr));
                      heap -= (((int) ptr) - ((int) cur_entry));

                      num_entries--;

                      if (entryno >= num_entries)
                        entryno--;
                      if (first_entry && num_entries < 12 + first_entry)
                        first_entry--;
                    }

                  print_entries (3, 12, first_entry, entryno, menu_entries);
                }

              cur_entry = menu_entries;
              if (c == 27)
                return;
              if (c == 'b')
                break;
            }

//TODO : remove "{"
//
            {
              if ((c == 'e') && nosecurity)
                {
                  int new_num_entries = 0, i = 0;
                  char *new_heap;

                  if (config_entries)
                    {
                      new_heap = heap;
                      cur_entry = get_entry (config_entries,
                                             first_entry + entryno,
                                             1);
                    }
                  else
                    {
                      /* safe area! */
                      new_heap = heap + NEW_HEAPSIZE + 1;
                      cur_entry = get_entry (menu_entries,
                                             first_entry + entryno,
                                             0);
                    }

                  do
                    {
                      while ((*(new_heap++) = cur_entry[i++]) != 0);
                      new_num_entries++;
                    }
                  while (config_entries && cur_entry[i]);

                  /* this only needs to be done if config_entries is non-NULL,
                     but it doesn't hurt to do it always */
                  *(new_heap++) = 0;

                  if (config_entries)
                    run_menu (heap, NULL, new_num_entries, new_heap, 0);
                  else
                    {
                      cls ();
                      print_cmdline_message (0);

                      new_heap = heap + NEW_HEAPSIZE + 1;

                      saved_drive = boot_drive;
                      saved_partition = install_partition;
                      current_drive = 0xFF;

                      if (! get_cmdline (PACKAGE " edit> ", new_heap,
                                         NEW_HEAPSIZE + 1, 0, 1))
                        {
                          int j = 0;

                          /* get length of new command */
                          while (new_heap[j++])
                            ;

                          if (j < 2)
                            {
                              j = 2;
                              new_heap[0] = ' ';
                              new_heap[1] = 0;
                            }

                          /* align rest of commands properly */
                          memmove (cur_entry + j, cur_entry + i,
                                   ((int) heap) - (((int) cur_entry) + i));

                          /* copy command to correct area */
                          memmove (cur_entry, new_heap, j);

                          heap += (j - i);
                        }
                    }

                  goto restart;
                }
              if ((c == 'c') && nosecurity)
                {
                  enter_cmdline (heap, 0);
                  goto restart;
                }
            }
        }
    }

  /* Attempt to boot an entry.  */

 boot_entry:
  /* Enable the auto fill mode.  */
  auto_fill = 1;

  /* tell Pulse 2 that we will execute a menu entry */
  udp_init();
  log[2] = (first_entry + entryno) & 255;               /* entry number */
  udp_send_to_pulse2(log, 3);
  udp_close();

  cls ();
  setcursor (1);
  /* if our terminal needed initialization, we should shut it down
   * before booting the kernel, but we want to save what it was so
   * we can come back if needed */
#if 0
  if (current_term->shutdown)
     {
       (*current_term->shutdown)();
       current_term = term_table; /* assumption: console is first */
     }
#endif
  while (1)
    {
      //cls ();

#ifdef DEBUG
      if (config_entries)
        printf ("\n  Executing \'%s\'\n\n",
                get_entry (menu_entries, first_entry + entryno, 0));
      else
        printf ("  Executing command-list\n\n");
#endif //DEBUG
      if (! cur_entry)
        cur_entry = get_entry (config_entries, first_entry + entryno, 1);

      /* Set CURRENT_ENTRYNO for the command "savedefault".  */
      current_entryno = first_entry + entryno;

      if (run_script (cur_entry, heap))
        {
          if (fallback_entry < 0)
            break;
          else
            {
              cur_entry = NULL;
              first_entry = 0;
              entryno = fallback_entry;
              fallback_entry = -1;
            }
        }
      else
        break;
    }

  /* if we get back here, we should go back to what our term was before */
  current_term = prev_term;
  if (current_term->startup)
      /* if our terminal fails to initialize, fall back to console since
       * it should always work */
      if ((*current_term->startup)() == 0)
          current_term = term_table; /* we know that console is first */
  show_menu = 1;
  goto restart;
}


static int
get_line_from_config (char *cmdline, int maxlen)
{
  int pos = 0, literal = 0, comment = 0;
  char c;  /* since we're loading it a byte at a time! */

  while (grub_read (&c, 1))
    {
      /* translate characters first! */
      if (c == '\\' && ! literal)
        {
          literal = 1;
          continue;
        }
      if (c == '\r')
        continue;
      if ((c == '\t') || (literal && (c == '\n')))
        c = ' ';

      literal = 0;

      if (comment)
        {
          if (c == '\n')
            comment = 0;
        }
      else if (! pos)
        {
          if (c == '#')
            comment = 1;
          else if ((c != ' ') && (c != '\n'))
            cmdline[pos++] = c;
        }
      else
        {
          if (c == '\n')
            break;

          if (pos < maxlen)
            cmdline[pos++] = c;
        }
    }

  cmdline[pos] = 0;

  return pos;
}


/* This is the starting function in C.  */
void
cmain (void)
{
  int config_len, menu_len, num_entries, desc_len;
  char *config_entries, *menu_entries;
  char *kill_buf = (char *) KILL_BUF;

  /* Initialize the environment for restarting Stage 2.  */
  grub_setjmp (restart_env);

  /* Initialize the kill buffer.  */
  *kill_buf = 0;

  /* Never return.  */
  for (;;)
    {
      auto_fill = 1;
      config_len = 0;
      menu_len = 0;
      num_entries = 0;
      config_entries = (char *) (mbi.mmap_addr + mbi.mmap_length);
      menu_entries = (char *) MENU_BUF;

      desc_entries = (char *) HISTORY_BUF;
      desc_len=0;

      init_config ();

      cls();

      /* Here load the configuration file.  */

      if (grub_open (config_file))
        {
          /* STATE 0:  Before any title command.
             STATE 1:  In a title command.
             STATE >1: In a entry after a title command.  */
          int state = 0, prev_config_len = 0, prev_menu_len = 0;
          int got_desc = 0;
          char *cmdline;

          cmdline = (char *) CMDLINE_BUF;
          while (get_line_from_config (cmdline, NEW_HEAPSIZE))
            {
              struct builtin *builtin;
              if(strstr(cmdline, "menupwd") != NULL) set_master_password_func(cmdline+8, 0);
	      if(strstr(cmdline, "timeout") != NULL && modifietimeout)
	      {// modifie cmdline pour mettre timeout a 0
		cmdline[7]=0;
		grub_strncat (cmdline," 0",10) ;
	      }
              /* Get the pointer to the builtin structure.  */
              builtin = find_command (cmdline);
              errnum = 0;
              if (! builtin)
                /* Unknown command. Just skip now.  */
                continue;
              if (builtin->flags & BUILTIN_DESC)
              {
               char *ptr;
               ptr=skip_to(1,cmdline);
               while ((desc_entries[desc_len++] = *(ptr++)) != 0)
                    ;
               got_desc=1;
               continue;
              }

              if (builtin->flags & BUILTIN_TITLE)
                {
                  char *ptr;

                  /* the command "title" is specially treated.  */
                  if (state > 1)
                    {
                      /* The next title is found.  */
                      num_entries++;
                      config_entries[config_len++] = 0;
                      prev_menu_len = menu_len;
                      prev_config_len = config_len;
                      if (!got_desc)
                      {
                       const char *ptr="No description... xx";
                       while ((desc_entries[desc_len++] = *(ptr++)) != 0)
                            ;
                       desc_entries[desc_len-3]='0';
                       desc_entries[desc_len-2]='0'+num_entries;
                      }
                    }
                  else
                    {
                      /* The first title is found.  */
                      menu_len = prev_menu_len;
                      config_len = prev_config_len;
                    }

                  /* Reset the state.  */
                  state = 1;
                  got_desc = 0;

                  /* Copy title into menu area.  */
                  ptr = skip_to (1, cmdline);
                  while ((menu_entries[menu_len++] = *(ptr++)) != 0)
                    ;
                }
              else if (! state)
                {
                  /* Run a command found is possible.  */
                  if (builtin->flags & BUILTIN_MENU)
                    {
                      char *arg = skip_to (1, cmdline);
                      (builtin->func) (arg, BUILTIN_MENU);
                      errnum = 0;
                    }
                  else
                    /* Ignored.  */
                    continue;
                }
              else
                {
                  char *ptr = cmdline;

                  state++;
                  /* Copy config file data to config area.  */
                  while ((config_entries[config_len++] = *ptr++) != 0)
                    ;
                }
            }

          if (state > 1)
            {
              /* Finish the last entry.  */
              num_entries++;
              config_entries[config_len++] = 0;
              if (!got_desc)
              {
               const char *ptr="No description... xx";
               while ((desc_entries[desc_len++] = *(ptr++)) != 0)
                    ;
               desc_entries[desc_len-3]='0';
               desc_entries[desc_len-2]='0'+num_entries;
              }
            }
          else
            {
              menu_len = prev_menu_len;
              config_len = prev_config_len;
            }

          menu_entries[menu_len++] = 0;
          config_entries[config_len++] = 0;
          desc_entries[desc_len++]=0;

          grub_memmove (config_entries + config_len, menu_entries, menu_len);
          menu_entries = config_entries + config_len;
          grub_memmove (config_entries + config_len + menu_len, desc_entries, desc_len);
          desc_entries = config_entries + config_len + menu_len;

          grub_close ();
        }

      /* go ahead and make sure the terminal is setup */
      if (current_term->startup)
        (*current_term->startup)();

      if (! num_entries)
        {
          /* If no acceptable config file, goto command-line, starting
             heap from where the config entries would have been stored
             if there were any.  */
          enter_cmdline (config_entries, 1);
        }
      else
        {
          /* Run menu interface.  */
          run_menu (menu_entries, config_entries, num_entries,
                    menu_entries + menu_len, default_entry);
        }
    }
}


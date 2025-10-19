/*
 * lcdtest: LCD monitor test pattern generator
 *
 * Main program
 * $Id: lcdtest.c 44 2010-01-26 06:28:14Z eric $
 * Copyright 2005, 2007, 2010 Eric Smith <eric@brouhaha.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.  Note that permission is
 * not granted to redistribute this program under the terms of any
 * other version of the General Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#define UNUSED __attribute__ ((unused))

#define QMAKESTR(x) #x
#define MAKESTR(x) QMAKESTR(x)

char *progname;


const char help_teaser_message [] =
{
  "press / or ? for help"
};

const char help_message [] =
{
  "commands:\n"
  "  Change foreground color:\n"
  "      r:  red              w:  white\n"
  "      g:  green            k:  black\n"
  "      b:  blue\n"
  "  Change pattern:\n"
  "      s:  solid fill       h:  horizontal lines\n"
  "      c:  crosshatch       v:  vertical lines\n"
  "      d:  dots             x:  diagonal crosshatch\n"
  "      a:  white vertical lines with two pixel pitch\n"
  "  arrow keys: move lines in arrow direction\n"
  "  plus:   increase spacing between lines\n"
  "  minus:  decrease spacing between lines\n"
  "  0..9:   select predefined line spacings\n"
  "  q or Escape: exit program\n"
  "  ? or /:  toggle display of help message\n"
  "\n"
  "lcdtest " MAKESTR(RELEASE) "    Free Software (GPLv3)\n"
  "Copyright 2005-2010 Eric Smith <eric@brouhaha.com>\n"
  "http://www.brouhaha.com/~eric/software/lcdtest/"
};


void help (FILE *f)
{
  fprintf (f, "%s", help_message);
}


void usage (FILE *f)
{
  fprintf (f, "lcdtest " MAKESTR(RELEASE) " Copyright 2005-2007 Eric Smith <eric@brouhaha.com>\n");
  fprintf (f, "http://www.brouhaha.com/~eric/software/lcdtest/");
  fprintf (f, "\n");
  fprintf (f, "usage:  %s [options]\n", progname);
  fprintf (f, "options:\n");
  fprintf (f, "    --help         show usage and command keys\n");
  fprintf (f, "    --window       display in a window rather than full screen\n");
  fprintf (f, "    -x <count>     use a video mode with the specified horizontal resolution\n");
  fprintf (f, "    -y <count>     use a video mode with the specified vertical resolution\n");
  fprintf (f, "    --list-modes   list available video modes (including resolution)\n");
}


void fatal (int ret, char *format, ...)
{
  va_list ap;

  fprintf (stderr, "fatal error: ");
  va_start (ap, format);
  vfprintf (stderr, format, ap);
  va_end (ap);
  if (ret == 1)
    usage (stderr);
  exit (ret);
}


void get_text_size (TTF_Font *font, const char *text, int *width, int *height)
{
  *width = 0;
  *height = 0;

  while (*text)
    {
      const char *p;
      int n;
      int line_width, line_height;
      char buf [200];

      p = strchr (text, '\n');
      if (p)
	{
	  n = p - text;
	  p++;
	}
      else
	{
	  n = strlen (text);
	  p = text + n;
	}

      if (n > (sizeof (buf) - 1))
	n = sizeof (buf) - 1;
      if (n == 0)
	buf [n++] = ' ';
      else
	strncpy (buf, text, n);
      buf [n] = '\0';

      if (TTF_SizeText (font, buf, & line_width, & line_height) != 0)
	fatal (2, "TTF_SizeText failed (missing glyph?)\n");
      *height += line_height;
      if (line_width > *width)
	*width = line_width;

      text = p;
    }
}

SDL_Surface *create_text_surface (const char *message,
				  const char *font_path,
				  int font_size,
				  SDL_Color fg,
				  SDL_Color bg UNUSED)
{
  TTF_Font *font;
  int width, height;
  SDL_Surface *text_surface;
  SDL_Rect dest_rect;
  int line_height;

  if ((! TTF_WasInit ()) && (TTF_Init () != 0))
    fatal (2, "can't initialize SDL_ttf\n");

  font = TTF_OpenFont (font_path, font_size);
  if (! font)
    fatal (2, "can't open font\n");

  line_height = TTF_FontLineSkip (font);

  get_text_size (font, message, & width, & height);
  printf ("width: %d, height: %d\n", width, height);

  text_surface = SDL_CreateRGBSurface (SDL_HWSURFACE,
				       width,
				       height,
				       32,
				       0x000000ff,
				       0x0000ff00,
				       0x00ff0000,
				       0);

  dest_rect.w = width;
  dest_rect.h = height;
  dest_rect.x = 0;
  dest_rect.y = 0;

  while (*message)
    {
      const char *p;
      int n;
      SDL_Surface *line_surface;
      char buf [200];

      p = strchr (message, '\n');
      if (p)
	{
	  n = p - message;
	  p++;
	}
      else
	{
	  n = strlen (message);
	  p = message + n;
	}

      if (n > (sizeof (buf) - 1))
	n = sizeof (buf) - 1;
      if (n == 0)
	buf [n++] = ' ';
      else
	strncpy (buf, message, n);
      buf [n] = '\0';

      if (n)
	{
	  line_surface = TTF_RenderText_Blended (font, buf, fg);
	  if (! line_surface)
	    fatal (2, "can't render text\n");
	}

      SDL_BlitSurface (line_surface,
		       & line_surface->clip_rect,
		       text_surface,
		       & dest_rect);
      SDL_FreeSurface (line_surface);

      dest_rect.y += line_height;

      message = p;
    }

  TTF_CloseFont (font);
  return (text_surface);
}


typedef struct
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} my_color_t;

my_color_t test_colors [] =
{
  { 0xff, 0x00, 0x00 },
  { 0x00, 0xff, 0x00 },
  { 0x00, 0x00, 0xff },
  { 0xff, 0xff, 0xff },
  { 0x80, 0x80, 0x80 },
  { 0x00, 0x00, 0x00 }
};


// For special keys without Unicode codepoints (such as escape and
// the cursor keys), we translate to a codepoint in the Unicode
// Private Use Area (PUA), from 0xe000 to 0xf8ff.
#define PUA_ESCAPE 0xe000
#define PUA_UP     0xe001
#define PUA_DOWN   0xe002
#define PUA_LEFT   0xe003
#define PUA_RIGHT  0xe004


// wait for a keypress and return the corresponding Unicode code point

wchar_t wait_key (void)
{
  SDL_Event event;

  while (1)
    {
      SDL_WaitEvent (& event);
      switch (event.type)
	{
	case SDL_KEYDOWN:
	  switch (event.key.keysym.sym)
	    {
	    case SDLK_ESCAPE:
	      return PUA_ESCAPE;
	    case SDLK_UP:
	      return PUA_UP;
	    case SDLK_DOWN:
	      return PUA_DOWN;
	    case SDLK_LEFT:
	      return PUA_LEFT;
	    case SDLK_RIGHT:
	      return PUA_RIGHT;
	    default:
	      if (event.key.keysym.unicode != 0)
		return event.key.keysym.unicode;
	    }
	  break;
	case SDL_QUIT:
	  exit (0);
	}
    }
}


void lock_screen (SDL_Surface *screen)
{
  if (SDL_MUSTLOCK (screen))
    if (SDL_LockSurface (screen) < 0)
      fatal (2, "can't lock screen\n");
}


void unlock_screen (SDL_Surface *screen)
{
  if (SDL_MUSTLOCK (screen))
    SDL_UnlockSurface (screen);
}


void fill_screen (SDL_Surface *screen, uint32_t color)
{
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = screen->w;
  rect.h = screen->h;
  if (SDL_FillRect (screen, & rect, color) < 0)
    fatal (2, "SDL_FillRect failed; %s\n", SDL_GetError ());
}


void draw_horiz_lines (SDL_Surface *screen,
		       uint32_t color,
		       int pitch,
		       int offset)
{
  SDL_Rect rect;
  rect.x = 0;
  rect.w = screen->w;
  rect.h = 1;
      
  for (rect.y = offset; rect.y < screen->h; rect.y += pitch)
    if (SDL_FillRect (screen, & rect, color) < 0)
      fatal (2, "SDL_FillRect failed; %s\n", SDL_GetError ());
}


void draw_vert_lines (SDL_Surface *screen,
		      uint32_t color,
		      int pitch,
		      int offset)
{
  SDL_Rect rect;
  rect.y = 0;
  rect.h = screen->h;
  rect.w = 1;
      
  for (rect.x = offset; rect.x < screen->w; rect.x += pitch)
    if (SDL_FillRect (screen, & rect, color) < 0)
      fatal (2, "SDL_FillRect failed; %s\n", SDL_GetError ());
}


void draw_diagonal_lines (SDL_Surface *screen,
			  uint32_t color,
			  int h_pitch,
			  int h_offset,
			  int v_pitch UNUSED,
			  int v_offset,
			  int slope)  // -1 or +1 if 45 degree angle
{
  SDL_Rect rect;
  rect.h = 1;
  rect.w = 1;
      
  h_offset -= (v_offset * slope);
  if (h_offset < 0)
    h_offset += h_pitch;
  else if (h_offset >= h_pitch)
    h_offset -= h_pitch;

  for (rect.y = 0; rect.y < screen->h; rect.y++)
    {
      for (rect.x = h_offset % h_pitch; rect.x < screen->w; rect.x += h_pitch)
	if (SDL_FillRect (screen, & rect, color) < 0)
	  fatal (2, "SDL_FillRect failed; %s\n", SDL_GetError ());
      h_offset += slope;
      if (h_offset < 0)
	h_offset += h_pitch;
      else if (h_offset >= h_pitch)
	h_offset -= h_pitch;
    }
}


void draw_dots (SDL_Surface *screen,
		uint32_t color,
		int h_pitch,
		int h_offset,
		int v_pitch,
		int v_offset)
{
  SDL_Rect rect;
  rect.h = 1;
  rect.w = 1;
      
  for (rect.x = h_offset; rect.x < screen->w; rect.x += h_pitch)
    for (rect.y = v_offset; rect.y < screen->h; rect.y += v_pitch)
      if (SDL_FillRect (screen, & rect, color) < 0)
	fatal (2, "SDL_FillRect failed; %s\n", SDL_GetError ());
}


uint32_t black;


void pattern_solid (SDL_Surface *screen, uint32_t color)
{
  lock_screen (screen);
  fill_screen (screen, color);
  unlock_screen (screen);
  SDL_UpdateRect (screen, 0, 0, screen->w, screen->h);
}

void pattern_horiz_lines (SDL_Surface *screen,
			  uint32_t color,
			  int v_pitch,
			  int v_offset)
{
  lock_screen (screen);
  fill_screen (screen, black);
  draw_horiz_lines (screen, color, v_pitch, v_offset);
  unlock_screen (screen);
  SDL_UpdateRect (screen, 0, 0, screen->w, screen->h);
}

void pattern_vert_lines (SDL_Surface *screen,
			 uint32_t color,
			 int h_pitch,
			 int h_offset)
{
  lock_screen (screen);
  fill_screen (screen, black);
  draw_vert_lines (screen, color, h_pitch, h_offset);
  unlock_screen (screen);
  SDL_UpdateRect (screen, 0, 0, screen->w, screen->h);
}

void pattern_crosshatch (SDL_Surface *screen,
			 uint32_t color,
			 int h_pitch,
			 int h_offset,
			 int v_pitch,
			 int v_offset)
{
  lock_screen (screen);
  fill_screen (screen, black);
  draw_horiz_lines (screen, color, v_pitch, v_offset);
  draw_vert_lines (screen, color, h_pitch, h_offset);
  unlock_screen (screen);
  SDL_UpdateRect (screen, 0, 0, screen->w, screen->h);
}

void pattern_diagonal (SDL_Surface *screen,
		       uint32_t color,
		       int h_pitch,
		       int h_offset,
		       int v_pitch,
		       int v_offset)
{
  lock_screen (screen);
  fill_screen (screen, black);
  draw_diagonal_lines (screen, color, h_pitch, h_offset, v_pitch, v_offset, -1);
  draw_diagonal_lines (screen, color, h_pitch, h_offset, v_pitch, v_offset, +1);
  unlock_screen (screen);
  SDL_UpdateRect (screen, 0, 0, screen->w, screen->h);
}

void pattern_dots (SDL_Surface *screen,
		   uint32_t color,
		   int h_pitch,
		   int h_offset,
		   int v_pitch,
		   int v_offset)
{
  lock_screen (screen);
  fill_screen (screen, black);
  draw_dots (screen, color, h_pitch, h_offset, v_pitch, v_offset);
  unlock_screen (screen);
  SDL_UpdateRect (screen, 0, 0, screen->w, screen->h);
}


void show_help (SDL_Surface *screen, SDL_Surface *help_image)
{
  SDL_Rect src_rect, dest_rect;

  src_rect.x = 0;
  src_rect.h = help_image->h;
  src_rect.y = 0;
  src_rect.w = help_image->w;

  dest_rect.x = (screen->w - help_image->w) / 2;
  dest_rect.h = help_image->h;
  dest_rect.y = (screen->h - help_image->h) / 2;
  dest_rect.w = help_image->w;

  lock_screen (screen);
  SDL_BlitSurface (help_image, & src_rect, screen, & dest_rect);
  unlock_screen (screen);
  SDL_UpdateRect (screen, dest_rect.x, dest_rect.y, dest_rect.w, dest_rect.h);
}


typedef enum { solid, vert_lines, horiz_lines, crosshatch, dots, diagonal } pattern_t;


#define set_pitch(x) do \
                       if ((pattern != solid) && ((x) < screen->w) && ((x) < screen->h)) \
                         pitch = (x); \
                     while (0)


SDL_Surface *help_teaser_image;
SDL_Surface *help_image;


void main_loop (SDL_Surface *screen)
{
  pattern_t pattern = crosshatch;
  uint32_t fg_color;
  int pitch = 32;
  int h_offset = 0;
  int v_offset = 0;
  uint16_t key_unicode;
  int help_flag = 1;  // 0 for hidden, 1 for teaser, 2 for full help

  black = SDL_MapRGB (screen->format, 0x00, 0x00, 0x00);

  fg_color = SDL_MapRGB (screen->format, 0xff, 0x00, 0x00);

  SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  while (1)
    {
      if (h_offset >= pitch)
	h_offset %= pitch;
      if (v_offset >= pitch)
	v_offset %= pitch;

      switch (pattern)
	{
	case solid:
	  pattern_solid (screen, fg_color);
	  break;
	case vert_lines:
	  pattern_vert_lines (screen, fg_color, pitch, h_offset);
	  break;
	case horiz_lines:
	  pattern_horiz_lines (screen, fg_color, pitch, v_offset);
	  break;
	case crosshatch:
	  pattern_crosshatch (screen, fg_color, pitch, h_offset, pitch, v_offset);
	  break;
	case diagonal:
	  pattern_diagonal (screen, fg_color, pitch, h_offset, pitch, v_offset);
	  break;
	case dots:
	  pattern_dots (screen, fg_color, pitch, h_offset, pitch, v_offset);
	  break;
	}

      if (help_flag == 1)
	show_help (screen, help_teaser_image);
      else if (help_flag == 2)
	show_help (screen, help_image);

      key_unicode = wait_key ();
      switch (key_unicode)
	{
	case PUA_ESCAPE:
	case L'q':
	case L'Q':
	  exit (0);
	case L'r':
	case L'R':
	  fg_color = SDL_MapRGB (screen->format, 0xff, 0x00, 0x00);
	  help_flag = 0;
	  break;
	case L'g':
	case L'G':
	  fg_color = SDL_MapRGB (screen->format, 0x00, 0xff, 0x00);
	  help_flag = 0;
	  break;
	case L'b':
	case L'B':
	  fg_color = SDL_MapRGB (screen->format, 0x00, 0x00, 0xff);
	  help_flag = 0;
	  break;
	case L'w':
	case L'W':
	  fg_color = SDL_MapRGB (screen->format, 0xff, 0xff, 0xff);
	  help_flag = 0;
	  break;
	case L'k':
	case L'K':
	  fg_color = SDL_MapRGB (screen->format, 0x00, 0x00, 0x00);
	  help_flag = 0;
	  break;
	case L'a':
	case L'A':
	  fg_color = SDL_MapRGB (screen->format, 0xff, 0xff, 0xff);
	  pattern = vert_lines;
	  pitch = 2;
	  h_offset = 0;
	  help_flag = 0;
	  break;
	case L's':
	case L'S':
	  pattern = solid;
	  help_flag = 0;
	  break;
	case L'v':
	case L'V':
	  pattern = vert_lines;
	  help_flag = 0;
	  break;
	case L'h':
	case L'H':
	  pattern = horiz_lines;
	  help_flag = 0;
	  break;
	case L'c':
	case L'C':
	  pattern = crosshatch;
	  help_flag = 0;
	  break;
	case L'x':
	case L'X':
	  pattern = diagonal;
	  help_flag = 0;
	  break;
	case L'd':
	case L'D':
	  pattern = dots;
	  help_flag = 0;
	  break;
	case PUA_UP:
	  v_offset = v_offset - 1;
	  if (v_offset < 0)
	    v_offset = pitch - 1;
	  help_flag = 0;
	  break;
	case PUA_DOWN:
	  v_offset = v_offset + 1;
	  if (v_offset >= pitch)
	    v_offset = 0;
	  help_flag = 0;
	  break;
	case PUA_LEFT:
	  h_offset = h_offset - 1;
	  if (h_offset < 0)
	    h_offset = pitch - 1;
	  help_flag = 0;
	  break;
	case PUA_RIGHT:
	  h_offset = h_offset + 1;
	  if (h_offset >= pitch)
	    h_offset = 0;
	  help_flag = 0;
	  break;
	case L'-':
	  if ((pattern != solid) && (pitch > 2))
	    pitch--;
	  help_flag = 0;
	  break;
	case L'+':
	case L'=':
	  set_pitch (pitch+1);
	  help_flag = 0;
	  break;
	case L'0': set_pitch (  4); help_flag = 0; break;
	case L'1': set_pitch (  8); help_flag = 0; break;
	case L'2': set_pitch ( 16); help_flag = 0; break;
	case L'3': set_pitch ( 32); help_flag = 0; break;
	case L'4': set_pitch ( 64); help_flag = 0; break;
	case L'5': set_pitch ( 96); help_flag = 0; break;
	case L'6': set_pitch (128); help_flag = 0; break;
	case L'7': set_pitch (192); help_flag = 0; break;
	case L'8': set_pitch (256); help_flag = 0; break;
	case L'9': set_pitch (320); help_flag = 0; break;
	case L'/':
	case L'?':
	  if (help_flag == 2)
	    help_flag = 0;
	  else
	    help_flag = 2;
	  break;
	default:
	  break;
	}
    }
}


int enumerate_sdl_modes (int *width, int *height, FILE *out)
{
  SDL_Rect **modes;
  int w = -1;
  int h = -1;
  int m = -1;
  int i;

  modes = SDL_ListModes (NULL, SDL_HWSURFACE | SDL_FULLSCREEN);

  if (! modes)
    fatal (2, "no SDL display modes available\n");

  if (modes == (SDL_Rect **) -1)
    fatal (2, "all SDL resolutions available (on hardware?)\n");

  for (i = 0; modes [i]; i++)
    {
      if (((*width) >= 0) & (modes [i]->w != *width))
	continue;
      if (((*height) >= 0) & (modes [i]->h != *height))
	continue;
      if (out)
	fprintf (out, "  mode %d: %d x %d\n", i, modes [i]->w, modes [i]->h);
      if ((modes [i]->w >= w) && (modes [i]->h >= h))
	{
	  w = modes [i]->w;
	  h = modes [i]->h;
	  m = i;
	}
    }

  if (m < 0)
    fatal (2, "no matching SDL video modes found\n");

  *width = w;
  *height = h;
  return (m);
}


const char font_path [] = "/usr/share/fonts/liberation/LiberationMono-Regular.ttf";

int main (int argc, char *argv [])
{
  SDL_Surface *screen;
  bool fullscreen = true;
  uint32_t sdl_flags;
  int width = -1;
  int height = -1;
  SDL_Color black = { 0x00, 0x00, 0x00, 0x00 };
  SDL_Color white = { 0xff, 0xff, 0xff, 0x00 };

  progname = argv [0];

  if (SDL_Init (SDL_INIT_VIDEO) < 0)
    fatal (2, "SDL initialization error %s\n", SDL_GetError ());
  atexit (SDL_Quit);
  SDL_EnableUNICODE (1);

  help_teaser_image = create_text_surface (help_teaser_message,
					   font_path,
					   18,
					   white,
					   black);
  help_image = create_text_surface (help_message,
				    font_path,
				    18,
				    white,
				    black);

  while (--argc)
    {
      argv++;
      if ((strcmp (argv [0], "--help") == 0) ||
	  (strcmp (argv [0], "-?") == 0))
	{
	  usage (stderr);
	  help (stderr);
	  exit (0);
	}
      else if (strcmp (argv [0], "--list-modes") == 0)
	{
	  fprintf (stderr, "SDL video modes:\n");
	  enumerate_sdl_modes (& width, & height, stderr);
	  exit (0);
	}
      else if ((strcmp (argv [0], "--window") == 0) ||
	       (strcmp (argv [0], "-w") == 0))
	{
	  fullscreen = false;
	  if (width < 0)
	    width = 620;
	  if (height < 0)
	    height = 460;
	}
      else if (strcmp (argv [0], "-x") == 0)
	{
	  if (--argc == 0)
	    fatal (1, "-x option must be followed by resolution\n");
	  width = atoi (argv [1]);
	  argv++;
	}
      else if (strcmp (argv [0], "-y") == 0)
	{
	  if (--argc == 0)
	    fatal (1, "-y option must be followed by resolution\n");
	  height = atoi (argv [1]);
	  argv++;
	}
      else
	fatal (1, "unrecognized option '%s'\n", argv [0]);
    }

  if (fullscreen)
    {
      enumerate_sdl_modes (& width, & height, NULL);
      //printf ("using video resolution %d x %d\n", width, height);
      sdl_flags = SDL_HWSURFACE | SDL_FULLSCREEN;
    }
  else
    sdl_flags = SDL_SWSURFACE;

  screen = SDL_SetVideoMode (width, height, 32, sdl_flags);
  if (! screen)
    fatal (2, "can't set video mode: %s\n", SDL_GetError ());

  if (fullscreen)
    SDL_ShowCursor (SDL_DISABLE);

  main_loop (screen);

  exit (0);
}

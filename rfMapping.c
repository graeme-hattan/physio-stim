/*********************************************************/
/*                                                       */
/* Display flashing spot                                 */
/* to be used as stimulus in conjucntion with the phsyio */
/* reording software                                     */
/*                                                       */
/* written by Matthias Hennig 2003                       */
/* GPL, of course                                        */
/*                                                       */
/*********************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "SDL.h"

/* default parameters */
#define DIAMETER 20
#define REFRESHRATE 30
#define FREQUENCY 0

/* 8 Bit Graphics */
#define NUM_COLORS	256

/* wifth and height of the spot */
int sw, sh;

/* the blinking frequency */
double frequency;

/* the spot */
SDL_Rect spot;

SDL_Surface *screen, *buffer;
SDL_PixelFormat *fmt;

/* back/fore color */
int back, fore;

SDL_Event redrawEvent;

SDL_Surface *CreateScreen(Uint16 w, Uint16 h, Uint8 bpp, Uint32 flags)
{
	SDL_Surface *screen;
	SDL_Color palette[NUM_COLORS];
	Uint8 *buffer;

	/* Set the video mode */
	screen = SDL_SetVideoMode(w, h, bpp, flags);
	if ( screen == NULL ) {
	  fprintf(stderr, "Couldn't set display mode: %s\n", SDL_GetError());
	  return(NULL);
	}
	fprintf(stderr, "Screen is in %s mode\n", (screen->flags & SDL_FULLSCREEN) ? "fullscreen" : "windowed");

	if ( SDL_LockSurface(screen) < 0 ) {
	  fprintf(stderr, "Couldn't lock display surface: %s\n",
		  SDL_GetError());
	  return(NULL);
	}
	fmt = screen->format;

	SDL_FillRect(screen, NULL, SDL_MapRGB(fmt, back,back,back));
	
	SDL_UnlockSurface(screen);
	SDL_UpdateRect(screen, 0, 0, 0, 0);

	return(screen);
}


int FilterEvents(const SDL_Event *event) {
 
    if ( event->type == SDL_MOUSEMOTION ) {
      SDL_LockSurface(screen);
      spot.w = sw;
      spot.h = sh;
      SDL_FillRect(screen, &spot, SDL_MapRGB(fmt,back,back,back));
      spot.x = event->motion.x;
      spot.y = event->motion.y;
      SDL_UnlockSurface(screen);
      /* no update here, emit user event instead */
      SDL_PushEvent(&redrawEvent);
      return(0);  
    }
    return(1);
}

Uint32 refreshTimer(Uint32 interval, void *params)
{
        SDL_PushEvent(&redrawEvent);
	return interval;
}



int main(int argc, char *argv[])
{
	Uint32 videoflags;
	SDL_TimerID refreshTimerID;
	SDL_Event event;
	int width, height, bpp, refresh;
	int done;
	Uint8 *buffp;
	Uint32 startTimer, runTimer;
	int i, j;

	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 ) {
	  fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
	  exit(1);
	}

	sw = DIAMETER;
	sh = DIAMETER;
	width = 640;
	height = 480;
	bpp = 32;
	back = 0;
	fore = 255;
	refresh = REFRESHRATE;
	frequency = FREQUENCY;

	videoflags =  SDL_DOUBLEBUF|SDL_FULLSCREEN;

	redrawEvent.type = SDL_USEREVENT;
	redrawEvent.user.code = 1;
	redrawEvent.user.data1 = NULL;
	redrawEvent.user.data2 = NULL;

	while ( argc > 1 ) {
	  --argc;
	  if ( argv[argc-1] && (strcmp(argv[argc-1], "-w") == 0) ) {
	    width = atoi(argv[argc]);
	    --argc;
	  } else if ( argv[argc-1] && (strcmp(argv[argc-1], "-h") == 0) ) {
	    height = atoi(argv[argc]);
	    --argc;
	  } else if ( argv[argc-1] && (strcmp(argv[argc-1], "-diam") == 0) ) {
	    sw = atoi(argv[argc]);
	    sh = atoi(argv[argc]);
	    --argc;
	  } else if ( argv[argc-1] && (strcmp(argv[argc-1], "-sw") == 0) ) {
	    sw = atoi(argv[argc]);
	    --argc;
	  } else if ( argv[argc-1] && (strcmp(argv[argc-1], "-sh") == 0) ) {
	    sh = atoi(argv[argc]);
	    --argc;
	  } else if ( argv[argc-1] && (strcmp(argv[argc-1], "-freq") == 0) ) {
	    frequency = atof(argv[argc]);
	    --argc;
	  } else if ( argv[argc-1] && (strcmp(argv[argc-1], "-bpp") == 0) ) {
	    bpp = atoi(argv[argc]);
	    --argc;
	  } else if ( argv[argc] && (strcmp(argv[argc], "-sw") == 0) ) {
	    videoflags |= SDL_SWSURFACE;
	  } else if ( argv[argc] && (strcmp(argv[argc], "-i") == 0) ) {
	    back = 255;
	    fore = 0;
	  } else if ( argv[argc] && (strcmp(argv[argc], "-hw") == 0) ) {
	    videoflags |= SDL_HWSURFACE;
	  } else if ( argv[argc] && (strcmp(argv[argc], "-hwpalette") == 0) ) {
	    videoflags |= SDL_HWPALETTE;
	  } else if ( argv[argc] && (strcmp(argv[argc], "-window") == 0) ) {
	    videoflags ^= SDL_FULLSCREEN;
	  } else 
	    {
	      fprintf(stderr, "Usage: %s [-window] [-bpp #] [-w #] [-h #] [-diam #] [-sw #] [-sh #] [-i] [-freq #]\n", argv[0]);
	      exit(1);
	    }
	}

	spot.w = sw;
	spot.h = sh;
	
	/* Set video mode */
	screen = CreateScreen(width, height, bpp, videoflags);
	if ( screen == NULL ) {
		exit(2);
	}

	SDL_SetEventFilter(FilterEvents);
	SDL_ShowCursor(SDL_DISABLE);

	refreshTimerID = SDL_AddTimer((int)(refresh), refreshTimer, (void *)screen);
	runTimer = SDL_GetTicks();

	done = 0;
	while ( !done && SDL_WaitEvent(&event) ) {
	  switch (event.type) {
	  case SDL_KEYDOWN:
	    /* Ignore ALT-TAB */
	    if ( (event.key.keysym.sym == SDLK_LALT) || (event.key.keysym.sym == SDLK_TAB) ) {
	      break;
	    }
	    /* Any key quits */
	  case SDL_QUIT:
	    done = 1;
	    break;
	  case SDL_USEREVENT:
	    runTimer = SDL_GetTicks();
	    SDL_LockSurface(screen);
	    if((int)(runTimer/(500/frequency))%2 == 0) 
	      SDL_FillRect(screen, &spot,  SDL_MapRGB(fmt, fore, fore, fore));
	    else
	      SDL_FillRect(screen, &spot,  SDL_MapRGB(fmt, back, back, back));
	    SDL_UnlockSurface(screen);
	    SDL_UpdateRect(screen, 0, 0, 0, 0);
	    break;
	  default:
	    break;
	  }
	}
	SDL_ShowCursor(SDL_ENABLE);
	SDL_Quit();
	return(0);	
}

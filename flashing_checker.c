/*********************************************************/
/*                                                       */
/* Display flashing checker board                        */
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
#define REFRESHINT 80
#define FREQUENCY 1

/* 8 Bit Graphics */
#define NUM_COLORS	256

/* wifth and height of the spot */
int sqsize;

/* the blinking frequency */
double frequency;

/* the spot */
SDL_Rect grid;

/* back/fore color */
int back, fore;

SDL_Event redrawEvent;

SDL_Surface *CreateScreen(Uint16 w, Uint16 h, Uint8 bpp, Uint32 flags)
{
	SDL_Surface *screen;
	SDL_Color palette[NUM_COLORS];
	Uint8 *buffer1;
	Uint8 *buffer2;
	SDL_PixelFormat *fmt;

	/* Set the video mode */
	screen = SDL_SetVideoMode(w, h, bpp, flags);
	if ( screen == NULL ) {
	  fprintf(stderr, "Couldn't set display mode: %s\n", SDL_GetError());
	  return(NULL);
	}
	fprintf(stderr, "Screen is in %s mode\n", (screen->flags & SDL_FULLSCREEN) ? "fullscreen" : "windowed");
	return(screen);
}



Uint32 refreshTimer(Uint32 interval, void *params)
{
        SDL_PushEvent(&redrawEvent);
	return interval;
}



int main(int argc, char *argv[])
{
	SDL_Surface *screen, *buffer1, *buffer2;
	SDL_PixelFormat *fmt;
	Uint32 videoflags;
	SDL_TimerID refreshTimerID;
	int width, height, bpp, refresh;
	int done;
	Uint8 *buffp;
	Uint32 startTimer, runTimer;
	int i, j;
	SDL_Event event;

	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 ) {
	  fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
	  exit(1);
	}

	redrawEvent.type = SDL_USEREVENT;
	redrawEvent.user.code = 1;
	redrawEvent.user.data1 = NULL;
	redrawEvent.user.data2 = NULL;

	sqsize = DIAMETER;
	width = 640;
	height = 480;
	bpp = 32;
	back = 0;
	fore = 255;
	refresh = REFRESHINT;
	frequency = FREQUENCY;

	videoflags =  SDL_DOUBLEBUF|SDL_FULLSCREEN;

	while ( argc > 1 ) {
	  --argc;
	  if ( argv[argc-1] && (strcmp(argv[argc-1], "-w") == 0) ) {
	    width = atoi(argv[argc]);
	    --argc;
	  } else if ( argv[argc-1] && (strcmp(argv[argc-1], "-h") == 0) ) {
	    height = atoi(argv[argc]);
	    --argc;
	  } else if ( argv[argc-1] && (strcmp(argv[argc-1], "-sqsize") == 0) ) {
	    sqsize = atoi(argv[argc]);
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
	      fprintf(stderr, "Usage: %s [-window] [-bpp #] [-w #] [-h #] [-sqsize #] [-i] [-freq #] [-sw] [-hw] [-hwpalette]\n", argv[0]);
	      exit(1);
	    }
	}

	/* Set video mode */
	screen = CreateScreen(width, height, bpp, videoflags);
	if ( screen == NULL ) {
		exit(2);
	}


	/* prepare stimulus */
	fmt = screen->format;
	bpp = screen->format->BytesPerPixel;
	buffer1 = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_HWACCEL, width, height, bpp, 0,0,0,0);
	buffer1 = SDL_DisplayFormat(buffer1);
	SDL_LockSurface(buffer1);
	buffp = (Uint8 *)buffer1->pixels;
	for ( i=0; i<screen->h/sqsize; i++ ) {
		for ( j=i%2; j<screen->w/sqsize; j=j+2 ) {
			grid.x = j*sqsize;
			grid.y = i*sqsize;
			grid.w = sqsize;
			grid.h = sqsize;
			SDL_FillRect(buffer1, &grid,  SDL_MapRGB(fmt, fore, fore, fore));
			grid.x = j*sqsize+sqsize;
			grid.y = i*sqsize+sqsize;
			grid.w = sqsize;
			grid.h = sqsize;
			SDL_FillRect(buffer1, &grid,  SDL_MapRGB(fmt, back, back, back));
		}
	}
	SDL_UnlockSurface(buffer1);

	buffer2 = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_HWACCEL, width, height, bpp, 0,0,0,0);
	buffer2 = SDL_DisplayFormat(buffer2);
	SDL_LockSurface(buffer2);
	buffp = (Uint8 *)buffer2->pixels;
	for ( i=0; i<screen->h/sqsize; i++ ) {
		for ( j=(i+1)%2; j<screen->w/sqsize; j=j+2 ) {
			grid.x = j*sqsize;
			grid.y = i*sqsize;
			grid.w = sqsize;
			grid.h = sqsize;
			SDL_FillRect(buffer2, &grid,  SDL_MapRGB(fmt, fore, fore, fore));
			grid.x = j*sqsize+sqsize;
			grid.y = i*sqsize+sqsize;
			grid.w = sqsize;
			grid.h = sqsize;
			SDL_FillRect(buffer2, &grid,  SDL_MapRGB(fmt, back, back, back));
		}
	}
	SDL_UnlockSurface(buffer2);

	SDL_ShowCursor(SDL_DISABLE);

	refreshTimerID = SDL_AddTimer((int)(1000/frequency), refreshTimer, (void *)screen);
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
	    if((int)(runTimer/(1000/frequency))%2 == 0) 
	      SDL_BlitSurface(buffer1, NULL, screen, NULL);
	    else {
	      SDL_BlitSurface(buffer2, NULL, screen, NULL);
	    }
	    SDL_Flip(screen);
	    break;
	  default:
	    break;
	  }
	}
	SDL_ShowCursor(SDL_ENABLE);
	SDL_Quit();
	return(0);	
}

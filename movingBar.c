/*********************************************************/
/*                                                       */
/* Display a moving grating                              */
/* to be used as stimulus in conjucntion with the phsyio */
/* reording software                                     */
/*                                                       */
/* written by Bernd Porr 2005                            */
/* based on Matthias Hennigs' movingGrating              */
/* GPL, of course                                        */
/*                                                       */
/*********************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "SDL.h"

/* default parameters */
#define STIMLENGTH 20
#define FREQUENCY 1
#define REFRESHINT 50 // ms
#define ANGLE 0

/* 8 Bit Graphics */
#define NUM_COLORS	256

/* some global variables */
float stimLength = STIMLENGTH;
float frequency = FREQUENCY;
float angle = ANGLE;

SDL_Event redrawEvent;

int cycle;

SDL_Surface *CreateScreen(Uint16 w, Uint16 h, Uint8 bpp, Uint32 flags)
{
	SDL_Surface *screen;
	int i, j, k;
	SDL_Color palette[NUM_COLORS];
	SDL_PixelFormat *fmt;

	/* Set the video mode */
	screen = SDL_SetVideoMode(w, h, bpp, flags);
	if ( screen == NULL ) {
	  fprintf(stderr, "Couldn't set display mode: %s\n",
		  SDL_GetError());
	  return(NULL);
	}
	fprintf(stderr, "Screen is in %s mode\n",
		(screen->flags & SDL_FULLSCREEN) ? "fullscreen" : "windowed");
	
	/* Set a gray colormap */
	for ( i=0; i<NUM_COLORS; ++i ) {
		palette[i].r = (NUM_COLORS-1)-i * (256 / NUM_COLORS);
		palette[i].g = (NUM_COLORS-1)-i * (256 / NUM_COLORS);
		palette[i].b = (NUM_COLORS-1)-i * (256 / NUM_COLORS);
	}
	SDL_SetColors(screen, palette, 0, NUM_COLORS);

	if ( SDL_LockSurface(screen) < 0 ) {
	  fprintf(stderr, "Couldn't lock display surface: %s\n",
		  SDL_GetError());
	  return(NULL);
	}
	
	SDL_UnlockSurface(screen);
	SDL_UpdateRect(screen, 0, 0, 0, 0);

	return(screen);
}

Uint32 refreshTimer(Uint32 interval, void *params)
{
        SDL_PushEvent(&redrawEvent);
	return interval;
}


int main(int argc, char *argv[])
{
	SDL_Surface *screen, *buffer;
	Uint32 videoflags;
	SDL_TimerID refreshTimerID;
	SDL_Event event;
	int width, height, bpp, refresh;
	int done;
	int shift;
	Uint8 *buffp;
	int i, j;
	int t, interval_stat;
	Uint32 startTimer, runTimer;
	float pos;
	int x,y;
	float xt,yt;
	int d;
	int nsteps;
	SDL_PixelFormat *fmt;
	SDL_Rect grid;

	redrawEvent.type = SDL_USEREVENT;
	redrawEvent.user.code = 1;
	redrawEvent.user.data1 = NULL;
	redrawEvent.user.data2 = NULL;

	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		exit(1);
	}

	interval_stat = 0;
	t = 0;
	cycle = 0;

	frequency = FREQUENCY;
	refresh = REFRESHINT;

	width = 200;
	height = 200;
	bpp = 32;

	videoflags =  SDL_DOUBLEBUF |SDL_FULLSCREEN;

	while ( argc > 1 ) {
	  --argc;
	  if ( argv[argc-1] && (strcmp(argv[argc-1], "-w") == 0) ) {
	    width = atoi(argv[argc]);
	    --argc;
	  } else if ( argv[argc-1] && (strcmp(argv[argc-1], "-h") == 0) ) {
	    height = atoi(argv[argc]);
	    --argc;
	  } else if ( argv[argc-1] && (strcmp(argv[argc-1], "-angle") == 0) ) {
	    angle = atof(argv[argc]);
	    --argc;
	  } else if ( argv[argc-1] && (strcmp(argv[argc-1], "-length") == 0) ) {
	    stimLength = atoi(argv[argc]);
	    --argc;
	  } else if ( argv[argc-1] && (strcmp(argv[argc-1], "-freq") == 0) ) {
	    frequency = atof(argv[argc]);
	    --argc;
	  } else if ( argv[argc-1] && (strcmp(argv[argc-1], "-bpp") == 0) ) {
	    bpp = atoi(argv[argc]);
	    --argc;
	  } else if ( argv[argc] && (strcmp(argv[argc], "-sw") == 0) ) {
	    videoflags |= SDL_SWSURFACE;
	  } else if ( argv[argc] && (strcmp(argv[argc], "-hwpalette") == 0) ) {
	    videoflags |= SDL_HWPALETTE;
	  } else if ( argv[argc] && (strcmp(argv[argc], "-window") == 0) ) {
	    videoflags ^= SDL_FULLSCREEN;
	  } else 
	    {
	      fprintf(stderr, "Usage: %s [-window] [-w #] [-h #] [-angle #] [-length #] [-freq #] [-bpp] [-sw] [-hwpalette]\n", argv[0]);
	      exit(1);
	    }
	}
	
	/* Set video mode */
	screen = CreateScreen(width, height, bpp, videoflags);
	if ( screen == NULL ) {
		exit(2);
	}

	angle=angle/180*M_PI;

	/* create a buffer where the stimulus is prepared */
	buffer = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_HWACCEL, width, height, bpp, 0,0,0,0);
	buffer = SDL_DisplayFormat(buffer);

	bpp = screen->format->BytesPerPixel;
	SDL_ShowCursor(SDL_DISABLE);
	refreshTimerID = SDL_AddTimer((int)(refresh), refreshTimer, (void *)screen);
	runTimer = SDL_GetTicks();

	/* calculate the amount of shift per ms */
	nsteps=ceil(1000.0/(((float)refresh)*frequency));
	shift=screen->w/nsteps;
	fprintf(stderr,"f=%f, refresh=%d, shift=%d, nsteps=%d\n",frequency,refresh,shift,nsteps);

	/* MAIN LOOP, do screen refresh and wait for keys */
	done = 0;
	while ( !done && SDL_WaitEvent(&event) ) {
	  switch (event.type) {
	  case SDL_KEYDOWN:
	    /* Ignore ALT-TAB for windows */
	    if ( (event.key.keysym.sym == SDLK_LALT) ||
		 (event.key.keysym.sym == SDLK_TAB) ) {
	      break;
	    }
	    /* Any key quits the application... */
	  case SDL_QUIT:
	    done = 1;
	    break;
	  case SDL_USEREVENT:
	    i = runTimer;
	    runTimer = SDL_GetTicks();
	    startTimer = i;
	    interval_stat +=  runTimer - i;
	    t++;
	    SDL_LockSurface(buffer);
	    buffp = (Uint8 *)buffer->pixels;
	    grid.x = 0;
	    grid.y = 0;
	    grid.w = screen->w;
	    grid.h = screen->h;
	    fmt = screen->format;
	    SDL_FillRect(buffer, &grid,  SDL_MapRGB(fmt, 0, 0, 0));
	    //fprintf(stderr,"t=%d\n",t);
	    d=(t%nsteps)*shift-shift*nsteps/2;
	    buffp = (Uint8 *)buffer->pixels;
	    xt=cos(angle)*d+screen->w/2;
	    yt=sin(angle)*d+screen->h/2;
	    for(pos=-stimLength;pos<=stimLength;pos=pos+1) {
		    x=xt-sin(angle)*pos;
		    y=yt+cos(angle)*pos;
		    if ((x>0)&&(x<(screen->w-1))&&(y>0)&&(y<(screen->h-1))) {
			    buffp[x*bpp+y*screen->w*bpp+0]=NUM_COLORS-1;
			    buffp[x*bpp+y*screen->w*bpp+1]=NUM_COLORS-1;
			    buffp[x*bpp+y*screen->w*bpp+2]=NUM_COLORS-1;
		    }
	    }
	    SDL_UnlockSurface(buffer);
	    SDL_BlitSurface(buffer, NULL, screen, NULL);
	    SDL_Flip(screen);
	    cycle += shift*bpp;
	    break;
	  default:
	    break;
	  }
	}
	SDL_RemoveTimer(refreshTimerID);
	SDL_ShowCursor(SDL_ENABLE);
	printf("mean display interval:%f\n", (float)interval_stat/(float)t);
	SDL_Quit();
	return(0);	
}

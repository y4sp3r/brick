/*
  Basic bricks breaking game
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/time.h>

//Pi constant
#include "pi.h"

//X11 graphics functions
#include "flame.h"

//
#define MAX_STR 129

//
#define MAX_PYRAMID_WALLS 7

//
#define DIST 30

//
typedef unsigned char u8;
typedef int           i32;
typedef unsigned      u32;
typedef float         f32;

//Definition of a rectangle
typedef struct rect_s {

  //Active (1) or inactive (0)
  u8 state;

  //Filled (1) or empty (0)
  u8 fill;
  
  //Rectangle's position
  f32 x;
  f32 y;
  
  //Width
  f32 w;
  
  //Height
  f32 h;
  
  //Color
  u8 red;
  u8 green;
  u8 blue;
  
} rect_t;

//
typedef struct wall_s {

  //Wall's position
  f32 x;
  f32 y;
  
  //Number of rectangles
  u32 nb_rects;
  
  //List of rectangles
  rect_t *l_rects;
  
} wall_t;

//
typedef struct particle_s {
  
  //Previous particle position
  f32 px;
  f32 py;
  
  //Current particle position
  f32 x;
  f32 y;

  //Radius of the particle
  f32 r;
  
  //Velocity of the particle
  f32 vx;
  f32 vy;

  //
  u8 red;
  u8 green;
  u8 blue;
  
} particle_t;

//Current score
u32 score = 0;

//Top target score
u32 top_score = 0;

//Random number generator
static inline int randxy(int a, int b)
{ return (rand() % (b - a + 1)) + a; }

//
void draw_rect(flame_obj_t *fo, rect_t *r)
{
  flame_set_color(fo, r->red, r->green, r->blue);

  if (r->fill)
    {
      for (f32 y = r->y; y < r->y + r->h; y++)
	flame_draw_line(fo, r->x, y, r->x + r->w, y);
    }
  else
    {
      flame_draw_line(fo, r->x, r->y, r->x + r->w, r->y);
      flame_draw_line(fo, r->x, r->y + r->h, r->x + r->w, r->y + r->h);
      
      flame_draw_line(fo, r->x, r->y, r->x, r->y + r->h);
      flame_draw_line(fo, r->x + r->w, r->y, r->x + r->w, r->y + r->h);
    }
}

//
void draw_wall(flame_obj_t *fo, wall_t *w)
{
  //
  for (u32 i = 0; i < w->nb_rects; i++)
    if (w->l_rects[i].state)
      draw_rect(fo, &w->l_rects[i]);
}

//
void init_wall(wall_t *w, u32 n, f32 x, f32 y,
	       f32 rw, f32 rh, u8 rred, u8 rgreen, u8 rblue )
{
  //
  const f32 s = 5.0;
  
  //
  w->x = x;
  w->y = y;
  w->nb_rects = n;
  
  //Allocate array of rectangles/bricks
  w->l_rects = malloc(sizeof(rect_t) * n);
  
  //Initialize rectanbles/bricks
  for (u32 i = 0; i < n; i++)
    {
      //Active rectanle/brick
      w->l_rects[i].state = 1;

      //Fill state
      w->l_rects[i].fill = (i & 1);
      
      //Set the bricks' positions
      w->l_rects[i].x = x + (rw * (f32)(i + 1));
      w->l_rects[i].y = y;

      //Set brick dimensions
      w->l_rects[i].w = rw;
      w->l_rects[i].h = rh;
      
      //Set the brick color
      w->l_rects[i].red   = rred;
      w->l_rects[i].green = rgreen;
      w->l_rects[i].blue  = rblue;
    }
}

//
void release_wall(wall_t *w)
{
  //
  if (w)
    {
      //Clear fields
      w->nb_rects = 0;
      w->x = w->y = 0;

      //Free memory
      if (w->l_rects)
	free(w->l_rects);
    }
}

//
wall_t *init_ipyramid()
{
  //
  wall_t *w = malloc(sizeof(wall_t) * MAX_PYRAMID_WALLS);

  //
  u32 n = 13;
  
  //
  f32 pyr_w = 80;
  f32 pyr_h = 30;

  //
  f32 px = 60;
  f32 py = 30;

  //
  u8 red   = 128;
  u8 green = 100;
  u8 blue  = 103;
  
  //
  for (u32 i = 0; i < MAX_PYRAMID_WALLS; i++)
    {
      //
      top_score += (n - (i << 1));
      
      init_wall(&w[i],
		//Set number of walls
		n - (i << 1),
		
		//Set walls positions
		px + (i * pyr_w), py + (i * pyr_h),

		//Set rectangles' dimensions
		pyr_w, pyr_h,

		//Set walls colors
		red   + (i << 4),
		green + (i << 4),
		blue  + (i << 4));
    }
  
  return w;
}

//
void draw_ipyramid(flame_obj_t *fo, wall_t *w)
{
  for (u32 i = 0; i < MAX_PYRAMID_WALLS; i++)
    draw_wall(fo, &w[i]);
}

//
void release_ipyramid(wall_t **w)
{
  if (*w)
    {
      for (u32 i = 0; i < MAX_PYRAMID_WALLS; i++)
	release_wall(&(*w)[i]);

      free(*w);
    }
}

//
void draw_particle(flame_obj_t *fo, particle_t *p)
{
  //
  flame_set_color(fo, 0, 0, 0);

  for (f32 a = 0.0; a < 2 * PI; a += 0.01)
    flame_draw_point(fo, p->px + p->r * cos(a), p->py + p->r * sin(a));

  //
  flame_set_color(fo, p->red, p->green, p->blue);

  for (f32 a = 0.0; a < 2 * PI; a += 0.01)
    flame_draw_point(fo, p->x + p->r * cos(a), p->y + p->r * sin(a));
}

//
void resolve_particle_rect_collision(particle_t *p, rect_t *r)
{
  if (p->x >= r->x && p->x <= r->x + r->w)
    if (p->y + p->r >= r->y)
      p->vy *= -1;
}

//
void resolve_particle_pyramid_collision(flame_obj_t *fo, particle_t *p, wall_t *w)
{
  //Looping over walls
  for (u32 i = 0; i < MAX_PYRAMID_WALLS; i++)
    {
      //Looping over the wall's rectangles/bricks 
      for (u32 j = 0; j < w[i].nb_rects; j++)
	{
	  //Check if rectangle/brick is active 
	  if (w[i].l_rects[j].state)
	    {
	      //Check collision with the brick
	      if ((p->x >= w[i].l_rects[j].x)                     &&
		  (p->x <= (w[i].l_rects[j].x + w[i].l_rects[j].w)) && 
		  (p->y - p->r) <= (w[i].l_rects[j].y + w[i].l_rects[j].h))
	  	{
		  //Remove rectangle/brick
	  	  w[i].l_rects[j].state = 0;
		  w[i].l_rects[j].red   = 0;
		  w[i].l_rects[j].green = 0;
		  w[i].l_rects[j].blue  = 0;
		  
		  draw_rect(fo, &w[i].l_rects[j]);
		  
		  //Update particle y axis veocity
	  	  p->vy *= -1;

		  //Update score
		  score++;
	  	}
	    }
	}
    }
}

//
void update_particle(particle_t *p)
{
  //Save previous position
  p->px = p->x;
  p->py = p->y;

  //Check collision with left vertical bar
  if (p->x + p->r >= 1200)
    p->vx *= -1;
  
  //Check collision with right vertical bar
  if (p->x - p->r <= 120)
    p->vx *= -1;

  //
  if (p->y - p->r <= 20)
    p->vy *= -1;
      
  //Update position
  p->x += p->vx;
  p->y += p->vy;
}

//This function updates the hitter on the x axis
void update_rect(rect_t *r, f32 d)
{
  if (r->x > 120 && r->x < 1200 - r->w)
    r->x += d;
  else
    if (r->x <= 120)
      r->x += 5;
    else
      if (r->x >= 1200 - r->w)
	r->x -= 5;
}

//
void draw_vh_bars(flame_obj_t *fo)
{
  //Draw horizontal line
  flame_draw_line(fo, 120, 20, 1200, 20);
  
  //Left vertical bar
  flame_draw_line(fo,  120, 20,  120, 1000);

  //Right vertical bar
  flame_draw_line(fo, 1200, 20, 1200, 1000);  
}

//Check if particle is out of bounds 
void check_particle_oob(flame_obj_t *fo, particle_t *p, rect_t *r)
{
  u8 msg[MAX_STR];
  
  if (p->y + p->r > r->y + 1)
    {
      p->red   = 0;
      p->green = 0;
      p->blue  = 0;

      p->vx = p->vy = 0.0;
      
      draw_particle(fo, p);

      sprintf(msg, "GAME OVER!");

      flame_set_color(fo, 255, 0, 0);
      flame_draw_text(fo, 625, 400, msg);
    }
}

//
int main(int argc, char **argv)
{
  char c;
  int click_x, click_y;
  int x_min = 0, x_max = 1280;
  int y_min = 0, y_max = 1024; 
  flame_obj_t *fo = flame_open("Brick", x_max, y_max);

  //
  XEvent event;

  //(a x + b) % m
  srand(getpid());
  
  //A label for the goto statement
  rect_t hitter;
  particle_t ball;
  wall_t *pyr = NULL;

  //
  u8 score_str[MAX_STR];
  
 run:
  
  //Initialize pyramid
  pyr = init_ipyramid();

  //Innitialize ball
  ball.x  = ((f32)x_max / 2.0);
  ball.y  = ((f32)y_max / 2.0);

  ball.r = 10;
  
  ball.vx = 0.02;
  ball.vy = 0.015;
  
  ball.red   = 200;
  ball.green = 200;
  ball.blue  = 200;

  //Initialize the hitter
  hitter.x  = ((f32)x_max / 2.0);
  hitter.y  = 980;

  hitter.fill = 1;
  
  hitter.w = 100;
  hitter.h = 30;
  
  hitter.red   = 128;
  hitter.green = 100;
  hitter.blue  = 70;

  //Wipe the display clear
  flame_clear_display(fo);
  
  //Game loop
  while (1)
    {
      //Handle keyboard input
      if (XPending(fo->display) > 0)
	{
	  XNextEvent(fo->display, &event);
	  
	  if (event.type == KeyPress)
	    {
	      c = XLookupKeysym(&event.xkey, 0);

	      if (c == 'q')
		break;
	      else
		if (c == 'r')
		  {
		    release_ipyramid(&pyr);
		    goto run;
		  }
		else
		  if (c == 81) //Left key
		    {
		      //Remove previous hitter
		      u8 hr, hg, hb;

		      hr = hitter.red;
		      hg = hitter.green;
		      hb = hitter.blue;
		      
		      hitter.red = hitter.green = hitter.blue = 0;
		      draw_rect(fo, &hitter);

		      //
		      update_rect(&hitter, -DIST);

		      //Restore hitter initial colors
		      hitter.red   = hr;
		      hitter.green = hg;
		      hitter.blue  = hb;
		    }
		  else
		    if (c == 83) //Right key
		      {
			//Remove previous hitter
			u8 hr, hg, hb;
			
			hr = hitter.red;
			hg = hitter.green;
			hb = hitter.blue;
		      
			hitter.red = hitter.green = hitter.blue = 0;
			draw_rect(fo, &hitter);

			//
			update_rect(&hitter, DIST);

			//Restore hitter initial colors
			hitter.red   = hr;
			hitter.green = hg;
			hitter.blue  = hb;
		      }
	    }
	}
      
      //Game wizardry goes here

      //Print score
      sprintf(score_str, "Score: %u", score);
      flame_draw_text(fo, 10, 30, score_str);

      //
      sprintf(score_str, "Target score: %u", top_score);
      flame_draw_text(fo, 10, 50, score_str);

      //When done: YOU WON :)
      if (score == top_score)
	{
	  sprintf(score_str, "YOU WON :)");
	  
	  flame_set_color(fo, 100, 128, 100);
	  flame_draw_text(fo, 630, 400, score_str);;

	  ball.vx = 0;
	  ball.vy = 0;

	  ball.red = ball.green = ball.blue = 0;
	  
	  draw_particle(fo, &ball); 
	}
      
      //
      flame_set_color(fo, 255, 255, 255);

      //
      draw_vh_bars(fo);

      //
      draw_rect(fo, &hitter);
	
      //
      draw_ipyramid(fo, pyr);

      //
      draw_particle(fo, &ball);
      resolve_particle_rect_collision(&ball, &hitter);

      resolve_particle_pyramid_collision(fo, &ball, pyr);
      
      update_particle(&ball);

      check_particle_oob(fo, &ball, &hitter);
    }
  
  //
  release_ipyramid(&pyr);
  
  //
  flame_close(fo);
  
  //
  return 0;
}

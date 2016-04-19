/*

	Program: pBlock
	Version: 1.3
	
	Programmer: Zien Lim
	
*/

/* include required library */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <time.h>
#include <graphics.h>
#define fill_time 10
#define swap_time 200
#define move_time 50

/* welcome screen functions */
void ws_core(); /* core of welcome screen */
void ws_title(); /* print the welcome screen title */
void ws_menu(); /* the selection for pBlock */
void play_sound(int tone, int thedelay);
void sound_lib(int which);

/* game core functions */
int level = 0; /* the level variable */
int levelup = 0; /* c whether level up already or not, use for s_standby */
long score = 0; /* the score variable */
int thetime = 200; /* the time variable */
struct block got_move_block; /* store the block that can be move for hint */
int show_hint = 0; /* is hint available to show? */
enum thestate { s_fill_table,    /* fill the table */
				s_check_nomove,  /* check whether got next move or not */
				s_standby,       /* waiting for user click */
				s_choose,        /* choose the part when user click */
				s_scan,          /* scan the block for elimination */
				s_sort,          /* sort the table by column and fill blank box */
				s_gameover,      /* game over */
				s_exit,          /* show exit screen */
				s_exitnow,		 /* exit immediately */
				s_initialize
			  };
int color[8] = {  0, /* black */
				  4, /* red */
				  2, /* green */
				  1, /* blue */
				  14, /* yellow */
				  3, /* cyan */
			      5, /* magenta */
				  7  /* light gray */
				}; /* block color */
struct block{ /* the block row and clolumn structure */
	int row;
	int column;
};
struct block current_click = {0,0}; /* current click */
struct block next_click = {0,0};  /* the next click */
enum thestate game_state; 		 /* the current game status */
int block_table[8][8] = {0}; /* the 8x8 block table array */
int scan_mode = 0;
void gc_core(); /* core of the game */
void gc_initialize(); /* initialize the game screen */
void gc_title(); /* print the pBlock title */
void gc_draw_block(int row, int column, int color); /* draw the block */
void gc_draw_border(int row, int column, int color);
void gc_fill_table(int table[][8]); /* fill the pBlock table */
void gc_check_nomove(int table[][8]); /* check whether got move or not */
void gc_scan_block(int table[][8], int mode); /* find out the block and eliminate */
void gc_sort_block(int table[][8]); /* sort the block table */
void gc_gameover(); /* game over */
void gc_score(int eliminated); /* add the score */
void gc_draw_info(); /* draw left information */
void gc_draw_timebar(); /* draw the time bar */
struct block gc_click_position(int x, int y); /* get the clicked position */
void gc_game_end(); /* the game end screen */

/* mouse functions */
union REGS ireg, oreg;  					/* data type required for mouse functions */
char Reset_Mouse(); 						/* reset the mouse */
void Show_Mouse(); 							/* show the mouse */
void Hide_Mouse();							/* hide the mouse */
int Pressed_Status(int, int*, int*, int*);  /* get the current mouse status */


int main()
{

	int gd = DETECT, gmode, gerrormsg; /* request auto graphic mode detection */

	initgraph(&gd, &gmode, "BGI");  /* initialize graphic */

	if((gerrormsg = graphresult()) != grOk) /* if found graphic initialize error */
	{
		printf("Grapihcs can't initialize! \n");
		printf("Error found: %s\n\n", grapherrormsg(gerrormsg));
		printf("Press any key to halt");
		getch();
		return 1; /* terminate program with error code */
	}
	
	if(Reset_Mouse() == 0) /* reset the mouse */
	{
		printf("Mouse is not prepared !");
		return 1; /* terminate program with error */
	}

	ws_core(); /* load welcome screen code */

	gc_core(); /* load the game core */

	Reset_Mouse(); /* reset the mouse */
	closegraph(); /* close the graphic mode */

	return 0;	
}

/* game core functions - begin */
void gc_core()
{
	int CountP = 0, X, Y;
	int ctime = 0, i;
	char k_input;
	char *leveltext;

	while(1) /* get into infinite loop */
	{
		gc_title(); /* print the title text */

		switch(game_state) /* switch to current game_state */
		{
			/* each state description had written at the top when define the game_state structure */
			case s_initialize:
				gc_initialize(); /* initialize game graphics */
				Show_Mouse(); /* show the mouse */
			case s_fill_table:
				Hide_Mouse();
				if(levelup)
				{
					levelup = 0;
					level++;
					
					sound_lib(4);
													
					setfillstyle(1,0); /* set color to clear the screen */
					setcolor(15); /* white */
					settextstyle(3,0,5);
					
					sprintf(leveltext, "level_%d", level);
					bar(200, 20, 600, 420); /* clear the screen */
					outtextxy(220, 200, leveltext);
					
					delay(700);
				
					bar(200, 20, 600, 420); /* clear the screen */
					setcolor(8); /* set forecolor to dark grey */
					setlinestyle(0,1,0); /* set the line style */
	
					/* draw the table box */
					for(i=1;i<10;i++)
						line(50 * i + 150, 20, 50 * i + 150, 420); /* draw row line */
					for(i=1;i<10;i++)
						line(200, 50 * i - 30, 600, 50 * i - 30); /* draw column line */
				}
				gc_fill_table(block_table); /* fill the block array */
				Show_Mouse();
				break;
			case s_check_nomove:
				gc_check_nomove(block_table); /* check whether got next move */
				break;
			case s_standby:
				if(levelup) /* if the level is up */
				{	
			
					game_state = s_fill_table; /* go to refill the table */
					break;
				}
				show_hint = 1;
				scan_mode = 0;
				Pressed_Status(0, &CountP, &X, &Y); /* get current mouse status */
				if(CountP > 0) /* if clicked */
				{
					Hide_Mouse();
					if( X >= 200 && X <= 600 && Y >= 20 && Y <= 420) /* if in the table range */
					{
						
						sound_lib(1);
						Hide_Mouse();
						current_click = gc_click_position(X, Y); /* get the current click position */
						gc_draw_border(current_click.row, current_click.column, 15); /* draw the border */
						Show_Mouse();
						game_state = s_choose; /* go to choose another block */
					}
					Show_Mouse();
				}
				break;
			case s_choose:
				show_hint = 0;
				Pressed_Status(0, &CountP, &X, &Y);
				if(CountP > 0)
				{
					
					Hide_Mouse();	
					if( X >= 200 && X <= 600 && Y >= 20 && Y <= 420) /* if in the table range */
					{
						next_click = gc_click_position(X, Y); /* get the next click position */

						/* check whether in choose in range */
						if(((next_click.row == (current_click.row + 1)) && (next_click.column == (current_click.column)))||
							((next_click.row == (current_click.row - 1)) && (next_click.column == (current_click.column)))||
							((next_click.column == (current_click.column + 1)) && (next_click.row == (current_click.row)))||
							((next_click.column == (current_click.column - 1)) && (next_click.row == (current_click.row))))
						{
							gc_draw_border(next_click.row, next_click.column, 15); /* draw the border */
							game_state = s_scan; /* go to scan the block */
							sound_lib(1);
						}else{
							/* if not in range, clear it. */
							gc_draw_border(current_click.row, current_click.column, 0); /* draw the border */
							gc_draw_border(next_click.row, next_click.column, 0); /* draw the border */
							game_state = s_standby; /* go to standby */
							sound_lib(3);
						}
					}
					Show_Mouse();
				}
				break;
			case s_scan:
				Hide_Mouse();
				gc_scan_block(block_table, scan_mode); /* scan the block table and eliminate */
				Show_Mouse();
				break;
			case s_sort:
				Hide_Mouse();
				scan_mode = 1;
				gc_sort_block(block_table);
				Show_Mouse();
				break;
			case s_gameover:
				show_hint = 0;
				sound_lib(0);
				Hide_Mouse();
				gc_gameover();
				break;
			case s_exit:
				gc_game_end(); /* load the game end screen */
				break;
		}


		if(kbhit()) /* if the keyboard key is pressed */
		{
			k_input = getch(); /* get the keyboard input */

			if(k_input == 27)	/* ESC */
				game_state = s_exit; /* set game state to exit */
			

			/* debug mode	
			if(k_input == 32) /* get hint for the block *
			{
				if(show_hint)
				{
					gc_draw_border(got_move_block.row, got_move_block.column, WHITE);
					delay(swap_time);
					gc_draw_border(got_move_block.row, got_move_block.column, BLACK);
					delay(fill_time);
					gc_draw_border(got_move_block.row, got_move_block.column, WHITE);
					delay(swap_time);
					gc_draw_border(got_move_block.row, got_move_block.column, BLACK);	
					
					thetime -= 50; /* hint will let time detect 50 lenght! *
				}
				
			}
			*/

		}

		if(game_state == s_exitnow) /* quit the infinite loop and end the game core function */
			break;

		/* time decreasing */
		if(ctime-- <= 0 )
		{
			thetime -= level;
			ctime = 50  / level;
		}
			gc_draw_info(); /* draw the information */
			gc_draw_timebar(); /* draw the time bar */

		delay(50); /* delay for 1/10 seconds */


	}
}

void gc_initialize()
{
	int i;
	char leveltext[8],  scoretext[8];

	srand(time(NULL)); /* randomize seed */

	/* initialize the variable */
	level = 1;
	score = 0;
	thetime = 200;

	/* clear the screen */
	setfillstyle(1,0);
	bar(0,0,640,480);

	setcolor(8); /* set forecolor to dark grey */
	setlinestyle(0,1,0); /* set the line style */
		
	/* draw the table box */
	for(i=1;i<10;i++)
		line(50 * i + 150, 20, 50 * i + 150, 420); /* draw row line */
	for(i=1;i<10;i++)
		line(200, 50 * i - 30, 600, 50 * i - 30); /* draw column line */

	/* draw the timeline box */
	rectangle(200, 440, 600, 455);
	
	setcolor(15); /* set forecolor to white */
	
	/* draw the score title text */
	sprintf(scoretext,"%ld", score);
	settextstyle(2,0,5);
	outtextxy(16,150,"score :");
	outtextxy(80,150,scoretext);
	
	/* draw the level title text */
	sprintf(leveltext, "%d", level);
	outtextxy(20,220,"level :");
	outtextxy(80,220, leveltext);
	
}

void gc_title()
{
	/* print the title pBlock at screen */
	setcolor(rand()%15+1); /* get the random color */
	settextstyle(0,0,3); /* set the text style */
	outtextxy(30,40,"pBlock"); /* print the text */
	
}

void gc_draw_block(int row, int column, int color)
{
	int x, y, border;
	
	border = 5; /* set the border */
	
	setfillstyle(1, color); /* set the fill style */
	
	/* set the position */
	x = 200 + 50 * (column - 1) + border;
	y = 20 + 50 * (row - 1) + border;

	/* draw the box */
	bar(x, y, x + 50 - border * 2, y + 50 - border * 2);
	
	
}

void gc_draw_border(int row, int column, int color)
{
	int x, y, border;
	
	border = 3; /* set the border */
	
	setcolor(color); /* set the color */
	setlinestyle(2,1,0); /* set the line style */
	
	/* get the x, y */
	x = 200 + 50 * (column - 1) + border;
	y = 20 + 50 * (row - 1) + border;
	
	/* draw the border */
	rectangle(x, y, x + 50 - border * 2, y + 50 - border * 2);	
	
}

void gc_fill_table(int table[][8])
{
	
	int r, c;
	
	for(c = 0; c < 8; c++)		/* column */
	{
		for(r = 0; r < 8; r++)	/* row */
		{
			gc_draw_block(r + 1, c + 1 , 15); /* draw white block */
			delay(fill_time); /* delay execution */
			table[r][c] = color[rand() % 7 + 1];  /* put value to table array with random color */
			if(c > 1) /* column part */
			{
				/* make sure it won't drop same block together (3 or more )at first time */
				while((table[r][c] == table[r][c - 1]) &&
						(table[r][c] == table[r][c - 2]))
				{
					table[r][c] = color[rand() % 7 + 1];
				}
			}

			if(r > 1) /* row part */
			{
				/* make sure it won't drop same block together (3 or more) at first time */
				while((table[r][c] == table[r - 1][c]) &&
						(table[r][c] == table[r - 2][c]))
				{
					table[r][c] = color[rand() % 7 + 1];
				}
				
			}

			sound_lib(5);
			gc_draw_block(r + 1, c + 1, table[r][c]); /* draw the block */
		}	
	}
	
	
	game_state = s_check_nomove; /* go to check got next move or not */
	
}

void gc_check_nomove(int table[][8])
{
	int r, c, r2, c2, hold, got_move = 0;
	int temp[8][8];
	
	got_move = 0;

	for(r = 0; r < 8; r++)
	{
		for(c = 0; c < 8; c++)
		{
			hold = table[r][c]; /* hold current block */
			
			/* copy the table to temp */
			for(r2 = 0; r2 < 8; r2++)
				for(c2 = 0; c2 < 8; c2++)
					temp[r2][c2] = table[r2][c2];
				
			temp[r][c] = 0; /* set current block in temp to 0 */			
			
			/* simulate move to up, down, left and right */
			if(r != 0)
				temp[r - 1][c] = hold;
			if(r != 7)
				temp[r + 1][c] = hold;
			if(c != 0)
				temp[r][c - 1] = hold;
			if(c != 7)
				temp[r][c + 1] = hold;
				
			/* scan the temp table to detect elimination available or not */
			for(r2 = 0; r2 < 8; r2++)     /* row */
			{
				for(c2 = 0; c2 < 8; c2++) /* column */
				{
					if(c2 < 6) /* when the colum is smaller than 6 */
					{
						/* if the block right got 2 same with it */
						if((temp[r2][c2] == temp[r2][c2 + 1]) &&
							(temp[r2][c2] == temp[r2][c2 + 2]))
						{
							got_move = 1;
							
							break;
						}
				
					}
			
					if(r2 < 6) /* when the row is smaller than 6 */
					{
						/* if the block down got 2 same with it */
						if((temp[r2][c2] == temp[r2 + 1][c2]) &&
							(temp[r2][c2] == temp[r2 + 2][c2]))
						{
							got_move = 1;
							
							break;
						}
				
					}
				}
				if(got_move)
					break;
			}
			
			if(got_move)
				break;
						
		}	
		
		if(got_move)
			break;
	}
	
	if(got_move) /* check whether got move or not */
		game_state = s_standby; /* go to standby */
	else /* if not */
		game_state = s_fill_table; /* fill table again */
}

void gc_scan_block(int table[][8], int mode)
{
	int r, c;
	int eliminated = 0, colorhold;
	int mask[8][8] = {0};
	struct block current, next, hold;
	
	/* 
		mode = 0, <- get in from user click, will swap
		mode = 1, <- get in from other, will not swap
		
	*/
	
	if(!mode)
	{
		/* get the current click and next click */
		current = current_click;
		next = next_click;
	
		/* bright it up and swap */
		gc_draw_block(current.row, current.column, 15);
		gc_draw_block(next.row, next.column, 15);
		delay(swap_time);
	
		/* swap the block */
		colorhold = table[next.row - 1][next.column - 1]; /* put the next_click element color to colorhold */
		table[next.row - 1][next.column - 1] = table[current.row - 1][current.column - 1]; /* swap better current_click and next_click element */
		table[current.row - 1][current.column - 1] = colorhold; /* put the colorhold to current_click element */
		hold = next; /* hold the next block structure */
		next = current; /* swap the next and current block structure */
		current = hold; /* put the hold to current block structure */
	
		/* draw the block after swap */
		gc_draw_block(current.row, current.column, table[current.row - 1][current.column - 1]);
		gc_draw_block(next.row, next.column, table[next.row - 1][next.column - 1]);
		
	}
	

	for(r = 0; r < 8; r++)     /* row */
	{
		for(c = 0; c < 8; c++) /* column */
		{
			if(c < 6) /* when the colum is smaller than 6 */
			{
				/* if the block right got 2 same with it */
				if((table[r][c] == table[r][c + 1]) &&
					(table[r][c] == table[r][c + 2]))
				{
					mask[r][c] = 1; /* eliminate itself */
					mask[r][c + 1] = 1; /* eliminate the block right to it */
					mask[r][c + 2] = 1; /* eliminate the block right and right to it */
					
					eliminated += 3;
				}
				
			}
			
			if(r < 6) /* when the row is smaller than 6 */
			{
				/* if the block down got 2 same with it */
				if((table[r][c] == table[r + 1][c]) &&
					(table[r][c] == table[r + 2][c]))
				{
					mask[r][c] = 1; /* eliminate itself */
					mask[r + 1][c] = 1; /* eliminate the block down to it */
					mask[r + 2][c] = 1; /* eliminate the block down and down to it */
					
					eliminated += 3;
				}
				
			}
		}
	}
	
	
	if(eliminated != 0) /* if got elimination found */
	{
		/* clear the border */
		gc_draw_border(current_click.row, current_click.column, 0);
		gc_draw_border(next_click.row, next_click.column, 0);


		/* scan the mask and set the block table */
		for(r = 0;r < 8; r++)
		{
			for(c = 0;c < 8; c++)
			{
				if(mask[r][c] == 1) /* if found eliminated block in mask */
				{
					/* bright up and eliminated */
					table[r][c] = 0; /* set the block table element to blank */
					gc_draw_block(r + 1, c + 1, 15); /* bright up */
					delay(fill_time);
					gc_draw_block(r + 1, c + 1, 0); /* black it */
					sound_lib(2);
				}
			}
			
		}
		gc_score(eliminated); /* get the score */
		game_state = s_sort; /* go to sort after eliminated */
	}else{ /* if not */
		if(!mode) /* if in swapping mode, must swap back if not found */
		{
			delay(swap_time);

			sound_lib(7);

			/* recover default if not scan found */
			/* bright it up and swap */
			gc_draw_block(current.row, current.column, 15);
			gc_draw_block(next.row, next.column, 15);

			/* swap back the block */
			colorhold = table[next.row - 1][next.column - 1];
			table[next.row - 1][next.column - 1] = table[current.row - 1][current.column - 1];
			table[current.row - 1][current.column - 1] = colorhold;
			hold = next;
			next = current;
			current = hold;

			delay(swap_time);

			/* recover default if not scan found */
			gc_draw_block(current_click.row, current_click.column, table[current_click.row - 1][current_click.column - 1]);
			gc_draw_block(next_click.row, next_click.column, table[next_click.row - 1][next_click.column - 1]);
			gc_draw_border(current_click.row, current_click.column, 0);
			gc_draw_border(next_click.row, next_click.column, 0);

		}

		game_state = s_check_nomove; /* must scan after sorting no move check */

	}

}

void gc_sort_block(int table[][8])
{
	int c, r, p, s, hold;

	/* column by column */
	for(c = 0; c < 8; c++)
	{
		/* swapping effectively */
		for(p = 0; p < 7; p++)
		{
			for(s = 7; s > 0; s--)
			{
				if(table[s][c] == 0 && table[s - 1][c] != 0) /* if found current element is blank */
				{
					/* swap it */
					hold = table[s][c];
					table[s][c] = table[s - 1][c];
					table[s - 1][c] = hold;

					/* draw the block after swap */
					//gc_draw_block(s + 1, c + 1, 0);
					//gc_draw_block(s, c + 1, 0);
					delay(move_time);
					gc_draw_block(s + 1, c + 1, table[s][c]);
					gc_draw_block(s, c + 1, table[s - 1][c]);
					sound_lib(6);
				}
			}
		}


		/* refill table after swap */
		for(r = 0; r < 8 ; r++)
		{
			if(table[r][c] == 0) /* if found blank */
			{
				/* bright up */
				//gc_draw_block(r + 1, c + 1, 15);
				delay(move_time);

				/* change the element to random color */
				table[r][c] = color[rand() % 7 + 1];

				/* draw the block */
				gc_draw_block(r + 1, c + 1, table[r][c]);
				sound_lib(5);

			}else{ /* if not */
				break; /* break out directly because the column had swapped, no blank below */
			}

		}
	}

	game_state = s_scan; /* check got next move or not */

}

void gc_gameover()
{

	char k_input;
	/* draw game over text */
	setcolor(15);
	settextstyle(2,0,6);
	outtextxy(16, 300, ".. game over ..");
	outtextxy(16, 350, "R - Restart Game");
	outtextxy(16, 400, "ESC - Exit Game");


	/* draw block in table repeatly until key pressed */
	while(1)
	{

		gc_draw_block(rand() % 8 + 1, rand() % 8 + 1, rand() % 15 + 1);

		if(kbhit())
		{

			k_input = getch();

			switch(k_input)
			{
				case 27: /* ESC */
					game_state = s_exit;
					return;
				case 114:
				case 82: /* R, r key */
					game_state = s_initialize;
					return;
			}
		}

	}


}

void gc_score(int eliminated)
{
	score += (eliminated - 2) * level * 10;
	if(!levelup)
		thetime += (eliminated - 2) * 10; /* not level up then add the time */

	gc_draw_info(); /* draw the left information, level and score */
}


void gc_draw_info()
{

	char scoretext[10];
	char leveltext[10];


	/* draw the score title text */
	setcolor(15); /* white */
	sprintf(scoretext,"%ld", score);
	setfillstyle(1,0);
	bar(80,150,150,200);
	settextstyle(2,0,5);
	outtextxy(16,150,"score :");
	outtextxy(80,150,scoretext);

	/* draw the level title text */
	sprintf(leveltext, "%d", level);
	setfillstyle(1,0);
	bar(80, 220, 150, 270);
	settextstyle(2,0,5);
	outtextxy(20,220,"level :");
	outtextxy(80,220, leveltext);
}

void gc_draw_timebar()
{

	/* clear previous bar */
	setfillstyle(1, 0);
	bar(202,442,598,453);
	setfillstyle(1,8);

	if(thetime >= 400)
	{
		//level++;
		levelup = 1; /* level is up! */
		thetime = 200;
		bar(202, 442, 598, 453);
		game_state = s_sort; /* go to sort table */
		return;
	}

	if(thetime <= 0) /* if the time smaller than 0 */
	{
		bar(202,442, 202, 453);
		thetime = 200;
		game_state = s_gameover;
		return;
	}

	bar(202, 442, 202 + thetime, 453);
}


struct block gc_click_position(int x, int y)
{
	struct block theposition;

	/* if click in the table */
	if( x >= 200 && x <= 600 && y >= 20 && y <= 420)
	{
		theposition.row = (y + 30) / 50; /* set the row */
		theposition.column = (x - 150) / 50; /* set the column */
	}
	return theposition; /* return the position */

}

void gc_game_end()
{



	Hide_Mouse(); /* hide the mouse */

	/* clear the screen */
	setfillstyle(1, 0);
	bar(0,0,640,480);

	setcolor(15); /* white */

	/* print the end screen text */
	settextstyle(2,0,5); /* set the text style */
	outtextxy(170,170,"Thank you for playing this game... :)"); /* print the text */
	settextstyle(5,0,1); /* set the text style */
	outtextxy(170,250, "Deveoped by Lim Zheng"); /* print the text */
	outtextxy(170,300, "limzheng@msn.com"); /* print the text */

	sound_lib(9);

	getch(); /* waiting for input any key */

	game_state = s_exitnow; /* set the game state to exit now */


}
/* game core functions - end */



/* welcome screen functions - begin */
void play_sound(int tone, int thedelay)
{

	sound(tone);
	delay(thedelay);
	nosound();

}

void sound_lib(int which)
{

	int i;
 switch(which)
 {


	case 0:
		for(i = 200; i < 5000; i += 100)
			play_sound(i,10);
		break;
	case 1:
		for(i = 200; i < 5000; i += 1000)
		play_sound(i,10);
		break;
	case 2:
		for(i = 200; i < 5000; i += 500)
		play_sound(i,10);
		break;
	case 3:
		for(i = 200; i < 5000; i += 100)
		play_sound(i,1);
		break;
	case 4:
		play_sound(1000, 200);
		play_sound(2000, 100);
		break;
	case 5:
		play_sound(4000, 10);
		break;
	case 6:
		play_sound(3000, 10);
		break;
	case 7:
		play_sound(100,100);
		break;
	case 8:
		for(i = 200; i < 1000; i += 100)
			play_sound(i,100);
		break;
	case 9:
		for(i = 1000; i > 200; i -= 100)
			play_sound(i,100);
		break;


 }
}
void ws_core()
{

	char k_input; /* use to store the keyboard input key */

	srand(time(NULL)); /* randomize the seed */



	ws_menu(); /* print the menu */

	sound_lib(8);

	while(1) /* get into infinite loop */
	{
		ws_title();	 /* print the title text */

		if(kbhit()) /* when the keyboard key is pressed */
		{
			k_input = getch(); /* store the input key */
			if(k_input == 27) /* ESC */
			{
				game_state = s_exit; /* set to exit state */
				break;
			}else{ /* if other than ESC key */
				game_state = s_initialize; /* set to fill table state */
				break;
			}
		}

	}

}

void ws_title()
{

	/* print the title with random color */
	setcolor(rand()%15+1); /* get random color */
	settextstyle(0,0,10); /* set the text stlye */
	outtextxy(100,200, "pBlock"); /* print the text to screen */
	
}

void ws_menu()
{
	/* in this version currently only provide one game mode */
	/* so just press any key to start game */

	/* print the instruction */
	setcolor(15); /* white */
	settextstyle(2,0,4); /* set the text style */
	outtextxy(245,295,"..press any key to start game .."); /* print the text to screen */

}
/* welcome screen functions - end */



/* Mouse Functions - begin*/
char Reset_Mouse()
{
  ireg.x.ax = 0;
  int86(0x33, &ireg, &oreg);

  if (oreg.x.ax == 0)
	  return 0;
  else
	  return 1;
}

void Show_Mouse()
{
	ireg.x.ax= 1;
	int86(0x33, &ireg, &ireg);
}

void Hide_Mouse()
{
	ireg.x.ax= 2;
	int86(0x33, &ireg, &ireg);
}


int Pressed_Status(int button, int *counter, int *x, int *y)
{
	ireg.x.ax = 5;
	ireg.x.bx = button;
	int86(0x33, &ireg, &oreg);

	*counter = oreg.x.bx;
	*x = oreg.x.cx;
	*y = oreg.x.dx;
	return(oreg.x.ax);
}
/* Mouse Functions - end */
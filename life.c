#include<stdio.h>
#include<stdlib.h>
#include<ncurses.h>
#include<unistd.h>
#include<time.h>

#define MAX 40

typedef char * string;

void run(int numbersOn);
void print(int buf[][MAX], int numbersOn);
void init_screen();
void kill_screen();
void calcNext(int arr1[][MAX], int arr2[][MAX]);
int a(int i);
int s(int i);

int main(int args, string argv[])
{
	int numbersOn = 0;

	if(args > 1)
	{
		if(strcmp("-n", argv[1]) == 0)
		{
			numbersOn = 1;
		}
	}
	init_screen();
	run(numbersOn);
	kill_screen();
	return(0);
}

void run(int numbersOn)
{
	int arr1[MAX][MAX];
	int arr2[MAX][MAX];
	int neighbors[8];

	srand(time(NULL));

	int ranx = rand() % (MAX - 4) + 4;
	int rany = rand() % (MAX - 4) + 4;

	int i;
	int j;
	for(i = 0; i < MAX; i++)
	{
		for(j = 0; j < MAX; j++)
		{
			arr1[i][j] = arr2[i][j] = 0;
		}
	}

	//Initialize the startingpoints
	for(i = MAX * MAX / 2; i > 0; i--)
	{
		arr1[rany][ranx] = 1;
		ranx = rand() % (MAX - 4) + 4;
		rany = rand() % (MAX - 4) + 4;
		
	}
	print(arr1, numbersOn);


//321 
//4x0
//567
	int gameLife;
	for(gameLife = 0; gameLife <1000; gameLife++)
	{
		//Copy old array over to the storage array.
		for(i = 0; i < MAX; i++)
		{
			for(j = 0; j < MAX; j++)
			{
				arr2[i][j] = arr1[i][j];
			}
		}
		for(i = 0; i < MAX; i++)
		{
			for(j = 0; j < MAX; j++)
			{
				neighbors[0] = arr2[i][a(j+1)];
				neighbors[1] = arr2[s(i-1)][a(j+1)];
				neighbors[2] = arr2[s(i-1)][j];
				neighbors[3] = arr2[s(i-1)][s(j-1)];
				neighbors[4] = arr2[i][a(j-1)];
				neighbors[5] = arr2[a(i+1)][s(j-1)];
				neighbors[6] = arr2[a(i+1)][j];
				neighbors[7] = arr2[a(i+1)][a(j+1)];

				int k;
				int count = 0;
				for(k = 0; k < 8; k++)
				{
					if(neighbors[k] == 1)
					{
						count++;
					}
				}
				
				if(arr1[i][j] == 1 && count < 2)
				{//rule 1
					arr1[i][j] = 0;
				}
				else if(arr1[i][j] == 1 && count <= 3 && count >= 2)
				{//rule 2
					arr1[i][j] = 1;
				}
				else if(arr1[i][j] == 1 && count > 3)
				{//rule 3
					arr1[i][j] = 0;
				}
				else if(arr1[i][j] == 0 && count == 3)
				{//rule 4
					arr1[i][j] = 1;
				}
			}
		}

		print(arr1, numbersOn);
	}
}

//Quickfunction to test edge
int a(int i)
{
	if(i > MAX - 1)
	{
		return(0);
	}
	return(i);
}

int s(int i)
{
	if(i < 0)
	{
		return(MAX - 1);
	}
	return(i);
}

void print(int buf[][MAX], int numbersOn)
{
	clear();
	int i;
	int j;

	for(i = 0; i < MAX; i++)
	{
		for(j = 0; j < MAX; j++)
		{
			if(buf[i][j] == 1)
			{
				attron(COLOR_PAIR(1));
				if(numbersOn)
				{
					printw("%d", buf[i][j]);
				}
				else
				{
					printw(" ");
				}
				attroff(COLOR_PAIR(1));
			}
			else
			{
				if(numbersOn)
				{
					printw("%d", buf[i][j]);
				}
				else
				{
					printw(" ");
				}
			}
			printw(" ");
		}
		printw("\n");
	}
	refresh();
	sleep(1);
}

void init_screen()
{
	initscr();
	start_color();
	cbreak();
	noecho();

	init_pair(1, COLOR_BLACK, COLOR_GREEN);
}

void kill_screen()
{
	getchar();
	endwin();
}

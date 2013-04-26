#include<stdio.h>
#include<stdlib.h>
#include<ncurses.h>
#include<pthread.h>
#include<unistd.h>
#include<time.h>

#define MAXPRESETS 1

//Makes strings easier to understand
typedef char * string;

//data handler for the threads
typedef struct _colony
{
	int **arr1;
	int **arr2;
	int start;
	int end;
} colony;

//Settings
int row, col, numbersOn, MAXTHREADS, wait, generations;
int presetDesigns[MAXPRESETS];

void runLife();
void init_screen();
void kill_screen();
int ai(int i);
int si(int i);
int aj(int i);
int sj(int i);

//Thread function
void *calcNextGen(void *th);
void *print(void *buf);

int main(int args, string argv[])
{
	int i = 0;
	row = col = numbersOn = 0;
	MAXTHREADS = 2;
	wait = 200000;
	generations = 1000;

	for(i = 0; i < MAXPRESETS; i++)
	{
		presetDesigns[i] = 0;
	}

	if(args > 1)
	{
		for(i = 1; i < args; i++)
		{
			//Tells the print function to display the cell values
			if(strcmp("-n", argv[i]) == 0)
			{
				numbersOn = 1;
			}

			//Read number of threads from the command line.
			if(strcmp("--threads", argv[i]) == 0)
			{
				MAXTHREADS = atoi((char *)argv[++i]);
				continue;
			}

			//Read the time in milliseconds refresh rate from the command line.
			if(strcmp("-t", argv[i]) == 0)
			{
				wait = atoi((char *)argv[++i]) * 1000;
				continue;
			}

			//Read the number of iterations from the command line.
			if(strcmp("-i", argv[i]) == 0)
			{
				generations = atoi((char *)argv[++i]);
				continue;
			}
			
			//Tells the program to preload the glider.
			if(strcmp("-g", argv[i]) == 0)
			{
				presetDesigns[0] = 1;
			}

			if(strcmp("-h", argv[i]) == 0)
			{
				printf("----Help----\n");
				printf("Flags available to use:\n");
				printf("--threads [threads]   Set number of threads to process the buffer with (default 2)\n");
				printf("-t [refresh time]     Set the minimum refresh time in milliseconds (default 200ms)\n");
				printf("-g                    Set the preloaded seed for the glider\n");
				printf("-n                    Display the underlying array\n");
				printf("-i [iterations]       Set the max number of generations to iterate (default 1000)\n");

				exit(0);
			}
		}
	}
	init_screen();
	runLife();
	kill_screen();
	return(0);
}

//This is the function that initializes everything and then loops through the generations.
void runLife()
{
	int i, j;
	i = j = 0;

	//Get the screen size
	getmaxyx(stdscr, row, col);

	//Cut off the edge to make things run infinitely.
	row -= 2;
	col--;
	printw("Row: %d, Col: %d", row, col);
	refresh();
	sleep(2);

	//Makes sure that the number of threads are less than the amount of data to actually be able to be processed.
	if(MAXTHREADS > row / 2)
	{
		MAXTHREADS = row / 2;
	}
	else if(MAXTHREADS < 1)
	{
		MAXTHREADS = 1;
	}

	if(wait < 1000)
	{
		wait = 1000;
	}

	//Initiate rows
	int **arr1 = (int **)malloc(sizeof(int *) * row);	//Create 2 arrays of arrays that represent rows
	int **arr2 = (int **)malloc(sizeof(int *) * row);
	int **scrnBuf = (int **)malloc(sizeof(int *) * row);

	//Initiate columns
	for(i = 0; i < row; i++)
	{
		//Create an empty array of arrays
		arr1[i] = (int *)malloc(sizeof(int) * col);		//For the new generation
		arr2[i] = (int *)malloc(sizeof(int) * col);		//For the old generation
		scrnBuf[i] = (int *)malloc(sizeof(int) * col);	//For the generation to display on the screen.
	}

	//default 2 processor threads + display thread but the user can change it. Thread 0 is display
	MAXTHREADS++;
	colony *colonies;
	colonies = (colony *)malloc(sizeof(colony) * MAXTHREADS);	//Adds an extra thread for the screen buffer.
	pthread_t *thread;
	thread = (pthread_t *)malloc(sizeof(pthread_t) * MAXTHREADS);

	//Initialize the array to 0
	for(i = 0; i < row; i++)
	{
		for(j = 0; j < col; j++)
		{
			arr1[i][j] = arr2[i][j] = scrnBuf[i][j] = 0;
		}
	}

	if(presetDesigns[0])
	{
		//This is for making a glider that runs from {0,0} to {row,row}
		arr1[0][1] = 1;
		arr1[1][2] = 1;
		arr1[2][0] = 1;
		arr1[2][1] = 1;
		arr1[2][2] = 1;
	}
	else
	{
		//Randomly design a seed for the game
		srand(time(NULL));
		int ranx = rand() % (col);
		int rany = rand() % (row);
		for(i = row * col / 2; i > 0; i--)
		{
			arr1[rany][ranx] = 1;
			ranx = rand() % (col);
			rany = rand() % (row);	
		}
	}

	//Split up the data into multiple threads.
	int split = row / (MAXTHREADS - 1);
	for(i = 1; i < MAXTHREADS; i++)
	{
		colonies[i].arr1 = arr1;
		colonies[i].arr2 = arr2;
		colonies[i].start = (i - 1) * split;
		colonies[i].end = (i) * split;
	}

	//The main game loop
	int gameLife;
	for(gameLife = 0; gameLife < generations; gameLife++)
	{
		//Copy new array over to the storage array and the screen buffer array.
		for(i = 0; i < row; i++)
		{
			for(j = 0; j < col; j++)
			{
				arr2[i][j] = scrnBuf[i][j] = arr1[i][j];
			}
		}
	
		//Start the display thread.
		pthread_create(&thread[0], NULL, print, scrnBuf);

		//Start the 2 computing threads.
		for(i = 1; i < MAXTHREADS; i++)
		{
			pthread_create(&thread[i], NULL, calcNextGen, &colonies[i]);
		}

		//Close all of the threads.
		for(i = 0; i < MAXTHREADS; i++)
		{
			pthread_join(thread[i], NULL);
		}
	}
}

/**
 * This function is passed into the thread to run along with the struct of data to manipulate.
 */
void *calcNextGen(void *th)
{
	//Dereference the struct that is passed in
	colony* data = (colony*)th;
	int i, j, k, count;
	
	int neighbors[8];		//A temporary map for the surrounding cells of a 'pixel'
	
	//Iterate through all of the cells that the thread is to handle
	for(i = data->start; i < data->end; i++)
	{
		for(j = 0; j < col; j++)
		{
			//Placement of the neighbors is as followed.
			// 3 2 1 
			// 4 x 0
			// 5 6 7
			neighbors[0] = data->arr2[i][aj(j+1)];
			neighbors[1] = data->arr2[si(i-1)][aj(j+1)];
			neighbors[2] = data->arr2[si(i-1)][j];
			neighbors[3] = data->arr2[si(i-1)][sj(j-1)];
			neighbors[4] = data->arr2[i][aj(j-1)];
			neighbors[5] = data->arr2[ai(i+1)][sj(j-1)];
			neighbors[6] = data->arr2[ai(i+1)][j];
			neighbors[7] = data->arr2[ai(i+1)][aj(j+1)];

			//Count the living cells around the cell.
			count = 0;
			for(k = 0; k < 8; k++)
			{
				if(neighbors[k] == 1)
				{
					count++;
				}
			}
			
			//Applies the rules to the cell
			if(data->arr1[i][j] == 1 && count < 2)
			{//rule 1: x<2 dies of starvation
				data->arr1[i][j] = 0;
			}
			else if(data->arr1[i][j] == 1 && count <= 3 && count >= 2)
			{//rule 2: 2<=x<=3 stays alive.
				data->arr1[i][j] = 1;
			}
			else if(data->arr1[i][j] == 1 && count > 3)
			{//rule 3: x>3 dies of overcrowding.
				data->arr1[i][j] = 0;
			}
			else if(data->arr1[i][j] == 0 && count == 3)
			{//rule 4: If dead and there is exactly 3 living neighbors, comes to life as if reproduction.
				data->arr1[i][j] = 1;
			}
		}
	}
	usleep(wait);
}

//Handles calculating the row's bottom and top edges
//Quick function to test Max edges
int ai(int i)
{
	if(i > row - 1)
	{
		return(0);
	}
	return(i);
}

//Quick function to test the 0 edges
int si(int i)
{
	if(i < 0)
	{
		return(row - 1);
	}
	return(i);
}

//Handles calculating the column's left and right edges
//Quick function to test Max edges
int aj(int i)
{
	if(i > col - 1)
	{
		return(0);
	}
	return(i);
}

//Quick function to test the 0 edges
int sj(int i)
{
	if(i < 0)
	{
		return(col - 1);
	}
	return(i);
}

//Prints out everything in the array
void *print(void *inBuf)
{
	//natgeo *buf = (natgeo*)inBuf;
	int **buf = (int **)inBuf;
	clear();
	int i;
	int j;

	for(i = 0; i < row; i++)
	{
		for(j = 0; j < col; j++)
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
		}
		printw("\n\r");
	}
	refresh();
}

//Initializes the ncurses screen
void init_screen()
{
	initscr();
	start_color();
	nocbreak();
	noecho();

	init_pair(1, COLOR_BLACK, COLOR_GREEN);
}

//Closes up all the ncurses resources
void kill_screen()
{
	getchar();
	endwin();
}

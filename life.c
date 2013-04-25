#include<stdio.h>
#include<stdlib.h>
#include<ncurses.h>
#include<pthread.h>
#include<unistd.h>
#include<time.h>

#define MAX 200
#define MAXTHREADS 2

typedef char * string;

typedef struct _colony
{
	int **arr1;
	int **arr2;
	int start;
	int end;
} colony;

void run(int numbersOn);
void print(int **buf, int numbersOn);
void init_screen();
void kill_screen();
int a(int i);
int s(int i);

//Thread function
void *calcNextGen(void *th);

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

//This is the function that initializes everything and then loops through the generations.
void run(int numbersOn)
{
	int i, j;
	int **arr1 = (int **)malloc(sizeof(int *) * MAX);	//Create 2 arrays of arrays with 20 rows
	int **arr2 = (int **)malloc(sizeof(int *) * MAX);

	//Gonna just deal with 2 threads
	colony colonies[MAXTHREADS];
	pthread_t thread[MAXTHREADS];

	for(i = 0; i < MAX; i++)
	{
		arr1[i] = (int *)malloc(sizeof(int) * MAX);		//Create 20 columns per each row 
		arr2[i] = (int *)malloc(sizeof(int) * MAX);		//Create 20 columns per each row 
	}

	srand(time(NULL));

	int ranx = rand() % (MAX - 4) + 4;
	int rany = rand() % (MAX - 4) + 4;

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

	//Split up the data into multiple threads.
	int split = MAX / MAXTHREADS;

	for(i = 0; i < MAXTHREADS; i++)
	{
		colonies[i].arr1 = arr1;
		colonies[i].arr2 = arr2;
		colonies[i].start = i * split;
		colonies[i].end = (i + 1) * split;
	}

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
	
		//Start the 2 threads
		for(i = 0; i < MAXTHREADS; i++)
		{
			pthread_create(&thread[i], NULL, calcNextGen, &colonies[i]);
		}

		//Close the 2 threads
		for(i = 0; i < MAXTHREADS; i++)
		{
			pthread_join(thread[i], NULL);
		}

		//Print out the generation
		print(arr1, numbersOn);
	}
}

/**
 * This function is passed into the thread to run along with the struct of data to manipulate.
 */
void *calcNextGen(void *th)
{
	colony* data = (colony*)th;
	int i, j, k, count;
	
	int neighbors[8];
	
	for(i = data->start; i < data->end; i++)
	{
		for(j = 0; j < MAX; j++)
		{
			neighbors[0] = data->arr2[i][a(j+1)];
			neighbors[1] = data->arr2[s(i-1)][a(j+1)];
			neighbors[2] = data->arr2[s(i-1)][j];
			neighbors[3] = data->arr2[s(i-1)][s(j-1)];
			neighbors[4] = data->arr2[i][a(j-1)];
			neighbors[5] = data->arr2[a(i+1)][s(j-1)];
			neighbors[6] = data->arr2[a(i+1)][j];
			neighbors[7] = data->arr2[a(i+1)][a(j+1)];

			count = 0;
			for(k = 0; k < 8; k++)
			{
				if(neighbors[k] == 1)
				{
					count++;
				}
			}
				
			if(data->arr1[i][j] == 1 && count < 2)
			{//rule 1
				data->arr1[i][j] = 0;
			}
			else if(data->arr1[i][j] == 1 && count <= 3 && count >= 2)
			{//rule 2
				data->arr1[i][j] = 1;
			}
			else if(data->arr1[i][j] == 1 && count > 3)
			{//rule 3
				data->arr1[i][j] = 0;
			}
			else if(data->arr1[i][j] == 0 && count == 3)
			{//rule 4
				data->arr1[i][j] = 1;
			}
		}
	}
}

//Quick function to test Max edges
int a(int i)
{
	if(i > MAX - 1)
	{
		return(0);
	}
	return(i);
}

//Quick function to test the 0 edges
int s(int i)
{
	if(i < 0)
	{
		return(MAX - 1);
	}
	return(i);
}

//Prints out everything in the array
void print(int **buf, int numbersOn)
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

//Initializes the ncurses screen
void init_screen()
{
	initscr();
	start_color();
	cbreak();
	noecho();

	init_pair(1, COLOR_BLACK, COLOR_GREEN);
}

//Closes up all the ncurses resources
void kill_screen()
{
	getchar();
	endwin();
}

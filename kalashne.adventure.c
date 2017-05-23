#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include <dirent.h>
#include <assert.h>
#include <time.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <limits.h>
#include <pthread.h>

#define FALSE 0
#define TRUE !FALSE

extern  int alphasort();
char pathname[MAXPATHLEN];
char newest[PATH_MAX];
time_t mtime=0;
char *ROOMS[7];
char FILE_PATH[100];
char path[150];
int step_count=0;
char TIME_FILE[256];

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

void* get_time();

/*Main Game Function
 * Input: The most recently created directory and START room
 * Allows player to move around rooms until END room is found
 */
				
int game(char *dir_name, char *room)
{
	char substr[2],
		input[2],
		line[256],
		options[7],
		comp[22]="ROOM TYPE: END_ROOM",
		check_time[5]="time",
		*check;

	int num_lines=0,
		currLine=0,
		i=0;

	pthread_t thread_id;
	
	sprintf(FILE_PATH, "%s/%s", dir_name, room);

	FILE *fp;

	fp=fopen(FILE_PATH, "r");

	//Gets the number of lines in a file
	while (fgets(line, sizeof line, fp) != NULL)
	{
		num_lines++;

		//If any of the lines indicate the END room, 
		//Game ends on that room
		check=strstr(line,comp);
                if (check!=NULL)
                {
                        printf("CONGRATULATIONS, YOU HAVE FOUND THE END ROOM!\nYOU TOOK %i STEPS\nYOUR PATH TO VICTORY WAS %s\n", step_count,path);
			fflush(stdout);
                        exit(0);
                }

	}


	fclose(fp);

	fp=fopen(FILE_PATH, "r");

	//The first line of each file is the name of the room
	while(fgets(line,sizeof line, fp) != NULL)
	{
		if (currLine == 0)
		{
			//Grab the last char in the
			//first line of the file 	
			strncpy(substr,line+11,1);
			substr[1]='\0';	
			printf("CURRENT LOCATION: %s\n", substr);
			fflush(stdout);
		}

		//The rest of the lines (minus last) are room connections
		//For formatting purposes, grabbing the second 
		//line down separately from the rest
		if(currLine > 0)
		{
			if (currLine== 1)
			{
				printf("POSSIBLE CONNECTIONS: ");
				fflush(stdout);
				strncpy(substr,line+14,1);
				substr[1]='\0';
				printf("%s", substr);
				fflush(stdout);
				//To keep track of available rooms,
				//Creating an array "options"
				//That will lated be used that 
				//User entered a room from the list
				strcat(options,substr);	
			}	

			else if (currLine > 1 && currLine < num_lines-1)
			{
				printf(", ");
				fflush(stdout);
				strncpy(substr,line+14,2);
				substr[1]='\0';
				printf("%s", substr);
				fflush(stdout);
				strcat(options, substr);	
			}

			else
				printf(".");			
		}
		
		currLine++;
	}

	fclose(fp);	

	//Asking for user input to determine
	//The next room to be entered
	printf("\nWHERE TO? >");
	fflush(stdout);
	scanf("%s",input);
	fflush(stdin);

	//Check if user entered "time"
	check=strstr(check_time,input);

	//If they did, create a new thread 
	//And run the get_time function
	//Then return control back to 
	//Current function and continue
	//Where user left off
	if(check != NULL){
		pthread_create(&thread_id, NULL, get_time, NULL);
		pthread_join(thread_id, NULL);

		game(dir_name, room);
	}



	for(i=0; i<num_lines;i++)
	{
		//If input is among possible options, 
		//We increment step count,
		//And go to the next room
		check=strstr(options,input);

		if(check != NULL)
		{
			strcat(path,"\n");
			strcat(path,input);
			step_count++;

			//Create a variable to hold 
			//User's input as that is 
			//Where the next iteration
			//Of the game will take place 
			char *next_room=malloc(2);
			assert(next_room!=0);
			memset(next_room, '\0', 25);
			strncpy(next_room, input,1);
 
			game(dir_name,next_room);
		}

		else 
		{
			//If user enters invalid input, 
			//Complain a little 
			//And read in input again
			printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n");			
			fflush(stdout);
			scanf("%s", input);
			fflush(stdin);
		}
		 
	}

	return 0; 
}


//Helper function to get rid of the "." and ".." 
//When returning current working directory
int file_select(const struct direct *entry)
{
        if ((strcmp(entry->d_name, ".") == 0) ||(strcmp(entry->d_name, "..") == 0))
                 return (FALSE);
        else
                return (TRUE);
}

//Function utilizes mutexes/pthread library
//To create a new thread that opens a file, 
//Writes time to it and prints out that time
//To the user upon request
void* get_time()
{
	//new thread gets the lock
	pthread_mutex_lock(&mutex1);
	
	//Using time.h library to get
	//Current local time
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo=localtime(&rawtime);

	//Converting some numerical output
	//To strings to represent day/month
	int i=0,
		j=0,
		k=0,
		hrs=0;

	char *day[]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

	char *month[]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

	char *am_pm[]={"AM", "PM"};

	i=timeinfo->tm_wday;
	j=timeinfo->tm_mon;
	hrs=timeinfo->tm_hour;
	
	if((timeinfo->tm_hour>=0) && (timeinfo->tm_hour <12)){
		k=0;
		hrs=timeinfo->tm_hour;
	}

	if(timeinfo->tm_hour>12)
		hrs-=12;

	if((timeinfo->tm_hour>=12))
		k=1;

	//Create a new file in the same directory
	//As the executing program. 
	//This file holds current time output
	getwd(TIME_FILE); 
	char *file_str="currentTime.txt";

	sprintf(TIME_FILE, "%s/%s", TIME_FILE, file_str);

	FILE *fp;
	fp = fopen(TIME_FILE, "w");

	//Write to that file in the format requested
	fprintf(fp, "\n%d:%d%s, %s, %s %d, %d\n\n", hrs, timeinfo->tm_min, am_pm[k], day[i], month[j], timeinfo->tm_mday, timeinfo->tm_year+1900);
	
	fclose(fp);

	char line[256];

	//Read from file created above
	fp=fopen(TIME_FILE, "r");

	//Outout contents of it to the user
	while(fgets(line, sizeof line, fp) != NULL)
		printf(line);

	fclose(fp);

	//Unlock the mutex to allow the main program continue
	pthread_mutex_unlock(&mutex1);
}



 
int main()
{
	srand(time(NULL));

	int pid = getpid(),
		i = 0,
		j=0,
		size = 25,
		count=0;

		
	//Since many different room files can be created,
	//The program will check which of those files 
	//Is the most recent and work with that file
	struct direct **files;
	int file_select();
	char file[256];
	memset(file,'\0',256);

	//Only checking room files because
	//The most recent file overall is
	//The .o of currrent program
	char cmp[16]="kalashne.rooms.";

	char *check; 

	struct stat *st;
	st=malloc(sizeof(struct stat));

	getwd(pathname); //get the current working directory
	count=scandir(pathname, &files, file_select, alphasort); //find out how many files are in the directory 

        for (i=1;i<count+1;++i){

                sprintf(file,"%s/%s",pathname,files[i-1]->d_name);
		check=strstr(file,cmp);

		//Get st_mtime if the file in question is .rooms file
		if(check!=NULL){
                	stat(file,st);
                	int time = st->st_mtime;

			//If the file is more recent than "newest"
			//It replaces "newest" (global var to keep
			//Track of which file to use in the program)
                	if(time>mtime){
                        	mtime=time;
                        	strncpy(newest,file,PATH_MAX);
			}
       		 }

	}	

	//Populate the ROOMS array with values 
	//From the most recent .rooms directory
	DIR  *dir;
	struct dirent *ent;

	i=0;
	j=0;

	//Open the directory and read it
	//Save file names into an array
	if((dir=opendir(newest))!=NULL){
		while((ent=readdir(dir))!=NULL){
			
			if(i>1){
				ROOMS[j]=ent->d_name;
				j++;
			}
			i++;
		}
	}

	closedir(dir);
		
	//Create a file path with the newest directory
	//And rooms in it. Create a variable room to
	//Keep track of the START room 
	for (i=0; i < 7; i++)
	{
		
		sprintf(FILE_PATH,"%s/%s",newest,ROOMS[i]);
                char room[2];
                memset(room,'\0',2);
                strcpy(room,ROOMS[i]);


		FILE *fp;
		fp = fopen(FILE_PATH, "r");
		char line[256];
		char comp[22]="ROOM TYPE: START_ROOM";
		char *check;

		//If the room in question is the START room,
		//Launch thegame. Otherwise, circle back 
		//And check the following room. 
		while(fgets(line,sizeof line, fp) != NULL)
		{
			check=strstr(line,comp);
	
			if(check!=NULL)
			{
				printf("WELCOME, PLAYER\n");
				fflush(stdout);

				game(newest,room);
			}	
		}


	}
		

	return 0;
}


/* Resources: 
 *
 * getwd() - https://goo.gl/YntFfi
 * opendir()/readdir() - https://goo.gl/09sqhU
 * https://goo.gl/D9iom5
 * https://goo.gl/bpMI7t
 * file I/O - https://goo.gl/qACIsH 
 * https://goo.gl/SNqYIR
 * stat() - https://goo.gl/2z5lC8
 * https://goo.gl/HhYzib
 * alphasort/scandir - https://goo.gl/392ctD
 * sprintf - https://goo.gl/HkzBw3
 * strstr - https://goo.gl/mG1P0x
 *
 */

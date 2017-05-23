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

char *ROOMS[7];
char FILE_PATH[100];
int CONN_MATRIX[7][7] = {{0}};
const char *RM_NAMES[] ={"A","B","C","D","E","F","G","H","I","J"};

/*Create a directory called kalashne.rooms.processID
*
*Input: current process id
*
*Allocates and  verifies directory name string
*memset to fill name with \0 characters
*sprint creates a string from input name and current process id
*
*Using Stack Overflow example to create a new directory with mkdir
*stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
*/

char *build_rooms(int pid)
{
	int size=25,
		i=0,
		j=0,
		sum=0,
		rand_int=0,
		rand_room=0,
		num_conn=0;

	char *dir_name=malloc(size);
	assert(dir_name!=0);
	memset (dir_name,'\0',25);
	
	int *tempArr=malloc(size);
        assert (tempArr!=0);

        for (i=0;i<size;i++)
                tempArr[i]=0;
	
	//Make a string dir_name
	char *name= "kalashne.rooms.";
	snprintf(dir_name, size,"%s%d",name,pid);
	
	//Create a directory with string from above
        if (mkdir(dir_name, 0755) < 0)
		exit(1);
        

	/* Generates 7 different room files, one room per file
	*
	* rand() generates a random number between 0 and 9
	* Create a temporary array
	* If arr value is set to 0, a new room is created,
	* Assign name, and added to the directory
	*
	*/
             
	size=10;

	i=0;

        while(i<7)
        {
		//generate a random number 
                rand_int = rand()%10;

                while(tempArr[rand_int]==0)
                {	
                        sprintf(FILE_PATH,"%s/%s", dir_name, RM_NAMES[rand_int]);

			//Open the file and document room name
                        FILE *fp;
                        fp = fopen(FILE_PATH,"a");
                        fprintf(fp, "ROOM NAME: %s\n", RM_NAMES[rand_int]);
                        fclose(fp);
                        ROOMS[i]=RM_NAMES[rand_int];
                        tempArr[rand_int]=1;
                        i++;
                }
        }


	/* Gives each room 3-6  outgoing connections
	* That are randomly assigned and written to 
	* The room file along with room type
	*/

        for(i=0;i<7;i++)
        {
		//Generate random number
                rand_int = (rand()%4)+3;
		sum=0;

		//All rooms are connected
		//To other rooms at random
                for(j=0; j < 7; j++)
                        sum+=CONN_MATRIX[i][j];

                while(sum < rand_int + 1)
                {
                        rand_room = 0;
                        rand_room = rand()%7;

			while (rand_room == i)
                        	rand_room = rand() % 7;

			//Keep track of how many 
			//Connections each room already has
                        CONN_MATRIX[i][rand_room] = 1;
                        CONN_MATRIX[rand_room][i] = 1;
		
			sum++;
                }
        }

	//Write those connections to the room file
        for(i=0;i<7;i++)
        {
                num_conn = 1;

                for(j=0;j<7;j++)
                {
			//If a connection between the two rooms exists
			//Open that file and document that connection
                        if (CONN_MATRIX[i][j] == 1)
                        {
                                sprintf(FILE_PATH, "%s/%s",dir_name,ROOMS[i]);
                                FILE *fp;
                                fp=fopen(FILE_PATH,"a");
                                fprintf(fp, "CONNECTION %d: %s\n", num_conn, ROOMS[j]); 
                                fclose(fp);
                                num_conn++;
                        }
                }
 
		//Assign START/END/MID to each room
		//Since rooms above were randomly generated,
		//The first room gets START, last room gets 
		//END, and the rest get MIDDLE (randomness
		//Already assured above)
		sprintf(FILE_PATH, "%s/%s",dir_name,ROOMS[i]);
                FILE *fp;
                fp=fopen(FILE_PATH,"a");

                if(i==0)
                	fprintf(fp, "ROOM TYPE: START_ROOM\n");
                else if(i==6)
                        fprintf(fp, "ROOM TYPE: END_ROOM\n");
                else
                	fprintf(fp, "ROOM TYPE: MID_ROOM\n");

		fclose(fp);
	}

	return dir_name;
}






int main()
{
	srand(time(NULL));
	int pid = getpid();

	build_rooms(pid);

	return 0;
}























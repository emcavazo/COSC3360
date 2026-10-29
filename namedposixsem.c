#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h> 
#include <semaphore.h> 
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>

int main()
{
	sem_t *mysem;
	char name[] = "CARC"; 
	int value=10;
	unsigned int initial_value = 1;
	mysem = sem_open(name, O_CREAT,0600, initial_value); // user has permission to read + write
	sem_wait(mysem);
	sem_getvalue(mysem,&value);	
	printf("Semaphore Value %d\n",value);
	sleep(2);
 	sem_post(mysem); //like signal
	sem_unlink(name); // delete signal
}

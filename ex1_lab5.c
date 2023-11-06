#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include<aio.h>


int main(int argc, char * argv[])
{

	if(argc != 2)
	{
		printf("Usage /.program  <fisier_intrare>\n");
		return 1;
	}
	
	int input =open(argv[1],O_RDONLY );
		if(input == -1 )
		{
			perror("Eroare la deschidere");
			exit(-1);
		}	
	
	int output=creat("statistica.txt",S_IRUSR);
		if(output == -1)
		{
			perror("Eroare la crearea fisierului statistica.txt");
			int inchidere=close(input);
			if(inchidere == -1)
		{
			perror("Eroare la inchidere");
			exit(-4);
		}
		}
		
		
		ssize_t bytes=0;
		char aux[100];
		
		bytes=read(input,aux,18);
		
			if(bytes == -1)
			{
				perror("eroare la citire");
				exit(-5);
			}
			
		int latime;	
		bytes=read(input,&latime,4 );
		if(bytes == -1)
			{
				perror("eroare la citire");
				exit(-6);
			}
		
		
			
		
	
		int inaltime;
		bytes=read(input,&inaltime,4 );
		
		
		if(bytes == -1)
			{
				perror("eroare la citire");
				exit(-6);
			}
		
		struct stat file_stat;
		fstat(input,&file_stat);
		
		int size=file_stat.st_size;
		
		 int id_owner =file_stat.st_uid;
		 printf("%d\n",id_owner);
		struct timespec  st_atim;
		st_atim = file_stat.st_atim;
		
		
	int c=close(input);
		if(c == -1)
		{
			perror("Eroare la inchidere");
			exit(-3);
		}
return 0;
}































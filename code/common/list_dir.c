#include "reliable_udp.h"

char *list_dir(char *PATH) 
{
	
	DIR *d;
  	struct dirent *dir;
  	char *buff = dynamic_allocation(sizeof(char)*100); //TODO: rendere il buffer allocato dinamicamente (realloc ?) 
  	int v = 0;

  	d = opendir(PATH);

  	if (d) {
    	while ((dir = readdir(d)) != NULL) {
		if (strstr(".", dir->d_name) == NULL && strstr("..", dir->d_name) == NULL) //elimina le due root dir 
     	 		v += sprintf(buff+v, "%s\n", dir->d_name);		
    	}
        closedir(d);
     
     } else {
  		/* could not open directory */
 		 perror ("Error in opendir");
  		 exit(EXIT_FAILURE);
     }


    return buff;
}

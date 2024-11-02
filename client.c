
    #include <stdio.h>
    #include <stdlib.h>
    #include <errno.h>
    #include <string.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>

    #define FIFO_NAME "MyTest_FIFO" // client-> server
    #define FIFO_NEW_NAME "MyNew_FIFO" // server->client

    int main()
    {
        char s[300];
        int num, fd, fd_new;

        char s_return[300];

        mknod(FIFO_NEW_NAME, S_IFIFO | 0666, 0);

        fd = open(FIFO_NAME, O_WRONLY);
        printf("Introduceti o comanda \n");
        
        fd_new = open(FIFO_NEW_NAME, O_RDONLY);

        while (gets(s), !feof(stdin)) 
        {
            if ((num = write(fd, s, strlen(s))) == -1)
                perror("Problema la scriere in FIFO!");
            
            if ((num = read(fd_new, s_return, 300)) == -1)
                perror("Eroare la citirea din FIFO!");
            else 
            {
                s_return[num] = '\0';
                
                if(strcmp(s_return,"quit")==0)
                {
                    printf("Se inchide clientul \n");
                    break;
                }                    

                printf("%s\n", s_return);
            }
        }
    }

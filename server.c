
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utmp.h>
#include <sys/time.h>
#include <time.h>


#define FIFO_NAME "MyTest_FIFO"
#define FIFO_NEW_NAME "MyNew_FIFO"

#define MSG1 "Comunicam prin socketi!" 
#define MSG2 "Atentie!Socketpair() o generalizarea a pipe-urilor"
#define MSG3 "Successfull authentication"
#define MSG4 "Unsuccessfull authentication"
#define MSG5 "User not authenticated"

char** logged_users;
int logged_users_num;

char* get_logged_users()
{
    struct utmp* data;
    char user[50], host[50];
    
    int sockp[2],user_found = 0;
    char msg[1024], username[300];
	pid_t pid;
    
    char* responseToServer;
    
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0)
    {
        perror("Eroare la socketpair");
        exit(1);
    }
    
    if ((pid = fork()) == -1)
         perror("Eroare la fork");
    else
    {
        if (pid>0)   //parinte 
        {
            close(sockp[0]);
            if (read(sockp[1], msg, 1024) < 0) 
				perror("[parinte ]Eroare la read");

            close(sockp[1]);
            
            responseToServer = malloc(sizeof(char) * (strlen(msg) + 1));
            
            strcpy(responseToServer, msg);
            
            printf("responseToServer este %s\n", responseToServer);
            
            return responseToServer;
            
        }
        else     //copil
        {
            close(sockp[1]);
            
            if (logged_users_num)
            {
                char arr_get_logged_users[300];
                
                strcpy(arr_get_logged_users, "");
                
                data = getutent();
                //getutent() populates the data structure with information 
                while (data != NULL)
                {
                    if (data->ut_type == USER_PROCESS) // utilizator
                    {
                        strcat(arr_get_logged_users, data->ut_user);
                        strcat(arr_get_logged_users, "\n");
                        strcat(arr_get_logged_users, data->ut_host); 
                    }
                
                    data = getutent();
                }
                
                if (write(sockp[0], arr_get_logged_users, (strlen(arr_get_logged_users)+1)) < 0) 
					perror("[copil]Eroare la write");
                
            }
            else
                if (write(sockp[0], MSG5, sizeof(MSG5)) < 0) 
					perror("[copil]Eroare la write");
            
            close(sockp[0]);
            exit(0);
        }
    }
    return responseToServer;
}

int login_username_cmd(char* command)
{
    int sockp[2],utilizator = 0;
    char msg[1024], username[300];
    pid_t pid;

    char* token = strtok(command, " ");

    while (token) {
        if (strcmp(token, "login") != 0 && strcmp(token, ":") != 0)
            strcpy(username, token);
        token = strtok(NULL, " ");
    }

    printf("Username-ul este %s\n", username);

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0)
    {
        perror("Eroare la socketpair");
        exit(1);
    }

    if ((pid = fork()) == -1) 
		perror("Eroare la fork");
    else
        if (pid)   //parinte 
        {
            close(sockp[0]);
            if (read(sockp[1], msg, 1024) < 0)
				 perror("[parinte]Eroare la read");

            printf("[parinte] %s\n", msg);
            close(sockp[1]);

            if (strcmp(msg, "Successfull authentication") == 0)
                return 1;
            else
                return 0;

        }
        else     //copil
        {

            FILE* file = fopen("users.config", "r");

            char line[256];

            if (file != NULL) {
                while (fgets(line, sizeof(line), file)) 
				{
                    if (strcmp(strtok(line, "\n"), username) == 0)
                    {
                        utilizator = 1;
                        break;
                    }
                }

                fclose(file);
            }
            
            close(sockp[1]);

            if (utilizator == 1)
            {
                if (write(sockp[0], MSG3, sizeof(MSG3)) < 0) 
					perror("[copil]Eroare la write");
            }
            else
            {
                if (write(sockp[0], MSG4, sizeof(MSG4)) < 0) 
					perror("[copil]Eroare la write");
            }
            //if (read(sockp[0], msg, 1024) < 0) perror("[copil]Err..read"); 
            //printf("[copil]  %s\n", msg); 
            close(sockp[0]);
            exit(0);
        }

}

int quit_command()
{
    int pipe_child_to_parent[2];
    pid_t pid;
    char mesaj[255];

    if (pipe(pipe_child_to_parent) == -1)
    {
        perror("Eroare la crearea pipe-ului");
        exit(1);
    }

    if ((pid = fork()) == -1)
    {
        perror("Eroare la crearea procesului");
        exit(1);
    }

    if (pid > 0)  // Parent process
    {
        close(pipe_child_to_parent[1]);

        int num = read(pipe_child_to_parent[0], mesaj, sizeof(mesaj));

        if (num == -1)
            perror("Eroare la citire");
        else
            printf("%s \n", mesaj);

        close(pipe_child_to_parent[0]);
    }
    else  // copil
    {
        close(pipe_child_to_parent[0]);

        const char* message = "Se inchide serverul";
        int num = write(pipe_child_to_parent[1], message, strlen(message) + 1);
        if (num == -1)
        {
            perror("Eroare la scriere");
        }

        close(pipe_child_to_parent[1]);
        exit(0);
    }

    return 0;
}

int logout_command(int counter_login)
{
    int sockp[2];
    char msg[255];
    pid_t pid;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0)
    {
        perror("Eroare la socketpair");
        exit(1);
    }

    if ((pid = fork()) == -1)
        perror("Eroare la fork");
    else
        if (pid > 0)
        {
            close(sockp[0]);
            if (read(sockp[1], msg, 1024) < 0)
                perror("Eroare la citire");
            else
                printf("%s \n",msg);

            if (counter_login == 0) // daca utilizatorul nu este logat
                return 0;
            else
                return 1;

            close(sockp[1]);
        }
        else
        {
            close(sockp[1]);
            write(sockp[0],"M-am delogat", sizeof("M-am delogat"));
            close(sockp[0]);
            exit(0);
        }
}

int get_proc_info_command(int counter_login, char* received_command, int fd_new, char **getProcInfoResponse)
{
    int sockp[2];
    pid_t pid;
    char msg[1024];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0)
    {
        perror("Eroare la socketpair");
        exit(1);
    }

    if ((pid = fork()) == -1)
        perror("Eroare la fork");
    else
        if (pid > 0)   //parinte 
        {
            close(sockp[0]);
            if (read(sockp[1], msg, 1024) < 0) 
                perror("[parinte]Eroare la read");
            //printf("%s", msg);

            if (counter_login == 0)
                return 0;
            else
            {
                *getProcInfoResponse = malloc(sizeof(char) * (strlen(msg) + 1));
                strcpy(*getProcInfoResponse, msg);
                return 1;
            }
                

            close(sockp[1]);
        }
        else
        {
            close(sockp[1]);

            char pid[255], path[255];
            int cnt = 0;

            if (counter_login == 0)
                return 0;

            for (int i = 16; i < strlen(received_command); i++)
                pid[cnt++] = received_command[i];

            pid[cnt] = '\0';

            sprintf(path, "/proc/%s/status", pid);

            FILE* file = fopen(path, "r");
            char line[255];
            char buffer[255] = {};

            if (file != NULL)
            {
                while (fgets(line, sizeof(line), file))
                {
                    if (strstr(line, "Name"))
                    {
                        //printf("AM intrat \n");

                        char name[255];
                        int ind = 0;

                        for (int i = 7; i < strlen(line) - 1; i++)
                            name[ind++] = line[i];

                        name[ind] = '\0';
                        strcat(buffer, "Name: ");
                        strcat(buffer, name);
                        strcat(buffer, "\n");
                    }
                    else
                        if (strstr(line, "State"))
                        {
                            //printf("AM intrat \n");

                            char name[255];
                            int ind = 0;

                            for (int i = 8; i < strlen(line) - 1; i++)
                                name[ind++] = line[i];

                            name[ind] = '\0';
                            strcat(buffer, "State: ");
                            strcat(buffer, name);
                            strcat(buffer, "\n");
                        }
                        else
                            if (strstr(line, "PPid"))
                            {
                                //printf("AM intrat \n");

                                char name[255];
                                int ind = 0;

                                for (int i = 6; i < strlen(line) - 1; i++)
                                    name[ind++] = line[i];

                                name[ind] = '\0';
                                strcat(buffer, "PPid: ");
                                strcat(buffer, name);
                                strcat(buffer, "\n");
                            }
                            else
                                if (strstr(line, "Uid"))
                                {
                                    //printf("AM intrat \n");

                                    char name[255];
                                    int ind = 0;

                                    for (int i = 3; i < strlen(line); i++)
                                        name[ind++] = line[i];

                                    name[ind] = '\0';
                                    strcat(buffer, "Uid");
                                    strcat(buffer, name);
                                }
                                else
                                    if (strstr(line, "VmSize"))
                                    {
                                        //printf("AM intrat \n");

                                        char name[255];
                                        int ind = 0;

                                        for (int i = 7; i < strlen(line); i++)
                                            name[ind++] = line[i];

                                        name[ind] = '\0';
                                        strcat(buffer, "VmSize:");
                                        strcat(buffer, name);
                                    }

                }
                //write(fd_new,buffer,sizeof(buffer));

            }
            else
                perror("Eroare la deschidere");
            //printf("buffer: %s\n", buffer);
            if (write(sockp[0], buffer, sizeof(buffer)) < 0)
                perror("Eroare la write");

            fclose(file);

            close(sockp[0]);
            exit(1);
            return 1;
        }
}

int parse_command(char* s) // aici verificam ce comanda este
{
    if(strstr(s, "login"))
        return 1;
        
    if(strstr(s,"get-logged-users"))
        return 2;

    if(strstr(s, "get-proc-info"))
        return 3;

    if(strstr(s, "logout"))
        return 4;

    if(strstr(s, "quit"))
        return 5;
}

int main()
{
    char s[300];
    char received_command[300];
    int num, num_return, fd, fd_new,counter_logout = 0;

    char s_return[300], pid_command[255];

    mknod(FIFO_NAME, S_IFIFO | 0666, 0);

    printf("Astept o comanda \n");

    fd = open(FIFO_NAME, O_RDONLY);
    fd_new = open(FIFO_NEW_NAME, O_WRONLY);

       if ((num = read(fd, s, 300)) == -1)  // citim comanda de la client catre server
            perror("Eroare la citirea din FIFO!");
       else
            while(num>0) // in caz ca s-a citit
            {
                s[num] = '\0';
                strcpy(received_command, s); // facem o copie pt ca la strtok s devine null
                                                   // verificam ce comanda e
                if (parse_command(s) == 1)
                {
                    printf("Received the 'login : username' command\n");
                    printf("Comanda este: %s\n", received_command);
                    logged_users_num++;

                    if (login_username_cmd(received_command) == 1)
                     {
                        if ((num_return = write(fd_new, "Successfull authentication", strlen("Successfull authentication"))) == -1)
                            perror("Problema la scriere in FIFO de la server la client!");
                     }
                    else
                    {
                        if ((num_return = write(fd_new, "Unsuccessfull authentication", strlen("Unsuccessfull authentication"))) == -1)
                            perror("Problema la scriere in FIFO de la server la client!");
                    }
                }
                else
                    if (parse_command(s) == 5)
                    {
                        printf("Received the command quit \n");

                        if ((num_return = write(fd_new, "quit", strlen("quit"))) == -1)
                            perror("Problema la scriere in FIFO de la server la client!");
            
                        quit_command();
                    }
                    else
                        if (parse_command(s) == 4)
                        {
                            printf("Received the command logout \n");
                            counter_logout = 1;

                            if (logout_command(logged_users_num) == 0)
                            {
                                if ((num_return = write(fd_new, "Eroare la logout", strlen("Eroare la logout"))) == -1)
                                    perror("Problema la scriere in FIFO de la server la client!");
                            }
                            else
                            {
                                logged_users_num--;

                                if ((num_return = write(fd_new, "Logout a functionat!", strlen("Logout a functionat!"))) == -1)
                                    perror("Problema la scriere in FIFO de la server la client!");
                            }
                        }
                        else
                            if (parse_command(s) == 3)
                            {
                                char* getProcInfoResponse;

                                printf("Received the command get-proc-info : pid \n");

                                if (get_proc_info_command(logged_users_num, received_command, fd_new, &getProcInfoResponse) == 0)
                                {
                                    if ((num_return = write(fd_new, "Trebuie sa fii mai intai autentificat!", strlen("Trebuie sa fii mai intai autentificat!"))) == -1)
                                        perror("Problema la scriere in FIFO de la server la client!");
                                }
                                else
                                {
                                    if (strlen(getProcInfoResponse))
                                    {
                                        if ((num_return = write(fd_new, getProcInfoResponse, (strlen(getProcInfoResponse)+1))) == -1)
                                            perror("Problema la scriere in FIFO de la server la client!");
                                
                                        free(getProcInfoResponse);
                                    }
                                }
                            }
                            else
                                if (parse_command(s) == 2)
                                {
                                    char *arrGetLoggedUsers = get_logged_users();
                                                   
                                    if ((num_return = write(fd_new, arrGetLoggedUsers, (strlen(arrGetLoggedUsers)+1))) == -1)
                                        perror("Problema la scriere in FIFO de la server la client!");
                                }

                                if ((num = read(fd, s, 300)) == -1) 
                                {
                                    perror("Eroare la citirea din FIFO!");
                                    break;
                                }
        }
}

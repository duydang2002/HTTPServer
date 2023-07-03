#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <dirent.h>
#include <pthread.h>

void *client_thread(void *);
int port = 9009;
int main() 
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }
 
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5)) 
    {
        perror("listen() failed");
        return 1;
    }

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }
        printf("New client connected: %d\n", client);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }
    
    close(listener);    

    return 0;
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    char buf[2048];


    int ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = 0;

    puts(buf);

    char method[16], uri[256];
    sscanf(buf, "%s%s", method, uri);
    uri[strlen(uri)]=0;
    puts(uri);
    
    // THu Muc
    if (strchr(uri,'.')==NULL) {
        if (strcmp(uri,"/")==0){
            uri[0]=0;
        }
    char *path= malloc(strlen(uri));
    path[0]='.';
    strcat(path, uri);
    puts(path);

    // Doc noi dung thu muc
    DIR *dir = opendir(path);
    if (dir==NULL){
        char *msg = malloc(strlen("HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<html><body><h1>404 Not Found</h1><p>")+1);
        strcat(msg,"HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<html><body><h1>404 Not Found</h1><p>");
        send(client, msg, strlen(msg),0);
        free(msg);
        free(path);
        return NULL;
    }
    struct dirent *entry;
    
    char begin[256] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";
    begin[strlen(begin)]=0;

    char *msg = NULL;
    int size =0;
 
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == 8)
        {
            char *href = calloc(512,1);
            sprintf(href,"<i><a href= \"%s/%s\"",path,entry->d_name);
            href[strlen(href)] = 0;

            char *fileName=calloc(512,1);
            sprintf(fileName,"%s>%s</a></i><br>", href,entry->d_name);
            fileName[strlen(fileName)]=0;


            msg = realloc(msg,strlen(fileName)+size +1);
            puts(msg);
            size+= (int) strlen(fileName);
            
            strcat(msg,fileName);
            msg[strlen(msg)]=0;
            free(fileName);
            free(href);
            fileName=0;
            href=0;

        }
        else if (entry->d_type ==4){
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
                continue;
            }
            
            char *href=calloc(512,1);
            sprintf(href,"<b><a href= \"%s/%s\"",path,entry->d_name);
            href[strlen(href)]=0;

            char *dirName=calloc(512,1);
            sprintf(dirName,"%s>%s</a></b><br>", href,entry->d_name);
            dirName[strlen(dirName)]=0;

            msg = realloc(msg, size+ strlen(dirName)+1);
            puts(msg);
            size += (int) strlen(dirName);

            strcat(msg, dirName);
            msg[strlen(msg)]=0;
            free(dirName);
            free(href);
            dirName=0;
            href=0;
           
        }
    }
    char contentLength[256];
    
    char *end = "</p></body></html>";

    unsigned int length = strlen(msg)+strlen(end)+strlen(begin);
    char len[64];
    sprintf(len,"%u",length);
    len[strlen(len)]=0;

    sprintf(contentLength, "Content-Length: %s\r\n\r\n<html><body><h1> List </h1><p>", len);
    char *Header = malloc((length+strlen(len))*sizeof(char)); 
    strcat(Header,begin);
    strcat(Header,contentLength);
    strcat(Header,msg);
    strcat(Header,end);
    puts(Header);
    send(client,Header,strlen(Header),0);
    free(dir);
    dir = 0;
    free(path);
    }
    else {
        char *path= malloc(strlen(uri)+1);

    }
    
    close(client);
    
}
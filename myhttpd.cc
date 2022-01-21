const char * usage =
"                                                               \n"
"daytime-server:                                                \n"
"                                                               \n"
"Simple server program that shows how to use socket calls       \n"
"in the server side.                                            \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   daytime-server <port>                                       \n"
"                                                               \n"
"Where 1024 < port < 65536.             \n"
"                                                               \n"
"In another window type:                                       \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where daytime-server  \n"
"is running. <port> is the port number you used when you run   \n"
"daytime-server.                                               \n"
"                                                               \n"
"Then type your name and return. You will get a greeting and   \n"
"the time of the day.                                          \n"
"                                                               \n";


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>
#include <dlfcn.h>
#include <link.h>
#include <errno.h>

int QueueLength = 5;
pthread_mutex_t mutex;
char * stime;
int requests = 0;
double mint = 999999.9;
double maxt = 0.0;
char * minpath;
char * maxpath;
char * parentd = "Parent Directory";
char * dirs = "DIR";
int port = 0;

// Processes time request
void processRequest(int socket);
char * mysubstr(char *str, int begin, int length);
void poolSlave(int socket);
void processRequestThread (int socket);
int sortna(const void *a, const void *b);
int sortnd(const void *b, const void *a);
int sortta(const void *a, const void *b);
int sorttd(const void *b, const void *a);
int sortsa(const void *a, const void *b);
int sortsd(const void *b, const void *a);
void writedir(int socket, char * path, char * file, char * type, char * parent);
void comptime(double diff, char * path);

typedef void (*httprunfunc)(int ssock, const char * querystring);

extern "C" void killzombie(int sig) {
  pid_t pid = wait3(0, 0, NULL);
  while (waitpid(-1, NULL, WNOHANG) > 0);
}

int
main( int argc, char ** argv )
{
  // Print usage if not enough arguments
  if ( argc < 2 ) {
    fprintf( stderr, "%s", usage );
    exit( -1 );
  }

  time_t newtime;
  time(&newtime);
  stime = ctime(&newtime);


  //printf("argc = %d\n", argc);
  //printf("argv[1] = %s\n", argv[1]);
  //printf("argv[2] =  %s\n", argv[2]);
  //printf("argv[1][1] = %d\n", argv[1][1]);
  
  // Get the port from the arguments
  port = atoi( argv[1] );
  if (argc > 2) {
    port = atoi(argv[2]);
  }
  
  // Set the IP address and port for this server
  struct sockaddr_in serverIPAddress; 
  memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
  serverIPAddress.sin_family = AF_INET;
  serverIPAddress.sin_addr.s_addr = INADDR_ANY;
  serverIPAddress.sin_port = htons((u_short) port);
  
  // Allocate a socket
  int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
  if ( masterSocket < 0) {
    perror("socket");
    exit( -1 );
  }

  // Set socket options to reuse port. Otherwise we will
  // have to wait about 2 minutes before reusing the sae port number
  int optval = 1; 
  int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, 
		       (char *) &optval, sizeof( int ) );
   
  // Bind the socket to the IP address and port
  int error = bind( masterSocket,
		    (struct sockaddr *)&serverIPAddress,
		    sizeof(serverIPAddress) );
  if ( error ) {
    perror("bind");
    exit( -1 );
  }
  
  // Put socket in listening mode and set the 
  // size of the queue of unprocessed connections
  error = listen( masterSocket, QueueLength);
  if ( error ) {
    perror("listen");
    exit( -1 );
  }
  
  //p
  if (argv[1][1] == 112) {
    while (1) {
      struct sockaddr_in clientIPAddress;
      int alen = sizeof( clientIPAddress );
      //printf("kqew\n");
      int slaveSocket = accept( masterSocket,
			      (struct sockaddr *)&clientIPAddress,
			      (socklen_t*)&alen);

      //printf("igun\n");
      if ( slaveSocket < 0 ) {
        perror( "accept" );
        exit( -1 );
      }

      struct sigaction sa;
      sa.sa_handler = killzombie;
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = SA_RESTART;

      pid_t slave = fork();
      if (slave == 0) {
        processRequest(slaveSocket);
        shutdown(slaveSocket, SHUT_RDWR);
        sleep(1);
        close(slaveSocket);
        exit(0);
      } else {
        if (sigaction(SIGCHLD, &sa, NULL)) {
          perror("sigaction");
          exit(2);
        }
        close(slaveSocket);
      }

    }
  } else {

    //f
    if (argv[1][1] == 102) {
      while (1) {
        //printf("weqw\n");
        struct sockaddr_in clientIPAddress;
        int alen = sizeof( clientIPAddress );
        //printf("kqew\n");
        int slaveSocket = accept( masterSocket,
			        (struct sockaddr *)&clientIPAddress,
			        (socklen_t*)&alen);

        //printf("igun\n");
        if ( slaveSocket < 0 ) {
          perror( "accept" );
          exit( -1 );
        }

        pthread_t thread[5];
        pthread_mutex_init(&mutex, NULL);
        for (int i = 0; i < 5; i++) {
          pthread_create(&thread[i], NULL, (void *(*)(void *))poolSlave, (void *)masterSocket);
        }
        pthread_join(thread[0], NULL);


      }

    //t
    } else if (argv[1][1] == 116) {
      while (1) {
        //printf("weqw\n");
        struct sockaddr_in clientIPAddress;
        int alen = sizeof( clientIPAddress );
        //printf("kqew\n");
        int slaveSocket = accept( masterSocket,
			        (struct sockaddr *)&clientIPAddress,
			        (socklen_t*)&alen);

        //printf("igun\n");
        if ( slaveSocket < 0 ) {
          perror( "accept" );
          exit( -1 );
        }
        pthread_t thread;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&thread, &attr, (void *(*)(void *))processRequestThread, (void *)slaveSocket);

      }
    } else {

      while (1) {
        struct sockaddr_in clientIPAddress;
        int alen = sizeof( clientIPAddress );
        //printf("kqew\n");
        int slaveSocket = accept( masterSocket,
			        (struct sockaddr *)&clientIPAddress,
			        (socklen_t*)&alen);

        //printf("igun\n");
        if ( slaveSocket < 0 ) {
          perror( "accept" );
          exit( -1 );
        }

        // Process request.
        processRequest( slaveSocket );

        // Close socket
        shutdown(slaveSocket, SHUT_RDWR);
        sleep(1);
        close( slaveSocket );
      }
    }

  }

}

void
processRequestThread (int socket)
{
  processRequest(socket);
  shutdown(socket, SHUT_RDWR);
  sleep(1);
  close(socket);
}

void
processRequest(int socket) 
{
  const int MaxName = 1024;
  char name[MaxName];
  int nameLength = 0;
  int n;
  clock_t st;
  clock_t et;
  clock_t nt;
  st = clock();
  printf("%LF\n", (long double)st);

  unsigned char newChar;
  unsigned char oldChar;
  int gotGet = 0;
  int gotCrf = 0;
  char docpath[MaxName];

  requests++;

  while ((n = read(socket, &newChar, sizeof(newChar))) > 0) {

    if (newChar == ' ') {
      if (strncmp(name, "GET", strlen("GET"))  == 0) {
        gotGet = 1;
        //printf("' ': %s\n", name);
        nameLength = 0;

      } else if (gotGet == 1) {
        name[nameLength] = '\0';
        strcpy(docpath, name);
        gotGet = 0;
        //printf("docpath: %s\n", docpath);
        //printf("name: %s\n", name);

      }

    } else if (newChar == '\n' && oldChar == '\r') {
      nameLength = 0;
      if (gotCrf = 1) {
        break;
      }
      gotCrf = 1;

    } else {
      oldChar = newChar;
      if (newChar != '\r') {
        gotCrf = 0;
      }
      nameLength++;
      name[nameLength-1] = newChar;
           printf("else: %s\n", name);
    }

  }

  char filepath[MaxName+1];

  char * cwd = (char *)malloc(sizeof(char) * 256);
  cwd = getcwd(cwd, 256);
  int x = 0;
  //printf("cwd: %s\n", cwd);
  printf("22222222222222docpath: %s\n", docpath);
  if (strncmp(docpath, "/icons", 6) == 0) {
    sprintf(filepath, "%s/http-root-dir%s", cwd, docpath);
  } else if (strncmp(docpath, "/htdcos", 7) == 0) {
    sprintf(filepath, "%s/http-root-dir%s", cwd, docpath);
  } else if (strcmp(docpath, "/") == 0 || strlen(docpath) == 0) {
    printf("SSSSSSSSSSSSSSSSSS\n");
    sprintf(filepath, "%s/http-root-dir/htdocs/index.html", cwd);
  } else if (strstr(filepath, "..") != NULL) {
    x = 1;
  } else if (strncmp(docpath, "/cgi-bin", strlen("/cgi-bin")) == 0) {
    printf("CGI\n");
    sprintf(filepath, "%s/http-root-dir%s", cwd, docpath);
  } else {
    //printf("last: %s\n", docpath);
    sprintf(filepath, "%s/http-root-dir/htdocs%s", cwd, docpath);
  }

  printf("This is FILEPATHHHHH: %s\n", filepath);

  char contentType[256];
  DIR * dir;
  char * initial;


  if ((dir=opendir(filepath)) != NULL || mysubstr(filepath, strlen(filepath)-8, strlen(filepath)) == "?C=N;O=A" || mysubstr(filepath, strlen(filepath)-8, strlen(filepath)) == "?C=N;O=D" ) {

              printf("substr: %s\n", mysubstr(filepath, strlen(filepath)-8, strlen(filepath)));

    char c;
    char o;

    if (dir == NULL) {
      c = filepath[strlen(filepath)-5];
      o = filepath[strlen(filepath)-1];
      dir = opendir(filepath);
    } else {
      c = 'N';
      o = 'A';
    }

    const char * crlf = "\r\n";
    const char * protocol1 = "HTTP/1.1 200 Document follows";
    const char * protocol2 = "Server: ServerType";
    const char * protocol3 = "Content-type: ";
    write(socket, protocol1, strlen(protocol1));
    write(socket, crlf, 2);
    write(socket, protocol2, strlen(protocol2));
    write(socket, crlf, 2);
    write(socket, protocol3, strlen(protocol3));
    write(socket, contentType, strlen(contentType));
    write(socket, crlf, 2);
    write(socket, crlf, 2);
    printf("content: %s\n", contentType);

    struct dirent * de;
    char ** names = (char **)malloc(sizeof(char *) * 1024);
    int count = 0;

    printf("cwd: %s\n", cwd);

    while ((de = readdir(dir)) != NULL) {
      printf("dname: %s\n", de->d_name);
      if (strcmp(de->d_name, ".") && strcmp(de->d_name, "..")) {
      printf("%s\n", filepath);
        names[count] = (char *)malloc(sizeof(char) * (strlen(de->d_name) + strlen(filepath)+256));
        //names[count][0] = '\0';
        strcat(names[count], filepath);
        if(strcmp(mysubstr(filepath, strlen(filepath)-1, strlen(filepath)), "/") != 0) {
          printf("379: %s\n", mysubstr(filepath, strlen(filepath)-1, strlen(filepath)));
          strcat(names[count], "/");
        }
        count++;
        strcat(names[count-1], de->d_name);
      }
    }



    const char * protocol4 = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 FINAL//EN\">\n<html>\n<head>\n<title></title>\n</head>\n<body>\n<h1>";
    const char * protocol5 = "</h1>\n<table>\n<tr><th valign=\"top\"><img src=\"/icons/blank.gif\" alt=\"[ICO]\"></th>";
    write(socket, protocol4, strlen(protocol4));
    write(socket, "Index of ", strlen("Index of "));
    write(socket, filepath, strlen(filepath));
    write(socket, protocol5, strlen(protocol5));


    const char * protocol13 = "<th><a href=\"?C=N;O=D\">Name</a></th>";
    write(socket, protocol13, strlen(protocol13));

    const char * protocol6 = "<th><a href=\"?C=M;O=A\">Last modified</a></th>";
    write(socket, protocol6, strlen(protocol6));
     // }
     // if (c == 'S') {
    const char * protocol7 = "<th><a href=\"?C=S;O=A\">Size</a></th>";
    write(socket, protocol7, strlen(protocol7));
   //   }
    //  if (c == 'D') {
    const char * protocol8 = "<th><a href=\"?C=D;O=A\">Description</a></th>";
    write(socket, protocol8, strlen(protocol8));
     // }
   // } else {
   //   if (c == 'N') {
    //    const char * protocol13 = "<th><a href=\"?C=N;O=D\">Name</a></th>";
    //    write(socket, protocol13, strlen(protocol13));
   //   }
   //   if (c == 'M') {
   //     const char * protocol10 = "<th><a href=\"?C=M;O=D\">Last modified</a></th>";
   //     write(socket, protocol10, strlen(protocol10));
   //   }
    //  if (c == 'S') {
    //    const char * protocol11 = "<th><a href=\"?C=S;O=D\">Size</a></th>";
    //    write(socket, protocol11, strlen(protocol11));
   //   }
   //   if (c == 'D') {
   //     const char * protocol12 = "<th><a href=\"?C=D;O=D\">Description</a></th>";
    //    write(socket, protocol12, strlen(protocol12));
   //   }
  //  }*/

    if (c == 'M') {
      if (o == 'A') {
        qsort(names, count, sizeof(char *), sortta);
      }
      if (o == 'D') {
        qsort(names, count, sizeof(char *), sorttd);
      }
    } else if (c == 'S') {
      if (o == 'A') {
        qsort(names, count, sizeof(char *), sortsa);
      }
      if (o == 'D') {
        qsort(names, count, sizeof(char *), sortsd);
      }
    } else {
      if (o == 'D') {
        qsort(names, count, sizeof(char *), sortnd);
      } else {
        qsort(names, count, sizeof(char *), sortna);
      }
    }


    const char * protocol14 = "</tr><tr><th colspan=\"5\"><hr></th></tr>\n";
    write(socket, protocol14, strlen(protocol14));

    char * parent = (char *)malloc(sizeof(char) * 1024);
    strcpy(parent, docpath);
    printf("parent: %s\n", parent);

    printf("mysubstr: %s\n",mysubstr(filepath, strlen(filepath)-1, strlen(filepath)));
    printf("docpath: %s\n", docpath);

    if (strcmp(mysubstr(filepath, strlen(filepath)-1, strlen(filepath)), "/") != 0) {
      printf("466: %s\n", mysubstr(filepath, strlen(filepath)-1, strlen(filepath)));
      strcat(parent, "/.\0");
    } else {
      strcat(parent, ".\0");
    }
    strcat(parent, ".");

    const char * protocol15 = "<tr><td valign=\"top\"><img src=\"";
    write(socket, protocol15, strlen(protocol15));

    writedir(socket, parent, parentd, dirs, docpath);
    printf("new parent: %s\n", parent);

    for (int i = 0; i < count; i++) {
      char * files = (char *)malloc(sizeof(char) * strlen(names[i]));
      int begin = 0;

      for (int j = strlen(names[i])-1; j >= 0; j--) {
        if (names[i][j] == '/') {
          begin = j;
          break;
        }
      }
      for (int k = begin+1; k < strlen(names[i]); k++) {
        files[k-begin-1] = names[i][k];
        if (k == strlen(names[i])-1) {
          files[k-begin] = '\0';
        }
      }

      char * type;
      if (opendir(names[i]) != NULL) {
        type = "DIR";
      } else {
        type = "";
      }
      const char * protocol15 = "</tr><td valign=\"top\"><img src=\"";
      write(socket, protocol15, strlen(protocol15));
      printf("FFFFFFFFFFFFFFFF: %s\n", docpath);
      writedir(socket, names[i], files, type, docpath);


    }
    closedir(dir);

  } else if (strstr(filepath, "cgi-bin")) {

    initial = strchr(filepath, '?');

    char newpath[256];
    strcpy(newpath, filepath);
    if (initial) {
      *initial = '?';
    }

    if (!open(newpath, O_RDONLY, 0644)) {
      const char * crlf = "\r\n";
      const char * notFound = "File not Found";
      const char * protocol1 = "HTTP/1.1 404FileNotFound";
      const char * protocol2 = "Server: ServerType";
      const char * protocol3 = "Content-type: ";
      strcpy(contentType, "text/plain");
      write(socket, protocol1, strlen(protocol1));
      write(socket, crlf, 2);
      write(socket, protocol2, strlen(protocol2));
      write(socket, crlf, 2);
      write(socket, protocol3, strlen(protocol3));
      write(socket, contentType, strlen(contentType));
      write(socket, crlf, 2);
      write(socket, crlf, 2);
      write(socket, notFound, strlen(notFound));

    } else {
      char ** string = (char **)malloc(sizeof(char *)*2);
      string[0] = (char *)malloc(sizeof(char) * strlen(filepath));
      string[1] = NULL;

      initial = strchr(filepath, '?');
      if (initial != 0) {
        initial++;
        strcpy(string[0], initial);
      }
      const char * crlf = "\r\n";
      const char * protocol1 = "HTTP/1.1 200 Document follows";
      const char * protocol2 = "Server: ServerType";
      write(socket, protocol1, strlen(protocol1));
      write(socket, crlf, 2);
      write(socket, protocol2, strlen(protocol2));
      write(socket, crlf, 2);

      //printf("HERERERERERERERERE\n");

      if (mysubstr(".so", strlen(newpath)-3, strlen(newpath)) == 0) {
        void * handle = dlopen(newpath, RTLD_LAZY);
        if (handle == NULL) {
          perror("dlopen");
        }
        httprunfunc hrun = (httprunfunc)dlsym(handle, "httprun");
        if (hrun == NULL) {
          fprintf(stderr, ".so not found\n");
          perror("dlysm");
          exit(1);
        }
        hrun(socket, string[0]);

      } else {
     printf("HERERERERERERERERE\n");
        int out = dup(1);
        dup2(socket, 1);
        shutdown(socket, SHUT_RDWR);
        sleep(1);
        close(socket);

        pid_t ret = fork();
        if (ret == 0) {
          setenv("REQUEST_METHOD", "GET", 1);
          setenv("QUERY_STRING", string[0], 1);
          execvp(newpath, string);
          exit(2);
        }

    /*  int fd = open(newpath, O_RDONLY, 0644);
      printf("CCCCCCCCCCCCCC: %s\n", cwd);
      char buf[1111111];
      int i;
      while (i = read(fd, buf, 1111111)) {
        if(write(socket, buf, i) != i) {
          perror("write");
          break;
        }
       //write(socket, &buf, 1);

      }
      close(fd);*/
        dup2(out, 1);
        close(out);
      }
    }

  } else if (strcmp(mysubstr(filepath, strlen(filepath)-5, strlen(filepath)), "stats") == 0) {

      strcpy(contentType, "text/plain");
      const char * crlf = "\r\n";
      const char * protocol1 = "HTTP/1.1 200 Document follows";
      const char * protocol2 = "Server: ServerType";
      const char * protocol3 = "Content-type: ";
      write(socket, protocol1, strlen(protocol1));
      write(socket, crlf, 2);
      write(socket, protocol2, strlen(protocol2));
      write(socket, crlf, 2);
      write(socket, protocol3, strlen(protocol3));
      write(socket, contentType, strlen(contentType));
      write(socket, crlf, 2);
      write(socket, crlf, 2);

      write(socket, "Juha Jeon\n", strlen("Juha Jeon\n"));

      write(socket, stime, strlen(stime));
      write(socket, "\n", 1);

      std::string s = std::to_string(requests);
      const char * request = s.c_str();
      write(socket, request, strlen(request));
      write(socket, "\n", 1);

      std::stringstream ss;
      ss << mint;
      const char * mintime = ss.str().c_str();
      printf("HOWE\n");
      printf("MINT: %f\n", mint);
      printf("mintime: %s\n", mintime);
      write(socket, mintime, strlen(mintime));
      printf("HI\n");
      write(socket, "\n", 1);
      write(socket, minpath, strlen(minpath));
      printf("HHHI\n");
      write(socket, "\n", 1);

      std::stringstream sss;
      sss << maxt;
      const char * maxtime = sss.str().c_str();
      printf("HOWE\n");
      printf("MAXT: %f\n", maxt);
      write(socket, maxtime, strlen(maxtime));
      write(socket, "\n", 1);
      write(socket, maxpath, strlen(maxpath));
      write(socket, "\n", 1);

  } else {


    if (strcmp(mysubstr(filepath, strlen(filepath)-5, strlen(filepath)), ".html") == 0 ) {
      strcpy(contentType, "text/html");
    } else if (strcmp(mysubstr(filepath, strlen(filepath)-4, strlen(filepath)), ".gif") == 0) {
      strcpy(contentType, "image/gif");
    } else if (strcmp(mysubstr(filepath, strlen(filepath)-4, strlen(filepath)), ".svg") == 0 ) {
      strcpy(contentType, "image/svg+xml");
    //printf("svg: %s\n" ,mysubstr(filepath, strlen(filepath)-4, strlen(filepath)));
    } else if (strcmp(mysubstr(filepath, strlen(filepath)-4, strlen(filepath)), ".xbm") == 0 ) {
      strcpy(contentType, "image/x-xbitmap");
      printf("SSSSSSSSSSXXXXXXXXXXXXX\n");
    } else {
      strcpy(contentType, "text/plain");
    }

    int fd = open(filepath, O_RDWR);

    printf("xxxxxxxxxxxxxxxxxxxx: %d\n", x);

    if (fd < 0 || x == 1) {

      const char * crlf = "\r\n";
      const char * notFound = "File not Found";
      const char * protocol1 = "HTTP/1.1 404FileNotFound";
      const char * protocol2 = "Server: ServerType";
      const char * protocol3 = "Content-type: ";
      write(socket, protocol1, strlen(protocol1));
      write(socket, crlf, 2);
      write(socket, protocol2, strlen(protocol2));
      write(socket, crlf, 2);
      write(socket, protocol3, strlen(protocol3));
      write(socket, contentType, strlen(contentType));
      write(socket, crlf, 2);
      write(socket, crlf, 2);
      write(socket, notFound, strlen(notFound));

    } else {
      const char * crlf = "\r\n";
      const char * protocol1 = "HTTP/1.1 200 Document follows";
      const char * protocol2 = "Server: ServerType";
      const char * protocol3 = "Content-type: ";
      write(socket, protocol1, strlen(protocol1));
      write(socket, crlf, 2);
      write(socket, protocol2, strlen(protocol2));
      write(socket, crlf, 2);
      write(socket, protocol3, strlen(protocol3));
      write(socket, contentType, strlen(contentType));
      write(socket, crlf, 2);
      write(socket, crlf, 2);

      char buf[1111111];
      int i;
      while (i = read(fd, buf, 1111111)) {
        if(write(socket, buf, i) != i) {
          perror("write");
          break;
        }
      // write(socket, &buf, 1);

      }
    }
    printf("OOOOOOOOOOOOOOOOO: %s\n", filepath);

    close(fd);
  }

  if (strcmp(mysubstr(filepath, strlen(filepath)-5, strlen(filepath)), "stats") != 0 ) {
    et = clock();
    nt = (double)(et - nt);
    if (nt < mint) {
      mint = nt;
      minpath = filepath;
    }
    if (nt > maxt) {
      maxt = nt;
      maxpath = filepath;
    }
  }

}

char * mysubstr(char * str, int begin, int length)
{
  char * result = new char[length+1];
  for (int i = 0; i < length; i++) {
    result[i] = *(str + begin + i);
  }
  result[length] = 0;
  return result;
}

void poolSlave(int socket)
{
  while(1) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_lock(&mutex);
    struct sockaddr_in clientIPAdress;
    int alen = sizeof(clientIPAdress);
    int slaveSocket = accept(socket, (struct sockaddr *)&clientIPAdress, (socklen_t *)&alen);
    pthread_mutex_unlock(&mutex);
    if ( slaveSocket < 0 ) {
      perror( "accept" );
      exit( -1 );
    }
    processRequest(slaveSocket);
    shutdown(slaveSocket, SHUT_RDWR);
    close(slaveSocket);
    }
}

int sortna(const void *a, const void *b) {
  const char *aa = *(const char **)a;
  const char *bb = *(const char **)b;

  return strcmp(aa, bb);
}

int sortnd(const void *b, const void *a) {
  const char *aa = *(const char **)a;
  const char *bb = *(const char **)b;

  return strcmp(aa, bb);
}

int sortta(const void *a, const void *b) {
  const char *aa = *(const char **)a;
  const char *bb = *(const char **)b;

  struct stat saa;
  struct stat sbb;

  return difftime(saa.st_mtime, sbb.st_mtime);
}

int sorttd(const void *b, const void *a) {
  const char *aa = *(const char **)a;
  const char *bb = *(const char **)b;
  struct stat saa;
  struct stat sbb;

  stat(aa, &saa);
  stat(bb, &sbb);

  return difftime(saa.st_mtime, sbb.st_mtime);
}

int sortsa(const void *a, const void *b) {
  const char *aa = *(const char **)a;
  const char *bb = *(const char **)b;
  struct stat saa;
  struct stat sbb;

  stat(aa, &saa);
  stat(bb, &sbb);

  int first = saa.st_size;
  int second = sbb.st_size;

  return first - second;
}

int sortsd(const void *b, const void *a) {
  const char *aa = *(const char **)a;
  const char *bb = *(const char **)b;
  struct stat saa;
  struct stat sbb;

  stat(aa, &saa);
  stat(bb, &sbb);

  int first = saa.st_size;
  int second = sbb.st_size;

  return first - second;
}

void writedir(int socket, char * filepath, char * file, char * type, char * parent) {

  char * icon;
  char * alt = strdup(type);

  if (strcmp(file, parentd) != 0) {
    icon = "../icons/back.xbm";
  } else if (strcmp(type, dirs) != 0) {
    icon = "../icons/menu.xbm";
  } else {
    icon = "../icons/unknwon.xbm";
  }

  const char * protocol16 = "\" alt=\"[";
  const char * protocol17 = "]\"></td><td><a href= \"";
  write(socket, icon, strlen(icon));
  write(socket, protocol16, strlen(protocol16));
  write(socket, alt, strlen(alt));
  write(socket, protocol17, strlen(protocol17));

  if (strcmp(file, parentd)) {

    if (strcmp(mysubstr(parent, strlen(parent)-1, strlen(parent)), "/") != 0) {
      printf("file: %s\n", file);
      printf("696: %s\n", mysubstr(parent, strlen(parent)-1, strlen(parent)));
      char * parents = (char *)malloc(sizeof(char) * 1024);
      
      strcat(parents, parent);
      strcat(parents, "/");
      strcat(parents, file);
      write(socket, parents, strlen(parents));

    } else {
      write(socket, file, strlen(file));
    }

  } else {
    write(socket, filepath, strlen(filepath));
  }

  const char * protocol18 = "\">";
  const char * protocol19 = "</a>";
  write(socket, protocol18, strlen(protocol18));
  write(socket, file, strlen(file));
  write(socket, protocol19, strlen(protocol19));

  char * time = (char *)malloc(sizeof(char) * 64);
  
  int size = 0;

  if (!strcmp(file, parentd)) {
    const char * protocol20 = "</td><td>&nbsp;";
    write(socket, protocol20, strlen(protocol20));
  
  } else {
    struct stat fs;
    stat(filepath, &fs);
    struct tm *tm;
    tm = localtime(&(fs.st_mtime));
    strftime(time, 20, "%F %H:%M", tm);
    size = fs.st_size;
  }
  const char * protocol22 = "</td><td align=\"right\">";
  write(socket, protocol22, strlen(protocol22));
  write(socket, time, strlen(time));

  if (strcmp(file, parentd)) {
    write(socket, protocol22, strlen(protocol22));

    if (strcmp(type, dirs)) {
      char * sized = (char *)malloc(sizeof(char) * 64);
      snprintf(sized, 65, "%d", size);
      write(socket, sized, strlen(sized));
    } else {
      write(socket, "-", strlen("-"));
    }
  }
    const char * protocol21 = " </td><td>&nbsp;</td></tr>\n";
    write(socket, protocol21, strlen(protocol21));

}


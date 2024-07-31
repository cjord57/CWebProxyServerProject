/*
 * proxy.c - CS:APP Web proxy
 *
 * Corey Jordan-Zamora 
 *
 * My code implements a simple web proxy server by:
 * Initializing necessary variables, such as file pointers, semaphores, and global variables. The main function list\
ens for client connections continuously, which, upon receiving a connection, it creates a new thread to handle the H\
TTP request. The process_request thread function reads the request from the client, forwards it to the end server, w\
aits for the response, and forwards it back to the client. This function does most of the work, as it is the one tha\
t handles the parsing of the HTTP request, extracts the URI, opens a connection to the end server, forwards the requ\
est, receives the response, logs the request to a file, and cleans up resources, so one can see just how vital this \
process_request function is for implementing a proxy. Some helper functions are implemented as well to of course, as\
sist in reading and writing data, parsing URIs, opening client connections, and formatting log entries [which is als\
o looked over as part of this lab to see records of the properly-established connections with my proxy].
 * So overall, my code serves as a basic proxy server, facilitating communciation between clients and serers, and lo\
gs those requests so one can see that the connections have been properly established.
 */

#include "csapp.h"

/* The name of the proxy's log file */
#define PROXY_LOG "proxy.log"

/* Undefine this if you don't want debugging output */
#define DEBUG

/*
 * This struct remembers some key attributes of an HTTP request and
 * the thread that is processing it.
 */
typedef struct {
    int myid;    /* used to identify threads in debug messages */
    int connfd;                    /* Connected file descriptor */
    struct sockaddr_in clientaddr; /* Client IP address */
} arglist_t;

/*
 * Globals
 */
FILE *log_file; /* Log file with one line per HTTP request */
sem_t mutex;    /* Semaphore for when mutual exclusion is needed */

/*
 * Functions needed
 */
void *process_request(void* vargp);
int open_clientfd_ts(char *hostname, int port, sem_t *mutexp);
ssize_t Rio_readn_w(int fd, void *ptr, size_t nbytes);
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen);
void Rio_writen_w(int fd, void *usrbuf, size_t n);
int parse_uri(char *uri, char *target_addr, char *path, int  *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);

/*
 * main - Main routine for the proxy program
 */
int main(int argc, char **argv)
{
    int listenfd;             /* proxy's listening descriptor */
    int port; // Me // Added it. Declaring it here so that the port = atoi(argv[1]) line can access it
    pthread_t tid;            /* Pthread thread id */
    unsigned int clientlen;   /* Size in bytes of the client socket address */
    arglist_t *argp = NULL;   /* Argument struct passed to a thread */
    int request_count = 0;    /* Number of requests received so far */

    /* Check arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(0);
    }

    /*
     * Ignore any SIGPIPE signals elicited by writing to a connection
     * that has already been closed by the peer process.
     */
    signal(SIGPIPE, SIG_IGN); // Me

    /* Create a listening descriptor */
    port = atoi(argv[1]); // Me
    char port_str[6]; // Me // Added it since I initially declared port as int in order to work for the prev line. S\
o now it is a char so that the code in the rest of file can work with it. // Maximum length of port string is 5 digi\
ts plus null terminator
    snprintf(port_str, sizeof(port_str), "%d", port); // Me // Added it
    //listenfd = Open_listenfd(port);
    listenfd = Open_listenfd(port_str); // Me // I commented out line above and did this one instead

    /* Inititialize */
    log_file = Fopen(PROXY_LOG, "a");
    Sem_init(&mutex, 0, 1); /* mutex = 1 initially */ // Me

    /* Wait for and process client connections */
    while (1) {
        argp = (arglist_t *)Malloc(sizeof(arglist_t));
        clientlen = sizeof(argp->clientaddr);
        argp->connfd =
          Accept(listenfd, (SA *)&argp->clientaddr, &clientlen); // Me

        /* Start a new thread to process the HTTP request */
        argp->myid = request_count++;
        //pthread_create(&tid, NULL, process_request, (void *)argp); // Me
        pthread_create(&tid, NULL, process_request, argp); // Me // Used this one instead
    }

    /* Control never reaches here */
    exit(0);
}

/*
 * process_request - Thread routine.
 *
 * Each thread reads an HTTP request from a client, forwards it to the
 * end server (always as a simple HTTP/1.0 request), waits for the
 * response, and then forwards it back to the client.
 *
 * Note: this function is longer than usual, but
 * having everything in one function greatly simplifies error
 * handling, in particualar cleaning up after errors so as to avoid
 * memory leaks.
 */
void *process_request(void *vargp)
{
    arglist_t arglist;              /* Arg list passed into thread */
    struct sockaddr_in clientaddr;  /* Client socket address */
    int connfd;                     /* Socket descriptor for talking with client */
    int serverfd;                   /* Socket descriptor for talking with end server */
    char *request;                  /* HTTP request from client */
    char *request_uri;              /* Start of URI in first HTTP request header line */
    char *request_uri_end;          /* End of URI in first HTTP request header line */
    char *rest_of_request;          /* Beginning of second HTTP request header line */
    int request_len;                /* Total size of HTTP request */
    int response_len;               /* Total size in bytes of response from end server */
    int i, n;                       /* General index and counting variables */
    int realloc_factor;             /* Used to increase size of request buffer if necessary */

    char hostname[MAXLINE];         /* Hostname extracted from request URI */
    char pathname[MAXLINE];         /* Content pathname extracted from request URI */
    int port;                       /* Port number extracted from request URI (default 80) */
    char log_entry[MAXLINE];        /* Formatted log entry */

    rio_t rio;                      /* Rio buffer for calls to buffered rio_readlineb routine */
    char buf[MAXLINE];              /* General I/O buffer */

    /* Do some initial setup */
    arglist = *((arglist_t *)vargp); /* Copy the arguments onto the stack */
    connfd = arglist.connfd;         /* Put connfd and clientaddr in scalars for convenience */
    clientaddr = arglist.clientaddr;
    Pthread_detach(pthread_self());  /* Detach the thread */ // Me
    Free(vargp);                     /* Free up the arguments */ // Me

    /*
     * Read the entire HTTP request into the request buffer, one line
     * at a time.
     */
    request = (char *)Malloc(MAXLINE);
    request[0] = '\0';
    realloc_factor = 2;
    request_len = 0;
    Rio_readinitb(&rio, connfd); // Me
    while (1) {
      if ((n = Rio_readlineb_w(&rio, buf, MAXLINE)) <= 0) { // Me
            printf("process_request: client issued a bad request (1).\n");
            close(connfd); // Me
            free(request); // Me
            return NULL;
        }

        /* If not enough room in request buffer, make more room */
        if (request_len + n + 1 > MAXLINE)
            Realloc(request, MAXLINE*realloc_factor++);

        strcat(request, buf);
        request_len += n;

        /* An HTTP requests is always terminated by a blank line */
        if (strcmp(buf, "\r\n") == 0)
            break;
    }

#if defined(DEBUG)
    {
        struct hostent *hp;
        char *haddrp;

        P(&mutex); // Me
        hp = Gethostbyaddr((char *)&clientaddr.sin_addr.s_addr,
                           sizeof(clientaddr.sin_addr.s_addr),
                           AF_INET);
        haddrp = inet_ntoa(clientaddr.sin_addr);
        printf("Thread %d: Received request from %s (%s):\n", arglist.myid,
               hp->h_name, haddrp);
        printf("%s", request);
        printf("*** End of Request ***\n");
        printf("\n");
        fflush(stdout);
        V(&mutex); // Me
    }
#endif

    /*
     * Make sure that this is indeed a GET request
     */
    if (strncmp(request, "GET ", strlen("GET "))) {
        printf("process_request: Received non-GET request\n");
        close(connfd); // Me
        free(request); // Me
        return NULL;
    }
    request_uri = request + 4;

    /*
     * Extract the URI from the request
     */
    request_uri_end = NULL;
    for (i = 0; i < request_len; i++) {
        if (request_uri[i] == ' ') {
            request_uri[i] = '\0';
            request_uri_end = &request_uri[i];
            break;
        }
    }

    /*
     * If we hit the end of the request without encountering a
     * terminating blank, then there is something screwy with the
     * request
     */
    if ( i == request_len ) {
        printf("process_request: Couldn't find the end of the URI\n");
        close(connfd); // Me
        free(request); // Me
        return NULL;
    }

    /*
     * Make sure that the HTTP version field follows the URI
     */
    if (strncmp(request_uri_end + 1, "HTTP/1.1\r\n", strlen("HTTP/1.1\r\n"))) {
        printf("process_request: client issued a bad request (4).\n");
        close(connfd); // Me
        free(request); // Me
        return NULL;
    }

    /*
     * We'll be forwarding the remaining lines in the request
     * to the end server without modification
     */
    rest_of_request = request_uri_end + strlen("HTTP/1.1\r\n") + 1;

    /*
     * Parse the URI into its hostname, pathname, and port components.
     * Since the recipient is a proxy, the browser will always send
     * a URI consisting of a full URL "http://hostname:port/pathname"
     */
    if (parse_uri(request_uri, hostname, pathname, &port) < 0) {
        printf("process_request: cannot parse uri\n");
        close(connfd); // Me
        free(request); // Me
        return NULL;
    }

    /*
     * Forward the request to the end server
     */
    if ((serverfd = open_clientfd_ts(hostname, port, &mutex)) < 0) { // Me
        printf("process_request: Unable to connect to end server.\n");
        free(request); // Me
        return NULL;
    }
    Rio_writen_w(serverfd, "GET /", strlen("GET /"));
    Rio_writen_w(serverfd, pathname, strlen(pathname));
    Rio_writen_w(serverfd, " HTTP/1.0\r\n", strlen(" HTTP/1.0\r\n"));
    Rio_writen_w(serverfd, rest_of_request, strlen(rest_of_request));

#if defined(DEBUG)
    P(&mutex); // Me
    printf("Thread %d: Forwarding request to end server:\n", arglist.myid);
    printf("GET /%s HTTP/1.0\r\n%s", pathname, rest_of_request);
    printf("*** End of Request ***\n");
    printf("\n");
    fflush(stdout);
    V(&mutex); // Me
#endif

    /*
     * Receive reply from server and forward on to client
     */
    Rio_readinitb(&rio, serverfd); // Me
    response_len = 0;
    while( (n = Rio_readn_w(serverfd, buf, MAXLINE)) > 0 ) { // Me
        response_len += n;
        Rio_writen_w(connfd, buf, n); // Me
#if defined(DEBUG)
        printf("Thread %d: Forwarded %d bytes from end server to client\n", arglist.myid, n);
        fflush(stdout);
#endif
        bzero(buf, MAXLINE);
    }

    /*
     * Log the request to disk
     */
    format_log_entry(log_entry, &clientaddr, request_uri, response_len);
    P(&mutex); // Me
    fprintf(log_file, "%s %d\n", log_entry, response_len);
    fflush(log_file);
    V(&mutex); // Me

    /* Clean up to avoid memory leaks and then return */
    close(connfd); // Me
    close(serverfd); // Me
    free(request); // Me
    return NULL;
}

/*
 * Rio_readn_w - A wrapper function for rio_readn (csapp.c) that
 * prints a warning message when a read fails instead of terminating
 * the process.
 */
ssize_t Rio_readn_w(int fd, void *ptr, size_t nbytes)
{
    ssize_t n;

    if ((n = rio_readn(fd, ptr, nbytes)) < 0) {
        printf("Warning: rio_readn failed\n");
        return 0;
    }
    return n;
}

/*
 * Rio_readlineb_w - A wrapper for rio_readlineb (csapp.c) that
 * prints a warning when a read fails instead of terminating
 * the process.
 */
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen)
{
    ssize_t rc;

    if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0) {
        printf("Warning: rio_readlineb failed\n");
        return 0;
    }
    return rc;
}

/*
 * Rio_writen_w - A wrapper function for rio_writen (csapp.c) that
 * prints a warning when a write fails, instead of terminating the
 * process.
 */
void Rio_writen_w(int fd, void *usrbuf, size_t n)
{
    if (rio_writen(fd, usrbuf, n) != n) {
        printf("Warning: rio_writen failed.\n");
   }
}

/*
 * open_clientfd_ts - A thread safe version of the open_clientfd
 * function (csapp.c) that uses the lock-and-copy technique to deal
 * with the Class 3 thread unsafe gethostbyname function.
 */
int open_clientfd_ts(char *hostname, int port, sem_t *mutexp)
{
    int clientfd;
    struct hostent hostent, *hp = &hostent;
    struct hostent *temp_hp;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1; /* check errno for cause of error */

    /* Use lock-and-copy to keep this function thread safe */
    P(mutexp); /* lock */ // Me
    temp_hp = gethostbyname(hostname);
    if (temp_hp != NULL)
        hostent = *temp_hp; /* copy */
    V(mutexp); // Me

    /* Fill in the server's IP address and port */
    if (temp_hp == NULL)
        return -2; /* check h_errno for cause of error */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
   bcopy((char *)hp->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(port);

    /* Establish a connection with the server */
    if (connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    return clientfd;
}

/*
 * parse_uri - URI parser
 *
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, int *port)
{
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;

    if (strncasecmp(uri, "http://", 7) != 0) {
        hostname[0] = '\0';
        return -1;
    }

    /* Extract the host name */
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';

    /* Extract the port number */
    *port = 80; /* default */
    if (*hostend == ':')
        *port = atoi(hostend + 1);

    /* Extract the path */
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL) {
        pathname[0] = '\0';
    }
    else {
        pathbegin++;
        strcpy(pathname, pathbegin);
    }

    return 0;
}

/*
 * format_log_entry - Create a formatted log entry in logstring.
 *
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr,
                      char *uri, int size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /*
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;


    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s", time_str, a, b, c, d, uri);
}

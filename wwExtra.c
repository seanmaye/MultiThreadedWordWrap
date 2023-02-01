#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

#define BUFSIZE 256
#define DEBUG 0

DIR *fdopendir(int fd);
int error;
int width;

int align(int toAlign, int ifd, int ofd)
{
    if (ifd == -1)
    {
        perror("ERROR BAD FILE DESCRIPTOR");
        return (EXIT_FAILURE);
    }

    char buf[BUFSIZE];
    int bytesRead;

    char *wordArray = malloc(BUFSIZE * sizeof(char)); // contains word to be put into line
    int wordIndex = 0;
    char spaceArray[1]; // array of spaces
    spaceArray[0] = ' ';

    char newLineArray[2]; // array of newLines
    newLineArray[0] = '\n';
    newLineArray[1] = '\n';

    int lineCounter = 0;
    int newLineCounter = 0;
    int exitValue = EXIT_SUCCESS;

    while ((bytesRead = read(ifd, buf, BUFSIZE)) > 0)
    {
        for (int i = 0; i < bytesRead; i++)
        {
            if (isspace(buf[i]) == 0) // not space
            {
                wordArray[wordIndex] = buf[i];
                wordIndex++;
                newLineCounter = 0;
            }
            else
            {
                if (buf[i] == '\n')
                {
                    newLineCounter++;
                }
                if (newLineCounter == 2)
                {
                    write(ofd, newLineArray, 2);
                    lineCounter = 0;
                    newLineCounter = 0;
                }
                if (wordIndex > 0)
                {
                    if (wordIndex > toAlign && lineCounter == 0) // word is too big and is at start of line
                    {
                        exitValue = EXIT_FAILURE;
                        write(ofd, wordArray, wordIndex);
                        write(ofd, newLineArray, 1);
                        lineCounter = 0;
                        if (DEBUG == 1)
                        {
                            printf("\nlinecounter:%d wordIndex:%d\nCASE1\n", lineCounter, wordIndex);
                        }
                    }
                    else if (wordIndex > toAlign && lineCounter > 0) // word is too big and is at some point in line
                    {
                        exitValue = EXIT_FAILURE;
                        write(ofd, newLineArray, 1);
                        write(ofd, wordArray, wordIndex);
                        write(ofd, newLineArray, 1);
                        lineCounter = 0;
                        if (DEBUG == 1)
                        {
                            printf("\nlinecounter:%d wordIndex:%d\nCASE2\n", lineCounter, wordIndex);
                        }
                    }
                    else if (lineCounter + wordIndex > toAlign) // new word cannot fit on current line
                    {
                        write(ofd, newLineArray, 1);
                        write(ofd, wordArray, wordIndex);
                        lineCounter = wordIndex + 1;
                        if (DEBUG == 1)
                        {
                            printf("\nlinecounter:%d wordIndex:%d\nCASE3 \n", lineCounter, wordIndex);
                        }
                    }
                    else if (lineCounter + wordIndex == toAlign) // new word fits prefectly on line
                    {
                        if (lineCounter != 0) // not the start of a line
                        {
                            write(ofd, spaceArray, 1);
                        }
                        write(ofd, wordArray, wordIndex);
                        write(ofd, newLineArray, 1);
                        lineCounter = 0;
                        if (DEBUG == 1)
                        {
                            printf("\nlinecounter:%d wordIndex:%d\nCASE4 \n", lineCounter, wordIndex);
                        }
                    }
                    else if (lineCounter + wordIndex < toAlign) // new word fits on the line
                    {
                        if (lineCounter != 0) // not the start of a line
                        {
                            write(ofd, spaceArray, 1);
                        }
                        write(ofd, wordArray, wordIndex);
                        lineCounter += wordIndex;
                        lineCounter++;
                        if (DEBUG == 1)
                        {
                            printf("\nlinecounter:%d wordIndex:%d\nCASE5 \n", lineCounter, wordIndex);
                        }
                    }
                    else // nothing applies
                    {
                        perror("ERROR NOTHING APPLIES");
                        return (EXIT_FAILURE);
                    }
                    wordIndex = 0;
                }
            }
        }
    }

    // we still have a word in the word array and we need to add it accordingly
    // the following code is a repeat of previous code
    if (newLineCounter == 1)
    {
        // file ends with newline and nothing should be printed
    }
    else
    {
        if (wordIndex > toAlign && lineCounter == 0)
        {
            exitValue = EXIT_FAILURE;
            write(ofd, wordArray, wordIndex);
            write(ofd, newLineArray, 1);
            lineCounter = 0;
            if (DEBUG == 1)
            {
                printf("\nlinecounter:%d wordIndex:%d\nCASE1\n", lineCounter, wordIndex);
            }
        }
        else if (wordIndex > toAlign && lineCounter > 0)
        {
            exitValue = EXIT_FAILURE;
            write(ofd, newLineArray, 1);
            write(ofd, wordArray, wordIndex);
            write(ofd, newLineArray, 1);
            lineCounter = 0;
            if (DEBUG == 1)
            {
                printf("\nlinecounter:%d wordIndex:%d\nCASE2\n", lineCounter, wordIndex);
            }
        }
        else if (lineCounter + wordIndex > toAlign)
        {
            write(ofd, newLineArray, 1);
            write(ofd, wordArray, wordIndex);
            lineCounter = wordIndex;
            if (DEBUG == 1)
            {
                printf("\nlinecounter:%d wordIndex:%d\nCASE3 \n", lineCounter, wordIndex);
            }
        }
        else if (lineCounter + wordIndex == toAlign)
        {
            if (lineCounter != 0)
            {
                write(ofd, spaceArray, 1);
            }

            write(ofd, wordArray, wordIndex);
            write(ofd, newLineArray, 1);
            lineCounter = 0;
            if (DEBUG == 1)
            {
                printf("\nlinecounter:%d wordIndex:%d\nCASE4 \n", lineCounter, wordIndex);
            }
        }
        else if (lineCounter + wordIndex < toAlign)
        {
            if (lineCounter != 0) // not the start of a line
            {
                write(ofd, spaceArray, 1);
            }
            write(ofd, wordArray, wordIndex);
            lineCounter += wordIndex;
            if (DEBUG == 1)
            {
                printf("\nlinecounter:%d wordIndex:%d\nCASE5 bot\n", lineCounter, wordIndex);
            }
        }
    }

    // we are now done with the repeat code

    free(wordArray);
    return exitValue;
}

struct node
{
    char *filename;
    struct node *next;
};
typedef struct node node;

struct queue
{
    int state; // open of close
    int size;
    node *first, *last, *head;
    pthread_mutex_t lock;
    pthread_cond_t enqueue_ready, dequeue_ready;
};
typedef struct queue queue;

struct params
{
    queue *directory;
    queue *file;
};
typedef struct params params;

void initq(queue *q)
{
    q->size = 0;
    q->first = NULL;
    q->last = NULL;
    q->head = NULL;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->enqueue_ready, NULL);
    pthread_cond_init(&q->dequeue_ready, NULL);
    
}

void enqueue(queue *q, char *name)
{
    pthread_mutex_lock(&q->lock);
    node *new = malloc(sizeof(node)); // node to add
    new->filename = name;
    new->next = NULL;

    if (q->size == 0)
    {
        q->first = new;
        q->last = new;
        q->head = new;
    }
    else
    {
        q->last->next = new;
        q->last = new;
    }

    q->size++;

    pthread_mutex_unlock(&q->lock);
}

char *dequeue(queue *q)
{
    pthread_mutex_lock(&q->lock);
    if (q->size != 0)
    {
        node *r; // node to remove
        char *name = q->first->filename;

        r = q->first;
        q->first = q->first->next;
        q->size--;
        free(r);
        if (DEBUG == 2)
        {
            printf("Current File Name is %s\n", name);
        }
        pthread_mutex_unlock(&q->lock);
        return (name);
    }
    else
    {
        if(DEBUG==1){
        printf("TRYING TO DEQUEUE WHEN NOTHING IS INSIDE BOZO");
        }
        pthread_mutex_unlock(&q->lock);
        return (NULL);
    }
}

void *lineThread(void *vargp)
{
    queue *q = (queue *)vargp;
    while (q->size != 0)
    {
        char *reg = (char *)dequeue(q);
        if (reg == NULL)
        {
            return NULL;
        }

        char *str3 = reg;
        char *str = (char *)malloc(strlen(str3) + 1);
        char *str2 = (char *)malloc(strlen(str3) + 1);
        memcpy(str, str3, strlen(str3) + 1);
        memcpy(str2, str3, strlen(str3) + 1);

        char *token = strtok(str, "/");
        char *final;
        char wrap[] = "wrap";
        // get file name
        while ((token = strtok(NULL, "/")))
        {
            final = token;
        }

        // concat file and wrap
        int wlen = strlen(wrap);
        int flen = strlen(final);
        char *newpath = (char *)malloc(wlen + flen + 2);
        memcpy(newpath, wrap, wlen);
        newpath[wlen] = '.';
        memcpy(newpath + wlen + 1, final, flen + 1);

        // get path
        int plen = strlen(str2) - flen;
        char *path = (char *)malloc(plen);
        memcpy(path, str2, plen - 1);
        path[plen - 1] = '\0';
        if (DEBUG == 2)
        {
            printf("%s\n", path);
        }
        char *newFile = newpath;
        if (DEBUG == 2)
        {
            printf("%s\n", newFile);
        }
        plen = strlen(path);
        flen = strlen(newFile);

        // concat file and path
        char *finalPath = (char *)malloc(plen + flen + 2);
        memcpy(finalPath, path, plen);
        finalPath[plen] = '/';
        memcpy(finalPath + plen + 1, newFile, flen + 1);

        if (DEBUG == 2)
        {
            printf("final Path: %s\n", finalPath);
        }

        int fd = open(reg, O_RDONLY, S_IRWXU);

        int od = open(finalPath, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
        int temp=0;
        temp = align(width, fd, od);
        if (temp == 1)
        {
            error = 1;
        }
        free(newpath);
        free(path);
        free(finalPath);
        free(str2);
        free(str);
        free(reg);
        close(fd);
        close(od);
        // pthread_mutex_unlock(&q->lock);
    }
    return NULL;
}

void display(node *head)
{
    if (head == NULL)
    {
        printf("end\n");
    }
    else
    {
        printf("%s\n", head->filename);
        display(head->next);
    }
}

void freeq(queue *q)
{
    while (q->size != 0)
    {
        char *path = dequeue(q);
        if (path != NULL)
        {
            free(path);
        }
    }
}

void *directoryTraversal(void *paramsp)
{
    params *queueArray = (params *)paramsp;
    queue *regQueue = queueArray->file;
    queue *dirQueue = queueArray->directory;

    while (dirQueue->size != 0)
    {
        char *path = (char *)dequeue(dirQueue);
        DIR *folder;
        // int dirError;
        struct dirent *entry;
        folder = opendir(path);

        if (folder == NULL)
        {
            perror("ERROR NO SUCH FOLDER");
            return (NULL);
        }
        while ((entry = readdir(folder)))
        {
            int isReg = 0;
            int isDir = 0;
            struct stat s;

            char *file = entry->d_name;
            int plen = strlen(path);
            int flen = strlen(file);
            char *newpath = (char *)malloc(plen + flen + 2);
            memcpy(newpath, path, plen);
            newpath[plen] = '/';
            memcpy(newpath + plen + 1, file, flen + 1);

            if (DEBUG == 2)
            {
                printf("File: %s, Path, %s, NewPath %s, \n", file, path, newpath);
            }
            stat(newpath, &s);
            if (S_ISREG(s.st_mode))
            {
                if (DEBUG == 2)
                {
                    printf("register!\n");
                }
                isReg = 1;
            }
            else if (S_ISDIR(s.st_mode))
            {
                if (DEBUG == 2)
                {
                    printf("directory!\n");
                }
                isDir = 1;
            }

            if (file[0] == '.')
            {
                // Case where we ignore file
                free(newpath);
            }
            else if (strncmp(file, "wrap.", 5) == 0)
            {
                // Case where we ignore file
                free(newpath);
            }
            else if (isReg == 1)
            {
                if (DEBUG == 2)
                {
                    printf("We enqueue a file: %s\n", newpath);
                }

                enqueue(regQueue, newpath);
            }
            else if (isDir == 1)
            {
                if (DEBUG == 2)
                {
                    printf("We enqueue a directory: %s\n", newpath);
                }
                enqueue(dirQueue, newpath);
            }
        }
        closedir(folder);
        free(path);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    error = 0;

    queue *dirQueue;
    queue *fileQueue;

    dirQueue = malloc(sizeof(queue));
    fileQueue = malloc(sizeof(queue));

    initq(dirQueue);
    initq(fileQueue);

    if (strncmp(argv[1], "-r", 2) == 0)
    {
        // do recursive traversal
        char *wrapType = argv[1];
        width = atoi(argv[2]);
        int dirThreads;
        int wrapThreads;
        int tokens[2] = {0, 0};
        char *token;
        token = strtok(wrapType, "-r,");
        int j = 0;
        while (token != NULL)
        {
            tokens[j] = atoi(token);
            j++;
            token = strtok(NULL, "-r,");
        }
        if (strlen(wrapType) == 2)
        {
            // part 1
            dirThreads = 1;
            wrapThreads = 1;
        }
        else if (strlen(wrapType) > 2 && tokens[1] == 0)
        {
            // part 2
            dirThreads = 1;
            wrapThreads = tokens[0];
        }
        else if (strlen(wrapType) > 2 && tokens[1] != 0)
        {
            // part 3
            dirThreads = tokens[0];
            wrapThreads = tokens[1];
        }
        for (int i = 3; i < argc; i++) // loop for recursive
        {
            char *regOrDir = malloc(sizeof(char) * strlen(argv[i]) + 1);
            memcpy(regOrDir, argv[i], strlen(argv[i]) + 1);

            struct stat s;
            stat(regOrDir, &s);
            if (S_ISDIR(s.st_mode))
            {
                enqueue(dirQueue, regOrDir);
            }
            else if (S_ISREG(s.st_mode))
            {
                enqueue(fileQueue, regOrDir);
            }
        }
        params *paramsp = (params *)malloc(sizeof(queue) * 2);
        paramsp->directory = dirQueue;
        paramsp->file = fileQueue;

        pthread_t *dtid = malloc(sizeof(pthread_t) * dirThreads);
        pthread_t *ftid = malloc(sizeof(pthread_t) * wrapThreads);
        clock_t t;
        t = clock();
        for (int i = 0; i < dirThreads; i++)
        {
            pthread_create(&dtid[i], NULL, directoryTraversal, paramsp);
        }

        for (int i = 0; i < dirThreads; i++)
        {
            pthread_join(dtid[i], NULL);
        }
        for (int i = 0; i < wrapThreads; i++)
        {

            pthread_create(&ftid[i], NULL, lineThread, (void *)fileQueue);
        }
        for (int i = 0; i < wrapThreads; i++)
        {

            pthread_join(ftid[i], NULL);
        }
        if (DEBUG == 2)
        {
            double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds

            printf("\nfun() took %f seconds to execute \n", time_taken);
        }
        
        
        pthread_mutex_destroy(&dirQueue->lock);
        pthread_mutex_destroy(&fileQueue->lock);
        free(paramsp);
        free(dtid);
        free(ftid);
    }
    else
    {
        // normal word wrap
        width = atoi(argv[1]);
        if (argc == 2)
        {
            error = align(width, 0, 1);
        }
        else if (argc > 2)
        {
            for (int i = 2; i < argc; i++) // loop for nonrecursive
            {
                char *regOrDir = malloc(sizeof(char) * strlen(argv[i]) + 1);
                memcpy(regOrDir, argv[i], strlen(argv[i]) + 1);
                int isReg = 0;
                int isDir = 0;
                struct stat s;
                stat(regOrDir, &s);

                if (S_ISREG(s.st_mode))
                {
                    isReg = 1;
                }
                else if (S_ISDIR(s.st_mode))
                {
                    isDir = 1;
                }else{
                    error=1;
                    free(regOrDir);
                }

                if (isReg == 1)
                {
                    if (argc == 3)
                    {
                        int fd = open(regOrDir, O_RDONLY, S_IRWXU);
                        error = align(width, fd, 1);
                        free(regOrDir);
                        close(fd);
                    }
                    else
                    {
                        enqueue(fileQueue, regOrDir);
                    }
                }
                else if (isDir == 1)
                {
                    DIR *folder;
                    struct dirent *entry;
                    folder = opendir(regOrDir);
                    
                    if (folder == NULL)
                    {
                        return EXIT_FAILURE;
                    }
                    while ((entry = readdir(folder)))
                    {
                        char *path = regOrDir;
                        char *file = entry->d_name;
                        int plen = strlen(path);
                        int flen = strlen(file);
                        char *newpath = (char *)malloc(plen + flen + 2);
                        memcpy(newpath, path, plen);
                        newpath[plen] = '/';
                        memcpy(newpath + plen + 1, file, flen + 1);

                        struct stat t;
                        stat(newpath, &t);

                        if (entry->d_name[0] == '.')
                        {
                            free(newpath);
                        }
                        else if (strncmp(entry->d_name, "wrap.", 5) == 0)
                        {
                            free(newpath);
                        }
                        else if (S_ISREG(t.st_mode))
                        {
                        
                            enqueue(fileQueue, newpath);
                        }
                        else
                        {
                            free(newpath);
                        }
                    }
                    closedir(folder);
                    free(regOrDir);
                   
                }
            }
            lineThread(fileQueue);
        }
    }
    freeq(fileQueue);
    freeq(dirQueue);
    free(fileQueue);
    free(dirQueue);
    
    return error;
}
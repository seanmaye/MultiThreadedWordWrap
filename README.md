Multithreaded Word Wrap for CS214
Contributors: Sean Maye(sam710) and Finn Peng(fxp5)
4/20/2022
_____________________________________
Description:

Our goal with this project is to make a version of a program that reformats a text file to fit a certain number of columns, or word wrapping, except this time with multithreading. This program will handle threads that will read a given directory and separate threads that will wrap the files in that directory and its subdirectories. We also have completed the extra credit to include multiple arguments.
(NOTE: when reading from standard input instead of a file, be sure to use CTRL D (^D) once done)
_____________________________________
Descriptions of file(s):

wwExtra.c - a file that contains our previous program for word wrapping along with a new program that allows for recursive directory traversal with the use of concurrent multithreading. this program now also allows for multiple arguments
Makefile - this file is used to quickly compile our programs with certain flags used for debugging
_____________________________________
Testplan:
We will implement a queue struct along with all the helper methods needed in order to recursively traverse a directory. By declaring two queues, one for handling directories and one for handling files, we can allow threading to handle the queues. The number of threads dedicated to each struct will be determined by user input. After recursive directory traversal, we must make sure to free all the malloced structs and pointers. In order to insure we keep track of filepathes correctly, we need to use memcpy(). We will also use mutex locks in order to ensure the threads are not acessing the queues when they should not be. Threads will run concurrent to each other to minimize run time. This can be tested with time.h. Note: for a high amount of threads, particularly a low number of files, the time of creating and using threads will increase run time, rather than increase efficiency.
_____________________________________
Methods and structs we will use:
- open()
- opendir()
- readdir()
- stat()
- memcpy()
- pthread_mutex_lock()
- pthread_mutex_unlock()
- pthread_cond_wait()

- node() - contains a string of a filename and a pointer to the next node
- queue() - FIFO linked list of nodes
- params() - struct of two queues to recursively traverse through directories
_____________________________________
Descriptions of function(s) in ww.c:

int align(int toAlign, int ifd, int ofd) - function which is used to wrap a file to a certain width and returns an integer for error condition
void initq(queue *q) - initializes a queue by setting its variables to NULL
void enqueue(queue *q, char *name) - adds a node to the queue struct and increments its size
char *dequeue(queue *q) - removes a node from the queue and returns a string with the name of the removed node
void *lineThread(void *vargp) - takes a queue of file paths and wraps them with the align function
void display(node *head) - displays a queue and all its nodes, used for debugging
void freeq(queue *q) - frees the nodes in a queue
void *directoryTraversal(void *dir, void *reg) - traverses a directory recursively and populates queues
_____________________________________
Stress testing our program:

1. various amounts of spaces
2. empty file
3. many newlines in file
4. widths of different sizes
5. buffers of differnt sizes
6. newline at the end of file
7. space at the end of file
8. many files
9. directory with many subdirectories
10. directory with no files
11. time using clock to see thread efficiency
12. multiple arguments

If given a file of only blank lines, our program will print only the newlines that indicate the start of a new paragraph
_____________________________________
Description of error messages:

"ERROR BAD FILE DESCRIPTOR" - the input file descriptor is -1 or does not exist
"ERROR NOTHING APPLIES" - word is unable to be formatted
"ERROR IS NOT FILE OR DIRECTORY" - the second argument is not a regular file nor a directory
"ERROR NO SUCH FOLDER" - if the input directory does not exist
"ERROR INCORRECT ARGUMENT" - the given number of arguments are incorrect
_____________________________________
Design notes:

A file with no words to contain no lines. Thus, the smallest non-empty file will contain at least two characters.
You can keep track of how many newlines you've seen since the last non-whitespace character.
If you get to the end of a sequence of whitespace and you've seen 2+ newlines, then you know to write a paragraph break.

Two working queues are being used with M and N threads for a queue of directories and a queue of files respectively.

If the input includes correct and incorrect files, we continue to wrap the correct files, but return error. If there are no correct files in the input then we return error.

We made sure not to do anything with a subdirectory when wrapping a directory without recurive directory traversal.
_____________________________________
Some issues we ran into:

At first we read the whole list as our buffer, as a result our buffer wasn't able to be manipulated.

We kept track of our lineCounter incorrectly, as we incremented it and also added the amount of the word.

Ran into many infinte loops as we originally used a while loop to traverse through the file, we would it much easier to traverse through our buffer using a for loop and iterating character by character.

Edge cases include newline handling, how to handle spaces, and last word in the word array not being used.

For multithreaded wordwrap, we ran into trouble freeing variables since we still need access to certain nodes that were freed during dequeue.

We also ran into trouble freeing correctly when we were traversing recursively.

Keeping certain variables global helped with minimizing helper method parameters.
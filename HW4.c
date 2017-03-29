#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "HW4.h"

/* On my honor, all work here is my own.
 * The basic structure of the pipe creation and use follows the example give on pipe(), pipe2() Linux manpage
 */

/* This program is structured to create 2 pipes and then fork the parent process.  The parent process calls
 * fillCharPipe, while the child process forks a second time. The child/parent process calls processCharPipe while
 * the child/child process calls printCharPipe.  This program assumes a system that supports Linux style pipes.
 */
int main() {
    int pipeFromFillBufferToProcessBuffer[2], pipeFromProcessBufferToPrintBuffer[2];
    pid_t pid;
    if (pipe(pipeFromFillBufferToProcessBuffer) == -1 || pipe(pipeFromProcessBufferToPrintBuffer) == -1) {
        perror("There was an error creating the pipe!");
        exit(EXIT_FAILURE);
    }
    // Fork processCharPipe
    pid = fork();
    if (pid == -1) {
        perror("There was an error with forking processCharPipe!");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // In child processCharPipe process
        // Close the writing side of the pipe from filling to processing the buffer
        close(pipeFromFillBufferToProcessBuffer[1]);
        // Fork printCharPipe
        pid = fork();
        if (pid == -1) {
            perror("There was an error with forking printCharPipe!");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            // In child/child printCharPipe process
            // Close the writing side of the pipe from processing to printing the buffer.
            close(pipeFromProcessBufferToPrintBuffer[1]);
            printCharPipe(pipeFromProcessBufferToPrintBuffer[0]);
            // Close the reading side of the pipe from processing to printing the buffer since we are done with it.
            close(pipeFromProcessBufferToPrintBuffer[0]);
            exit(EXIT_SUCCESS);
        }
        else {
            // In child/parent processCharPipe process
            processCharPipe(pipeFromFillBufferToProcessBuffer[0], pipeFromProcessBufferToPrintBuffer[1]);
            // Close the reading side of the pipe from filling to processing the buffer since we are done with it.
            close(pipeFromFillBufferToProcessBuffer[0]);
            // Close the writing side of the pipe from processing to printing the buffer since we are done with it.
            close(pipeFromProcessBufferToPrintBuffer[1]);
            wait((int *) 0);
            exit(EXIT_SUCCESS);
        }

    }
    else {
        // In parent fillCharPipe process
        fillCharPipe(pipeFromFillBufferToProcessBuffer[1]);
        close(pipeFromFillBufferToProcessBuffer[1]);
        wait((int *) 0);
        exit(EXIT_SUCCESS);
    }
}

void fillCharPipe(int pipeToWriteTo) {
    _ssize_t writeReturnValue;
    int intValOfChar = 0;
    // Get first char from stdin
    intValOfChar = getchar();

    while (intValOfChar != EOF) {
        // Place the previously acquired char into the buffer
        writeReturnValue = write(pipeToWriteTo, &intValOfChar, sizeof(char));
        if (writeReturnValue == -1) {
            perror("There was an error with writing a pipe.  Now terminating!");
            exit(EXIT_FAILURE);
        }
        intValOfChar = getchar();
    }
    // This is how the processCharPipe() knows when there is no more input to read from the buffer.
    intValOfChar = '\0'; //Null char
    writeReturnValue = write(pipeToWriteTo, &intValOfChar, sizeof(char));
    if (writeReturnValue == -1) {
        perror("There was an error with writing a pipe.  Now terminating!");
        exit(EXIT_FAILURE);
    }
}

void processCharPipe(int pipeToReadFrom, int pipeToWriteTo) {
    int const ASCIICODEFORLF = 10;
    int const ASCIICODEFORSPACE = 32;
    int const ASCIICODEFORSTAR = 42;
    int const ASCIICODEFORCARET = 94;
    int const ASCIICODEFORCR = 13;

    _ssize_t readReturnValue;
    _ssize_t writeReturnValue;
    char c;
    char charOutputBuffer;
    readReturnValue = read(pipeToReadFrom, &c, sizeof(char));
    if (readReturnValue == -1) {
        perror("There was an error with reading from a pipe.  Now terminating!");
        exit(EXIT_FAILURE);
    }
    while (c != '\0') {
        if (c == ASCIICODEFORLF) {
            charOutputBuffer = ASCIICODEFORSPACE;
            writeReturnValue = write(pipeToWriteTo, &charOutputBuffer, sizeof(char));
            if (writeReturnValue == -1) {
                perror("There was an error with writing to a pipe.  Now terminating!");
                exit(EXIT_FAILURE);
            }
        }
            // If the char just read in was a * then we need to see what the second character read in is.
        else if (c == ASCIICODEFORSTAR) {
            char secondChar;
            readReturnValue = read(pipeToReadFrom, &secondChar, sizeof(char));
            if (readReturnValue == -1) {
                perror("There was an error with reading from a pipe.  Now terminating!");
                exit(EXIT_FAILURE);
            }
            // If the second char is a * then we there were ** in the pipe that need to be replaced with ^
            if (secondChar == ASCIICODEFORSTAR) {
                charOutputBuffer = ASCIICODEFORCARET;
                writeReturnValue = write(pipeToWriteTo, &charOutputBuffer, sizeof(char));
                if (writeReturnValue == -1) {
                    perror("There was an error with writing to a pipe.  Now terminating!");
                    exit(EXIT_FAILURE);
                }
            }
                // If the second char was not a star see if it is a \n and needs special handling
            else if (secondChar == ASCIICODEFORLF) {
                charOutputBuffer = c;
                writeReturnValue = write(pipeToWriteTo, &charOutputBuffer, sizeof(char));
                if (writeReturnValue == -1) {
                    perror("There was an error with writing to a pipe.  Now terminating!");
                    exit(EXIT_FAILURE);
                }
                charOutputBuffer = ASCIICODEFORSPACE;
                writeReturnValue = write(pipeToWriteTo, &charOutputBuffer, sizeof(char));
                if (writeReturnValue == -1) {
                    perror("There was an error with writing to a pipe.  Now terminating!");
                    exit(EXIT_FAILURE);
                }
            }
                /* If the second char was not a star and does not need special handling, just
                 * put both chars in the pipeToWriteTo without changing them.
                 */
            else {
                charOutputBuffer = c;
                writeReturnValue = write(pipeToWriteTo, &charOutputBuffer, sizeof(char));
                if (writeReturnValue == -1) {
                    perror("There was an error with writing to a pipe.  Now terminating!");
                    exit(EXIT_FAILURE);
                }
                charOutputBuffer = secondChar;
                writeReturnValue = write(pipeToWriteTo, &charOutputBuffer, sizeof(char));
                if (writeReturnValue == -1) {
                    perror("There was an error with writing to a pipe.  Now terminating!");
                    exit(EXIT_FAILURE);
                }
                // If there was a * followed by '\0' then put them both in the pipeToWriteTo and break
                if (secondChar == '\0') break;

            }
        }
            /* If the char removed was neither a /n or a star followed by another star then just
             * put it in the processedBuffer.  I do not have to worry about what to do with the
             * secondChar because the condition where c == ASCIICODEFORSTAR handles those cases.
             */
        else {
            charOutputBuffer = c;
            writeReturnValue = write(pipeToWriteTo, &charOutputBuffer, sizeof(char));
            if(writeReturnValue == -1) {
                perror("There was an error with writing to a pipe.  Now terminating!");
                exit(EXIT_FAILURE);
            }
        }
        // Get the next char from the input pipe
        readReturnValue = read(pipeToReadFrom, &c, sizeof(char));
        if(readReturnValue == -1) {
            perror("There was an error with reading from a pipe.  Now terminating!");
            exit(EXIT_FAILURE);
        }
    }
    // Out of the while loop so a null character is either in secondChar or c.
    charOutputBuffer = '\0';
    writeReturnValue= write(pipeToWriteTo, &charOutputBuffer, sizeof(char));
    if(writeReturnValue == -1) {
        perror("There was an error with writing to a pipe.  Now terminating!");
        exit(EXIT_FAILURE);
    }
}

void printCharPipe(int pipeToReadFrom) {
    int const ASCIICODEFORLF = 10;
    int const OUTPUTLINESIZE = 80;
    _ssize_t readReturnValue;
    int j = 0; // lineOutputBuffer index
    char *lineOutputBuffer = calloc(OUTPUTLINESIZE, sizeof(char));
    char c;
    readReturnValue = read(pipeToReadFrom, &c, sizeof(char));
    if(readReturnValue == -1) {
        perror("There was an error with reading from a pipe.  Now terminating!");
        exit(EXIT_FAILURE);
    }
    // Check if the char just read is a null terminator
    while (c != '\0') {
        lineOutputBuffer[j] = c;
        j++;
        /* If the last char placed into the lineOutputBuffer was the 80th char set the lineOutputBuffer index j
         * back to 0 so we can loop through the buff.
         */
        if ((j % OUTPUTLINESIZE) == 0) {
            j = 0;
            while (j < OUTPUTLINESIZE) {
                putchar(lineOutputBuffer[j]);
                j++;
            }
            putchar(ASCIICODEFORLF);
            // Set j back to 0 because the lineOutputBuffer needs to be refilled from the start.
            j = 0;
        }
        // Get next char to be printed
        readReturnValue = read(pipeToReadFrom, &c, sizeof(char));
        if(readReturnValue == -1) {
            perror("There was an error with reading from a pipe.  Now terminating!");
            exit(EXIT_FAILURE);
        }
    }
}
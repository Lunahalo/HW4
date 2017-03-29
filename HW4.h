
/* fillCharPipe reads char input from stdin and copies it into the file descriptor give by pipeToWriteTo.  It continues
 * to place characters into the pipe until an EOF is encountered, in which case it null terminates the char string.
 */
void fillCharPipe(int pipeToWriteTo);

/* processCharPipe reads characters one at a time from the file descriptor given by pipeToReadFrom and changes /n
 * to spaces and changes ** to ^ and places the changed characters into to the file descriptor given by
 * pipeToWriteTo.  All other characters are just copied from pipeToReadFrom to pipeToWriteTo.
 */
void processCharPipe(int pipeToReadFrom, int pipeToWriteTo);

/*  printCharPipe reads characters from the file descriptor given by pipeToReadFrom and fills an internal buffer of
 *  size OUTPUTLINESIZE.  Once this buffer is fill the buffer is printed to stdout.  The preset value of OUTPUTLINESIZE
 *  causes the function to print lines of 80 characters followed by a /n.  If a null terminator is read from
 *  pipeToReadFrom the functions stops reading characters.  If the buffer is not full then the function will not
 *  print the partially filled buffer to stdout before terminating.
 */
void printCharPipe(int pipeToReadFrom);


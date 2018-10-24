# This is a school assignment for application programming course

## The idea 
Create an application that have two ends for a communication: one sending and the other receiving morse codes.

## Assignment requirements
- C programming language
- Clean exit if user presses CTRL-C
- Log file writing
- Error handling with failed functions
- Writing(e.g. log file) and reading(e.g. config file) from files
- Signal handling
- Creation of a child process
- File locking
- Nonblocking I/O

## Application requirements
- Build a program that forks a new child process
- Open two files: one for reading with child and other for writing with parent
- Child communicates the data in the file to parent using POSIX signal APIs
- The signals are encoded with morse codes
- Log codes sent and received
- Lock fil

## Sources:
[Stackoverflow: Morse code encoding tree](https://stackoverflow.com/questions/28045172/morse-code-converter-in-c)

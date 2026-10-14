#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

using namespace std;

const int READ_END = 0;
const int WRITE_END = 1;

bool isDifferent(char* s1, char* s2);
void createNextFilter(int readFromParentFd, int writeToParentFd);

int main(int argc, char* argv[]) 
{    
    if (argc > 1) {
        int parentToChildFd[2];
        int childToParentFd[2];

        if (pipe(parentToChildFd) < 0 || pipe(childToParentFd) < 0) {
            return 1;
        }

        pid_t childPid = fork();

        if (childPid == 0) {
            close(childToParentFd[READ_END]);
            close(parentToChildFd[WRITE_END]);
            createNextFilter(parentToChildFd[READ_END], childToParentFd[WRITE_END]);
            close(childToParentFd[WRITE_END]);
            close(parentToChildFd[READ_END]);
            exit(0);
        } else {
            close(parentToChildFd[READ_END]);

            for (int i = 1; i < argc; i++) {
                size_t numBytes = strlen(argv[i])+1;
                write(parentToChildFd[WRITE_END], &numBytes, sizeof(numBytes));
                write(parentToChildFd[WRITE_END], argv[i], numBytes);
            }

            close(parentToChildFd[WRITE_END]);

            int numProc;
            read(childToParentFd[READ_END], &numProc, sizeof(numProc));

            cout << numProc << " distinct codes found" << endl;

            int status;
            waitpid(childPid, &status, 0);
        }  
    }

    return 0;
}

void createNextFilter(int readFromParentFd, int writeToParentFd) {
        size_t numBytes;
        if (!read(readFromParentFd, &numBytes, sizeof(numBytes))) return;
        char word[numBytes];
        if (!read(readFromParentFd, word, numBytes)) return;

        pid_t pid = getpid();
        cout << "Process: " << pid << " my code is " << word << endl;

        int parentToChildFd[2];
        int childToParentFd[2];
        bool hasChild = false;
        pid_t childPid = 0;
        while (read(readFromParentFd, &numBytes, sizeof(numBytes))) {
            char toCompare[numBytes];
            if (!read(readFromParentFd, toCompare, numBytes)) break;

            if (isDifferent(word, toCompare)) {
                if (!hasChild) {
                    // create child
                    if (pipe(parentToChildFd) < 0 || pipe(childToParentFd)) {
                        exit(1);
                    }

                    hasChild = true;
                    childPid = fork();
                    if (childPid == 0) {
                        close(childToParentFd[READ_END]);
                        close(parentToChildFd[WRITE_END]);

                        createNextFilter(parentToChildFd[READ_END], childToParentFd[WRITE_END]);

                        close(childToParentFd[WRITE_END]);
                        close(parentToChildFd[READ_END]);
                        exit(0);
                    } else {
                        close(childToParentFd[WRITE_END]);
                        close(parentToChildFd[READ_END]);
                    }
                }

                // write different word to child process pipe
                write(parentToChildFd[WRITE_END], &numBytes, sizeof(numBytes));
                write(parentToChildFd[WRITE_END], toCompare, numBytes);
            }
            // else do nothing (filter)
        }

        close(parentToChildFd[WRITE_END]);

        // keep track of the number of processes
        int numProc = 1;
        if (hasChild) {
            read(childToParentFd[READ_END], &numProc, sizeof(numProc));
            numProc += 1;
            close(childToParentFd[READ_END]);
        }
        write(writeToParentFd, &numProc, sizeof(numProc));
        close(writeToParentFd);

        int status;
        if (childPid != 0) {
            waitpid(childPid, &status, 0);
        }
}

bool isDifferent(char* s1, char* s2) {
    if (strlen(s1) != strlen(s2)) return true;

    int count = 0;
    for (int i = 0; i < strlen(s1); i++) {
        if (s1[i] != s2[i]) {
            count++;
        }
    }

    return count > 1;
}
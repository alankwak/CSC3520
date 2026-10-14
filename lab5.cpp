#include <iostream>
#include <unistd.h>

using namespace std;

const int READ_END = 0;
const int WRITE_END = 1;

void print1to10(int writeFileDescriptor);  // Called by child
void print11to20(int readFileDescriptor);  // Called by parent

int main(int argc, char* argv[]) 
{    
    int fd[2];

    if(pipe(fd) < 0) //error
    {
        return 1;
    }

    if (fork() == 0) {
        close(fd[READ_END]);
        print1to10(fd[WRITE_END]);
        close(fd[WRITE_END]);
    } else {
        close(fd[WRITE_END]);
        print11to20(fd[READ_END]);
        close(fd[READ_END]);
    }
}

void print1to10(int writeFileDescriptor) {
    int squares[10];
    for (int i = 1; i <= 10; i++) {
        squares[i-1] = i*i;
    }
    write(writeFileDescriptor, squares, sizeof(squares));
}

void print11to20(int readFileDescriptor) {
    int last10[10];
    for(int i = 11; i <= 20; i++) {
        last10[i-11] = i*i;
    }
    int first10[10];
    read(readFileDescriptor, first10, sizeof(first10));
    for (int i = 0; i < 20; i++) {
        if (i < 10) {
            cout << i+1 << " " << first10[i] << endl;
        } else {
            cout << i+1 << " " << last10[i-10] << endl;
        }
    }
}
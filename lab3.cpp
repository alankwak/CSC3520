#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

void countdown(int i) {
    cout << i << "..." << endl;
    if (i > 1) {
        pid_t p = fork();

        if (p == 0) {
            countdown(i-1);
        }

        int status;
        waitpid(p, &status, 0);
    }

}

int main(int argc, char* argv[]) 
{    
    int n = 5;
    if (argc > 1) {
        n = stoi(argv[1]);
    }
    
    int p = fork();
    if (p == 0) countdown(n);
    else {
        int status;
        waitpid(p, &status, 0);
        cout << "LIFTOFF" << endl;
    }
}

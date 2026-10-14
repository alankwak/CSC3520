#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

void factorial(int curr) {
    if (curr > 1) {
        cout << "Process N = " << curr << ": Forking child..." << endl;
        int id = fork();

        if (id == 0) {
            factorial(curr-1);
        }

        int status;
        waitpid(id, &status, 0);
        cout << "Process N = " << curr << ": Child returned " << WEXITSTATUS(status) << ", computing " << WEXITSTATUS(status) << " * " << curr << " = " << WEXITSTATUS(status) * curr << endl;
        exit(curr * WEXITSTATUS(status));
    }
    
    cout << "Process N = " << curr << ": Base case, returning 1" << endl;
    exit(1);
}

int main(int argc, char* argv[]) 
{    
    int N = stoi(argv[1]);

    int child = fork();
    if (!child) {
        cout << "Process N = " << N << ": Starting factorial chain" << endl;
        factorial(N);
    }
    int status;
    waitpid(child, &status, 0);

    cout << N << "! = " << WEXITSTATUS(status) << endl;
}

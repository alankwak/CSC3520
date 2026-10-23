#include <iomanip>
#include <iostream>
#include <cmath>
#include <unistd.h>
#include <pthread.h> //for threads and semaphores
#include <semaphore.h>

using namespace std;

void* runner(void*);
void printLine(int &startDay, int monthNum, bool skip);
bool isLeapYear(int year);
int nextMonth(int monthNum, bool lastInRow, int row);

sem_t* monthSync;
sem_t* rowSync;

enum MonthNum {
    JANUARY,
    FEBRUARY,
    MARCH,
    APRIL,
    MAY,
    JUNE,
    JULY,
    AUGUST,
    SEPTEMBER,
    OCTOBER,
    NOVEMBER,
    DECEMBER
};
const string monthNames[12] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};
int daysInMonth[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

int numMonthsPerRow;
int skipStartingDays[12];
int year;

int numRows;

int main(int argc, char* argv[])
{
    if(argc != 4)
    {
        cout<<"Wrong number of command line arguments"<<endl;
        exit(0);
    }

    numMonthsPerRow = min(12, max(1, atoi(argv[1]))); // ensure input is 1-12
    skipStartingDays[JANUARY] = min(6, max(0, atoi(argv[2]))); // ensure input is 0-6
    year = atoi(argv[3]);
    if(isLeapYear(year)) {
        daysInMonth[FEBRUARY]++;
    }

    for(int i = FEBRUARY; i <= DECEMBER; i++) {
        skipStartingDays[i] = (skipStartingDays[i-1] + daysInMonth[i-1]) % 7; 
    }

    monthSync = new sem_t[12];
    for (int i = JANUARY; i <= DECEMBER; i++) {
        sem_init(monthSync+i, 0, i % numMonthsPerRow == 0); // will be initialized to 1 if January, else 0
    }

    numRows = ceil(1.0 * 12 / numMonthsPerRow);
    rowSync = new sem_t[numRows];
    for (int i = 0; i < numRows; i++) {
        sem_init(rowSync+i, 0, i == 0); // same idea
    }

    pthread_t threads[12];
    //create the array of threads and start them in the runner function
    for (int i = JANUARY; i <= DECEMBER; i++) {
        int* monthNum = new int(i);
        pthread_create(threads+i, 0, runner, monthNum);
    }

    for (int i = 0; i < 12; i++) {
        pthread_join(threads[i], nullptr);
    }

    //clean up any data on the heap

    return 0;
}
//--
void* runner(void* p_data)
{
    int* p_monthNum = (int*) p_data;
    int row = *p_monthNum / numMonthsPerRow;
    bool lastinrow = (*p_monthNum + 1)%numMonthsPerRow == 0 || *p_monthNum == DECEMBER;

    string* header = new string(monthNames[*p_monthNum] + " " + to_string(year));

    int leftPadding = (23 - (*header).size()) / 2;
    int rightPadding = 23 - (*header).size() - leftPadding;

    // the index of the next month in the current row (wraps around)
    int nextInRow = row * numMonthsPerRow;
    if (!lastinrow) nextInRow += (*p_monthNum + 1) % numMonthsPerRow;

    // if first in row wait for it to be ready
    if (*p_monthNum % numMonthsPerRow == 0) sem_wait(&rowSync[row]);

    // headings
    sem_wait(&monthSync[*p_monthNum]);
    cout << string(leftPadding, ' ') << *header << string(rightPadding, ' ');
    if (lastinrow) {
        cout << endl;
    }
    sem_post(&monthSync[nextInRow]);

    sem_wait(&monthSync[*p_monthNum]);
    cout << "\033[34m" << " S  M  T  W  R  F  S   " << "\033[0m";
    if (lastinrow) {
        cout << endl;
    }
    sem_post(&monthSync[nextInRow]);

    delete header;

    // calendar days
    int dateNum = 1;
    for (int line = 0; line < 6; line++) {
        sem_wait(&monthSync[*p_monthNum]);

        printLine(dateNum, *p_monthNum, line == 0);

        if (lastinrow) cout << endl;
        
        sem_post(&monthSync[nextInRow]);
    }

    if (lastinrow) sem_post(&rowSync[(row+1)%numRows]);

    delete p_monthNum;

    return 0;
}

void printLine(int &startDay, int monthNum, bool skip) {
    for (int i = 0; i < 7; i++) {
        cout << setw(2) << ((skip && i < skipStartingDays[monthNum]) || startDay > daysInMonth[monthNum] ? "" : to_string(startDay)) << " ";
        if (!skip || i >= skipStartingDays[monthNum]) startDay++;
    }
    // space between "calendars"
    cout << "  ";
}

bool isLeapYear(int year) {
    return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
}
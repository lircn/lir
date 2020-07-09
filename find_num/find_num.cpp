
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

/* Definations */
#define IN_FILE_PATH    "in.txt"
#define OUT_FILE_PATH   "out.txt"
#define ARR_SIZE        (20000)

/* Global vars */
static double data[ARR_SIZE] = {0};
static double target = 0;
static time_t start_time;
static int    total_cases = 0;

/* Declears */
void print_vector(vector<int> &v);
void find_target(double target, vector<int> &v_target);
bool equal(double x, double y);

/* Implements */
void cal(double sum, int idx, vector<int> tag)
{
    if (data[idx] <= 0) {
        return;
    }
    if (++total_cases % 10000000 == 0) {
        cout << "calculate cases: " << total_cases;
        time_t end;
        time(&end);
        printf(", cost time %.2fs\n", difftime(end, start_time));
    }

    if (sum > target) {
        return;
    } else if (equal(sum, target)) {
        //printf("-dbg- sum: %.2f, target: %.2f, equal: %s\n", sum, target,
        //        (equal(sum, target))?"true":"false");
        find_target(target, tag);
    } else {
        // do not add this
        cal(sum, idx + 1, tag);

        // add this
        sum += data[idx];
        tag.push_back(idx);
        cal(sum, idx + 1, tag);
    }
}

inline void print_vector(vector<int> &v)
{
    for (auto it : v) {
        cout << it << " ";
    }
    cout << endl;
}

void find_target(double target, vector<int> &v_target)
{
    ofstream of(OUT_FILE_PATH, ios::app);
    if (!of) {
        return;
    }

    cout << "find result: " << target << endl;
    of << "find result: " << target << endl;

    cout << "idx: ";
    of << "idx: ";
    for (auto it : v_target) {
        cout << it << " ";
        of << it << " ";
    }
    cout << endl;
    of << endl;

    cout << "val: ";
    of << "val: ";
    for (auto it : v_target) {
        cout << data[it] << " ";
        of << data[it] << " ";
    }
    cout << endl;
    of << endl;

    of.close();

    //exit(0); // find and exit
}

inline bool equal(double x, double y)
{
    return (x-y>-0.000001 && x-y<0.000001);
}

int main(int argc, const char *argv[])
{
    ifstream file;
    file.open(IN_FILE_PATH, ios::in);

    if (!file.is_open()) {
        cout << "open error" << endl;
        return 0;
    }

    cout << "input target number: ";
    cin >> target;

    size_t cnt = 0;
    string str;
    while (getline(file, str)) {
        if (str.empty()) {
            continue;
        }

        data[cnt] = stod(str, NULL);
        cnt++;
    }

    time(&start_time);
    vector<int> tag;
    cal(0, 0, tag);

    time_t end;
    time(&end);
    printf("total cost time %.2fs\n", difftime(end, start_time));

    system("pause");

    return 0;
}

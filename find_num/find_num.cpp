
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

static time_t start_time;
static bool stop = false;

/* Declears */
void print_res(const vector<vector<double>> &res, double target);
bool equal(double x, double y);

/* Implements */
void cal(vector<vector<double>> &res, const int order, const double target,
        vector<double> &local, const vector<double> &num, bool root)
{
    static uint64_t total_cases = 0;
    if (stop) {
        return;
    }

    total_cases++;
    if (total_cases % 100000000 == 0) {
        cout << "calculate cases: " << total_cases;
        time_t end;
        time(&end);
        printf(", cost time %.2fs\n", difftime(end, start_time));
    }

    if (equal(target, 0)) {
        res.push_back(local);
        stop = true;
        cout << "calculate cases: " << total_cases << endl;
        return;
    } else {
        for (int i = order; i < num.size(); i++) {
            if (num[i] > target) {
                return;
            }
            if (i && num[i]==num[i-1] && i>order) {
                continue;
            }
            local.push_back(num[i]);
            cal(res, i+1, target-num[i], local, num, false);
            local.pop_back();
        }
    }
}

void print_res(const vector<vector<double>> &res, double target)
{
    ofstream fout(OUT_FILE_PATH, ios::app);
    if (!fout) {
        return;
    }

    cout << "find result: " << target << endl;
    fout << "find result: " << target << endl;

    for (auto vec : res) {
        for (auto item : vec) {
            cout << item << " ";
            fout << item << " ";
        }
        cout << endl;
        fout << endl;
    }

    fout.close();
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

    double target;
    cout << "input target number: ";
    cin >> target;

    cout << target << endl;
    vector<double> num;
    string str;
    while (getline(file, str)) {
        if (str.empty()) {
            continue;
        }
        num.push_back(stod(str, NULL));
    }

    time(&start_time);

    vector<vector<double>> res;
    vector<double> local;

    sort(num.begin(), num.end());
    int tail = 0;
    for (; tail < num.size(); tail++) {
        if (num[tail] > target) {
            break;
        }
    }
    num = vector<double>(num.begin(), num.begin()+tail);
    cal(res, 0, target, local, num, true);
    print_res(res, target);

    time_t end;
    time(&end);
    printf("total cost time %fs\n", difftime(end, start_time));

    system("pause");

    return 0;
}

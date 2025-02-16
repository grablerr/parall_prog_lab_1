#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <chrono>
#include <string>
#include <filesystem>

using namespace std;
using namespace chrono;

ostream& operator<<(ostream& os, const vector<vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int num : row) {
            os << num << " ";
        }
        os << endl;
    }
    return os;
}

vector<vector<int>> readCSV(const string& filename, int size) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Can`t open file " << filename << endl;
        return {};
    }

    vector<vector<int>> matrix(size, vector<int>(size));
    string line;

    int rowIndex = 0;
    while (getline(file, line) && rowIndex < size) {
        stringstream ss(line);
        string cell;
        int colIndex = 0;

        while (getline(ss, cell, ',') && colIndex < size) {
            matrix[rowIndex][colIndex++] = stoi(cell);
        }

        rowIndex++;
    }

    return matrix;
}

void writeCSV(const string& filename, const vector<vector<int>>& matrix) {
    ofstream file(filename);
    for (const auto& row : matrix) {
        for (size_t j = 0; j < row.size(); ++j) {
            file << row[j];
            if (j < row.size() - 1) file << ",";
        }
        file << "\n";
    }
}

vector<vector<int>> multiplyMatrices(const vector<vector<int>>& A, const vector<vector<int>>& B, int size) {
    vector<vector<int>> result(size, vector<int>(size, 0));
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            for (int k = 0; k < size; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return result;
}

void multiplyAndSave(int size) {
    vector<vector<int>> A = readCSV("../../../matrix_A.csv", size);
    vector<vector<int>> B = readCSV("../../../matrix_B.csv", size);

    auto start = high_resolution_clock::now();
    vector<vector<int>> result = multiplyMatrices(A, B, size);
    auto end = high_resolution_clock::now();
    duration<double> duration = end - start;

    writeCSV("../../../Results/" + to_string(size) + "x" + to_string(size) + ".csv", result);

    ofstream timeFile("../../../Results/" + to_string(size) + "time.txt");
    timeFile << duration.count() << " seconds";
}

int main() {
    vector<int> sizes = { 128, 256, 384, 512, 640, 768, 896, 1024 };
    for (int size : sizes) {
        multiplyAndSave(size);
    }
   
    return 0;
}

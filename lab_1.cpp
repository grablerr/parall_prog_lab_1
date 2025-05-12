#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <chrono>

using namespace std;
using namespace chrono;

vector<vector<int>> readCSV(const string& filename, int size) {
    ifstream file(filename);
    vector<vector<int>> matrix(size, vector<int>(size));
    string line;
    int row = 0;
    while (getline(file, line) && row < size) {
        stringstream ss(line);
        string val;
        int col = 0;
        while (getline(ss, val, ',') && col < size) {
            matrix[row][col++] = stoi(val);
        }
        row++;
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

void multiplyAndSaveMPI(int size, int rank, int numProcs) {
    vector<vector<int>> A, B;

    if (rank == 0) {
        A = readCSV("../../../matrix_A.csv", size);
        B = readCSV("../../../matrix_B.csv", size);
    }

    vector<int> B_flat(size * size);
    if (rank == 0) {
        for (int i = 0; i < size; ++i)
            for (int j = 0; j < size; ++j)
                B_flat[i * size + j] = B[i][j];
    }
    MPI_Bcast(B_flat.data(), size * size, MPI_INT, 0, MPI_COMM_WORLD);

    int rowsPerProc = size / numProcs;
    int remainder = size % numProcs;
    int myRows = (rank < remainder) ? rowsPerProc + 1 : rowsPerProc;
    int offset = rank * rowsPerProc + min(rank, remainder);

    vector<int> A_part(myRows * size);
    if (rank == 0) {
        for (int p = 0; p < numProcs; ++p) {
            int pRows = (p < remainder) ? rowsPerProc + 1 : rowsPerProc;
            int pOffset = p * rowsPerProc + min(p, remainder);
            if (p == 0) {
                for (int i = 0; i < pRows; ++i)
                    for (int j = 0; j < size; ++j)
                        A_part[i * size + j] = A[pOffset + i][j];
            } else {
                vector<int> temp(pRows * size);
                for (int i = 0; i < pRows; ++i)
                    for (int j = 0; j < size; ++j)
                        temp[i * size + j] = A[pOffset + i][j];
                MPI_Send(temp.data(), pRows * size, MPI_INT, p, 0, MPI_COMM_WORLD);
            }
        }
    } else {
        MPI_Recv(A_part.data(), myRows * size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    vector<int> localResult(myRows * size, 0);
    for (int i = 0; i < myRows; ++i) {
        for (int k = 0; k < size; ++k) {
            for (int j = 0; j < size; ++j) {
                localResult[i * size + j] += A_part[i * size + k] * B_flat[k * size + j];
            }
        }
    }

    vector<vector<int>> result;
    if (rank == 0) result.resize(size, vector<int>(size));

    if (rank == 0) {
        for (int i = 0; i < myRows; ++i)
            for (int j = 0; j < size; ++j)
                result[i][j] = localResult[i * size + j];

        for (int p = 1; p < numProcs; ++p) {
            int pRows = (p < remainder) ? rowsPerProc + 1 : rowsPerProc;
            int pOffset = p * rowsPerProc + min(p, remainder);
            vector<int> temp(pRows * size);
            MPI_Recv(temp.data(), pRows * size, MPI_INT, p, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < pRows; ++i)
                for (int j = 0; j < size; ++j)
                    result[pOffset + i][j] = temp[i * size + j];
        }

        writeCSV("../../../Results/" + to_string(size) + "x" + to_string(size) + ".csv", result);
    } else {
        MPI_Send(localResult.data(), myRows * size, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    vector<int> sizes = { 128, 256, 384, 512, 640, 768, 896, 1024 };

    for (int size : sizes) {
        MPI_Barrier(MPI_COMM_WORLD); // синхронизация
        auto start = high_resolution_clock::now();
        multiplyAndSaveMPI(size, rank, numProcs);
        auto end = high_resolution_clock::now();

        if (rank == 0) {
            duration<double> elapsed = end - start;
            ofstream timeFile("../../../Results/" + to_string(size) + "time.txt");
            timeFile << elapsed.count() << " seconds";
        }
    }

    MPI_Finalize();
    return 0;
}

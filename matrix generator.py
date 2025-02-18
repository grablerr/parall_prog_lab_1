import numpy as np
import pandas as pd
import os


def generate_matrix_csv(filename, size=1024):
    matrix = np.random.randint(0, 100, (size, size))
    np.savetxt(filename, matrix, fmt='%d', delimiter=",")


def load_matrix(filename):
    return pd.read_csv(filename, header=None).values


def multiply_and_compare(matrix_A, matrix_B, results_folder):
    sizes = [128, 256, 384, 512, 640, 768, 896, 1024]

    for size in sizes:
        sub_A = matrix_A[:size, :size]
        sub_B = matrix_B[:size, :size]

        result = np.dot(sub_A, sub_B)

        result_file = os.path.join(results_folder, f"{size}x{size}.csv")
        if not os.path.exists(result_file):
            print(f"Файл {result_file} не найден!")
            continue

        expected_result = load_matrix(result_file)

        if np.array_equal(result, expected_result):
            print(f"Размер {size}x{size}: Произведение совпадает")
        else:
            print(f"Размер {size}x{size}: Произведение НЕ совпадает")


matrix_A = load_matrix("matrix_A.csv")
matrix_B = load_matrix("matrix_B.csv")

multiply_and_compare(matrix_A, matrix_B, "Results")

# generate_matrix_csv("matrix_A.csv")
# generate_matrix_csv("matrix_B.csv")

import numpy as np

def generate_matrix_csv(filename, size=1024):
    matrix = np.random.randint(0, 100, (size, size))
    np.savetxt(filename, matrix, fmt='%d', delimiter=",")

generate_matrix_csv("matrix_A.csv")
generate_matrix_csv("matrix_B.csv")
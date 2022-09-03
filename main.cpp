#include <stdio.h>
#define value_type double

#include "FastMatrixLoader/fcoo.h"
#include "FastMatrixLoader/fcsr.h"

int main(int argc, char **argv)
{
    if (coo *matrix_coo = coo_load(argv[1], 1))
    {
        printf("%d %d %d\n", matrix_coo->m, matrix_coo->n, matrix_coo->nnz);
        for (int i = 0; i < matrix_coo->nnz; ++i)
            printf("%d%c", matrix_coo->row[i], i == matrix_coo->nnz - 1 ? '\n' : '\t');
        for (int i = 0; i < matrix_coo->nnz; ++i)
            printf("%d%c", matrix_coo->col[i], i == matrix_coo->nnz - 1 ? '\n' : '\t');
        if (matrix_coo->val)
            for (int i = 0; i < matrix_coo->nnz; ++i)
                printf("%g%c", matrix_coo->val[i], i == matrix_coo->nnz - 1 ? '\n' : '\t');
        if (matrix_coo->val_im)
            for (int i = 0; i < matrix_coo->nnz; ++i)
                printf("%g%c", matrix_coo->val_im[i], i == matrix_coo->nnz - 1 ? '\n' : '\t');

        free_coo(matrix_coo);
    }

    printf("-------------------------\n");

    if (csr *matrix_csr = csr_load(argv[1], 1))
    {
        printf("%d %d %d\n", matrix_csr->m, matrix_csr->n, matrix_csr->nnz);
        for (int i = 0; i < matrix_csr->m; ++i)
            printf("%d%c", matrix_csr->row[i], i + 1 < matrix_csr->m ? '\t' : '\n');
        for (int i = 0; i < matrix_csr->nnz; ++i)
            printf("%d%c", matrix_csr->col[i], i + 1 < matrix_csr->nnz ? '\t' : '\n');
        if (matrix_csr->val)
            for (int i = 0; i < matrix_csr->nnz; ++i)
                printf("%g%c", matrix_csr->val[i], i + 1 < matrix_csr->nnz ? '\t' : '\n');
        if (matrix_csr->val_im)
            for (int i = 0; i < matrix_csr->nnz; ++i)
                printf("%g%c", matrix_csr->val_im[i], i + 1 < matrix_csr->nnz ? '\t' : '\n');
        free_csr(matrix_csr);
    }
    return 0;
}

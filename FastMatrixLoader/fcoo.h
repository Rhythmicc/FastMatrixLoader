#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "mmio.h"

#ifndef value_type
    typedef float value_type;
#endif // !value_type
#ifndef index_type
    typedef unsigned int index_type;
#endif // !index_type

    typedef struct
    {
        index_type m, n, nnz;
        index_type *row, *col;
        value_type *val, *val_im;
    } coo;

    coo *__new_coo(index_type m, index_type n, index_type nnz)
    {
        coo *c = (coo *)malloc(sizeof(coo));
        c->m = m;
        c->n = n;
        c->nnz = nnz;
        return c;
    }

    /*
     * Reads a COO matrix from a file.
     *
     * @param filename The name of the file to read.
     * @param is_zero_based Whether the matrix is zero-based or not.
     * @return The COO matrix.
     */
    coo *coo_load(const char *filepath, int is_zero_based)
    {
        coo *res = NULL;
        int isInteger = 0, isReal = 0, isPattern = 0, isSymmetric_tmp = 0, isComplex = 0, m, n, nnz;
        MM_typecode matcode;

        FILE *fp = fopen(filepath, "r");
        if (fp == NULL)
            return res;

        if (mm_read_banner(fp, &matcode) != 0)
        {
            printf("Could not process Matrix Market banner.\n");
            return res;
        }
        fclose(fp);

        if (mm_is_pattern(matcode))
            isPattern = 1;
        if (mm_is_real(matcode))
            isReal = 1;
        if (mm_is_complex(matcode))
            isComplex = 1;
        if (mm_is_integer(matcode))
            isInteger = 1;
        if (mm_is_symmetric(matcode) || mm_is_hermitian(matcode))
            isSymmetric_tmp = 1;

        char *data;
        size_t len = 0;

        int fd = open(filepath, O_RDONLY);
        if (fd < 0)
            return res;
        len = lseek(fd, 0, SEEK_END);

        char *mbuf = (char *)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
        data = (char *)malloc(len);
        memcpy(data, mbuf, len);
        close(fd);
        munmap(mbuf, len);

        char *line = strtok(data, "\n");
        while (line != NULL && line[0] == '%')
            line = strtok(NULL, "\n");
        sscanf(line, "%d%d%d", &m, &n, &nnz);
        size_t _len = isSymmetric_tmp ? nnz * 2 : nnz;

        index_type *ia = (index_type *)malloc(sizeof(index_type) * _len);
        index_type *ja = (index_type *)malloc(sizeof(index_type) * _len);

        value_type *val, *val_im;
        if (!isPattern)
            val = (value_type *)malloc(sizeof(value_type) * _len);
        if (isComplex)
            val_im = (value_type *)malloc(sizeof(value_type) * _len);

        line = strtok(NULL, "\n");

        for (int index = 0; index < nnz; ++index)
        {
            if (isPattern)
                sscanf(line, "%d%d", ia + index, ja + index);
            else if (isReal || isInteger)
                sscanf(line, sizeof(value_type) == sizeof(float) ? "%d%d%g" : "%d%d%lg", ia + index, ja + index, val + index);
            else if (isComplex)
                sscanf(line, sizeof(value_type) == sizeof(float) ? "%d%d%g%g" : "%d%d%lg%lg", ia + index, ja + index, val + index, val_im + index);

            if (is_zero_based)
                --ia[index], --ja[index];

            if (isSymmetric_tmp && ia[index] != ja[index])
            {
                ++index, ++nnz;
                index_type tmp = ia[index - 1];
                ia[index] = ja[index - 1];
                ja[index] = tmp;
                if (isReal || isInteger)
                    val[index] = val[index - 1];
                else if (isComplex)
                    val[index] = val[index - 1], val_im[index] = val_im[index - 1];
            }

            if ((line = strtok(NULL, "\n")) == NULL && (index + 1 < nnz))
            {
                free(data), free(ia), free(ja);
                if (!isPattern)
                    free(val);
                if (isComplex)
                    free(val_im);
                return res;
            }
        }

        res = __new_coo(m, n, nnz);
        res->row = ia;
        res->col = ja;
        if (!isPattern)
        {
            val = (value_type *)realloc(val, sizeof(value_type) * nnz);
            res->val = val;
        }
        if (isComplex)
        {
            val_im = (value_type *)realloc(val_im, sizeof(value_type) * nnz);
            res->val_im = val_im;
        }
        return res;
    }

    void free_coo(coo *c)
    {
        if (c->val)
            free(c->val);
        if (c->val_im)
            free(c->val_im);
        free(c->col), free(c->row), free(c);
    }

#ifdef __cplusplus
}
#endif
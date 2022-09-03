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
#define value_type float
#endif // !value_type
#ifndef index_type
    typedef unsigned int index_type;
#endif // !index_type

    typedef struct
    {
        index_type m, n, nnz;
        index_type *row, *col;
        value_type *val, *val_im;
    } csr;

    // allocate a csr data structure
    csr *__new_csr(index_type m, index_type n, index_type nnz, int isComplex, int isPattern)
    {
        csr *res = (csr *)malloc(sizeof(csr));
        res->m = m;
        res->n = n;
        res->nnz = nnz;

        res->row = (index_type *)malloc(sizeof(index_type) * (m + 1));
        res->col = (index_type *)malloc(sizeof(index_type) * nnz);
        if (!isPattern)
            res->val = (value_type *)malloc(sizeof(value_type) * nnz);
        else
            res->val = NULL;
        if (isComplex)
            res->val_im = (value_type *)malloc(sizeof(value_type) * nnz);
        else
            res->val_im = NULL;
        return res;
    }

    csr *csr_load(const char *filePath, int is_zero_based)
    {
        csr *res = 0;
        int isInteger = 0, isReal = 0, isPattern = 0, isSymmetric_tmp = 0, isComplex = 0, m, n, nnz;
        MM_typecode matcode;

        FILE *fp = fopen(filePath, "r");
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

        int fd = open(filePath, O_RDONLY);
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

        index_type *cnt_row = (index_type *)calloc(m + 1, sizeof(index_type));
        index_type *ia = (index_type *)malloc(sizeof(index_type) * nnz);
        index_type *ja = (index_type *)malloc(sizeof(index_type) * nnz);

        value_type *val, *val_im;
        if (!isPattern)
            val = (value_type *)malloc(sizeof(value_type) * nnz);
        if (isComplex)
            val_im = (value_type *)malloc(sizeof(value_type) * nnz);

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

            ++cnt_row[ia[index]];
            if ((line = strtok(NULL, "\n")) == NULL && (index + 1 < nnz))
            {
                free(data), free(ia), free(ja), free(cnt_row);
                if (!isPattern)
                    free(val);
                if (isComplex)
                    free(val_im);
                return res;
            }
        }

        if (isSymmetric_tmp)
            for (int i = 0; i < nnz; ++i)
                if (ia[i] != ja[i])
                    ++cnt_row[ja[i]];

        int old_val = cnt_row[0], new_val;
        cnt_row[0] = 0;
        for (int i = 1; i <= m; ++i)
        {
            new_val = cnt_row[i];
            cnt_row[i] = old_val + cnt_row[i - 1];
            old_val = new_val;
        }

        res = __new_csr(m, n, nnz, isComplex, isPattern);
        memcpy(res->row, cnt_row, sizeof(index_type) * (m + 1));
        memset(cnt_row, 0, sizeof(index_type) * (m + 1));

        for (int index = 0; index < nnz; ++index)
        {
            index_type offset = res->row[ia[index]] + cnt_row[ia[index]];
            res->col[offset] = ja[index];
            if (!isPattern)
                res->val[offset] = val[index];
            if (isComplex)
                res->val_im[offset] = val_im[index];
            ++cnt_row[ia[index]];

            if (isSymmetric_tmp && ia[index] != ja[index])
            {
                offset = res->row[ja[index]] + cnt_row[ja[index]];
                res->col[offset] = ia[index];
                if (!isPattern)
                    res->val[offset] = val[index];
                if (isComplex)
                    res->val_im[offset] = val_im[index];
                ++cnt_row[ja[index]];
                ++res->nnz;
            }
        }

        free(data), free(ia), free(ja), free(cnt_row);
        if (!isPattern)
            free(val);
        if (isComplex)
            free(val_im);
        return res;
    }

    void free_csr(csr *csr_p)
    {
        free(csr_p->row);
        free(csr_p->col);
        if (csr_p->val)
            free(csr_p->val);
        if (csr_p->val_im)
            free(csr_p->val_im);
        free(csr_p);
    }

#ifdef __cplusplus
}
#endif

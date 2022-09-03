# 一个快速矩阵加载器

目前仅支持从mtx文件读取，并保存为COO或CSR类型的矩阵。

## 安装

把`FastMatrixLoader`文件夹添加入你的include路径即可。

## API

```cpp
#define value_type double
#define index_type uint

#include "FastMatrixLoader/fcoo.h"
#include "FastMatrixLoader/fcsr.h"

...

/*
typedef struct
{
    index_type m, n, nnz;
    index_type *row, *col;
    value_type *val, *val_im;
} coo; 
*/

coo*coo_matrix = coo_load("dist/ash85.mtx", 1); // 1 -> zero_based, 0 -> one_based

/*
typedef struct
{
    index_type m, n, nnz;
    index_type *row, *col;
    value_type *val, *val_im;
} csr;
*/

csr*csr_matrix = csr_load("dist/ash85.mtx", 1); // 1 -> zero_based, 0 -> one_based

...

free_coo(coo_matrix);
free_csr(csr_matrix);
```

#!/bin/bash

# Step 1: 生成 transformer.cpp
echo "Running auto_gen_transformer..."
./scripts/auto_gen_transformer --input ./tests/01_spec.json --output transformer.cpp

# Step 2: 清理過去的編譯檔案
echo "Running make clean..."
make clean

# Step 3: 使用 devtoolset-8 進行編譯
echo "Running make with devtoolset-8..."
scl enable devtoolset-8 'make'

# Step 4: 執行 main 程式，並傳遞參數
echo "Running main with parameters 4000 ./tests/01.in ./tests/01.out..."
time ./main 4000 ./tests/01.in ./tests/01.out

# Step 5: 驗證輸出的結果
echo "Running verify script..."
./scripts/verify --output ./tests/01.out --answer ./tests/01.ans

echo "All steps completed."
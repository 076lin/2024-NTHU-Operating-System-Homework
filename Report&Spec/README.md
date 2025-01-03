# 說明
這裡就是放作業 report 跟 spec 的地方
# 其他我覺得有點用的
## 如何寫 bash 檔幫助你重新編譯與測試
舉例來說

```bash
#!/bin/bash
echo "Running auto_gen_transformer..."
./scripts/auto_gen_transformer --input ./tests/00_spec.json --output transformer.cpp
echo "Running make clean..."
make clean
echo "Running make with devtoolset-8..."
scl enable devtoolset-8 'make'
echo "Running main with parameters 200 ./tests/00.in ./tests/00.out..."
time ./main 200 ./tests/00.in ./tests/00.out
echo "Running verify script..."
./scripts/verify --output ./tests/00.out --answer ./tests/00.ans
echo "All steps completed."
```
### 解說上面 bash 檔大概在幹嘛
首先，你要確認你在哪個檔案裏執行，比如說上面的程式碼是在 NTHU-OS-Pthreads 檔案夾裡面 ( 這個找的到叫 auto_test00.sh 就在我 NTHU-OS-Pthreads 裡面，你可以去看 )

#### 第一行
```bash
#!/bin/bash
```
後來感覺好像不是很重要，因為我不加也能跑，但我查網路資料都叫我加

#### 第二行
```bash
echo "Running auto_gen_transformer..."
```
是用來輸出訊息的，由於執行 bash 檔時並不會像我們一樣輸入指令，所以在終端機介面不會有訊息讓我們掌握執行到什麼指令了 ( 但是指令自帶的輸出還是會有，比如說你 make 那些編譯器執行的輸出還是會有，只是不會有顯示 make 就這樣 ) ，所以適時加入像下面這樣的指令
```bash
echo "你想加(輸出)的消息"
```
這樣你便能比較清楚掌握現在在跑什麼，跑到哪一行了

#### 第五行
```bash
make clean
```
就是你要執行什麼就寫什麼 ( 你在終端機裡怎麼打，就在 bash 檔裡怎麼打 )，像我這樣就是在 NTHU-OS-Pthreads 檔案夾中執行 make clean ( 所以上面跟你說了確認你在哪個檔案中執行 bash 檔有差，要不然你就在前面加 cd 絕對路徑 去進到指定資料夾 )

#### 第九行
```bash
time ./main 200 ./tests/00.in ./tests/00.out
```
這個是執行下面的指令，只是前面加上 time 可以測量下面程式執行時間，那其實 bash 還提供很多功能，你就自己去探索吧
```bash
./main 200 ./tests/00.in ./tests/00.out
```

如何寫註解，就打上 # 之後的東西就變註解，例如 : 
```bash
# 這是註解
```

### 舉個例子吧
下面是 MP2 的指令教學在 MP2 spec 中能找到

![](../picture/6.png)

OK 那通常我們最常需要的是重新編譯，所以你可以在 test 檔案夾裡寫 bash 檔( 我是這樣做，當然你也可以寫在其他的地方，然後這個範例我沒有傳上 github ，不像上面解說的有在其中某個資料夾 ) ，如下

```bash
#!/bin/sh
# 從 test 資料夾進到 build.linux 資料夾，所以如果你不是寫在 test 資料夾，那你得自己看相對路徑，或者絕對路徑應該怎麼寫
cd ../build.linux
# 輸入指令清除舊的編譯後的檔案
make clean
# 重新編譯
make
# 從 build.linux 資料夾進到 test 資料夾
cd ../test
# 執行指定測試
../build.linux/nachos -e consoleIO_test1 -e consoleIO_test2
```
然後你取名假設叫 XX.sh ( XX 是你命的名字 )，在這邊我叫 auto.sh

你要在終端機下指令，這樣你的 bash 檔才能獲取執行權限 ( 現在在 test 資料夾中，因為我寫在 test 資料夾中 )

```bash
chmod +x auto.sh
```

你如果叫不同名字那就是( XX 是你命的名字自行替換 )

```bash
chmod +x XX.sh
```

那最後，你就可以在終端機中( 現在在 test 資料夾中 ) 輸入

```bash
./auto.sh
```

或者你如果叫不同名字那就是( XX 是你命的名字自行替換 )

```bash
./XX.sh
```

這樣你看，你就可以只輸一行指令執行五行指令，幫助你重新編譯並執行測試，不用在那複製貼上，多好，就問你是不是 ( 尤其是你重複修改多次時，我非常有感，嗚嗚，不要罵了，我知道我菜 )

或者，下面這個是本身提供的 ( 在 MP1, MP2, MP3 的 code/test 資料夾裡面可以找到，但只能幫你重新編譯 NachOS 不能執行測試，叫做 build_nachos.sh ，功能非常多，但是要注意的是，他默認是不會 make clean ，所以你想要下指令重新編譯要下 ./build_nachos.sh -c 不然你就自己魔改一下，然後順便加上測試的指令，這樣也行 )

```bash
#!/bin/sh

# Default values for flags
QUIET=false
JOBS=1
CLEAN=false

# Show usage/help message
show_help() {
    echo "Usage: $0 [-q] [-j] [-c] [-h|--help]"
    echo
    echo "Options:"
    echo "  -q        Silence output"
    echo "  -j        Run make with multiple jobs (uses all available cores)"
    echo "  -c        Clean before building"
    echo "  -h, --help Display this help message"
    exit 0
}

# Parse flags
while getopts "qjch-:" opt; do
    case $opt in
        q)
            QUIET=true
            ;;
        j)
            JOBS=$(nproc)  # Use all available cores by default
            ;;
        c)
            CLEAN=true
            ;;
        h)
            show_help
            ;;
        -) # Handle long options
            case "${OPTARG}" in
                help)
                    show_help
                    ;;
                *)
                    echo "Invalid option: --${OPTARG}"
                    exit 1
                    ;;
            esac
            ;;
        *)
            echo "Usage: $0 [-q] [-j] [-c] [-h|--help]"
            exit 1
            ;;
    esac
done

# Move to the build directory
cd ../build.linux || exit

# Clean build if -c flag is set
if [ "$CLEAN" = true ]; then
    echo "Cleaning..."
    if [ "$QUIET" = true ]; then
        make clean > /dev/null 2>&1
    else
        make clean
    fi
fi

# Build nachos
echo "Building with $JOBS job(s)..."
if [ "$QUIET" = true ]; then
    make -j"$JOBS" > /dev/null 2>&1
else
    make -j"$JOBS"
fi

# Check build result
if [ $? -eq 0 ]; then
    echo "Build success"
else
    echo "Build failed"
    exit 1
fi
```

# 評語
Demo: 16
Testcase: 42
Report: 16.3
Feedback:
trace code 部分 : stackallocate, semaphore, switch 可以解釋更詳細, 可以解釋三個 state 的轉換.
# 建議
個人與朋友討論後，感覺是 burst time 的更新有問題，建議你用他的招數(實際去 trace code 試圖釐清狀態變化的我根本小丑)，就是所有 burst time, wait time, Execution time, T 的更新與設定寫在 threads.h 有個 setStatus 函式(建議把實際實作寫在 threads.cc 裡面，這樣很多東西才能調用 threads.h 裡有些東西不好調用)，根據當前 state 與下個 state 去判斷應該做的事，這樣最好實作出來，也比較不容易有問題

雖然我實作錯爛了，但我還是留個紀錄給你，你就自己決定要不要看看吧
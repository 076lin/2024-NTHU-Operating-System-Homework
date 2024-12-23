# 評語
Demo: 16
Testcase: 49
Report: 16.3
Feedback:
trace code 部分 : stackallocate, semaphore, switch 可以解釋更詳細, 可以解釋三個 state 的轉換.
# 建議
個人與朋友討論後，感覺是 L1ReadyList 的實作部分有問題，所以我把他的程式碼幹來了，他說他實作有拿滿，是 NachOS_MP3_correct 那個，具體應該是差在 L1 compare 的地方，我寫錯了，應該是要比 remain burst time ，至於是否還有其他錯誤你可以再看看，或者直接看他的就好

雖然我實作錯爛了，但我還是留個紀錄給你，你就自己決定要不要看看吧，看看也能避免犯一樣的錯，倒也不錯吧
# NachOS_MP3_correct 連結
下面是 NachOS_MP3_correct 程式碼的連結，但我不知道他會不會刪所以複製一個留檔

https://github.com/yeemo116/NachOS_MP3/tree/main
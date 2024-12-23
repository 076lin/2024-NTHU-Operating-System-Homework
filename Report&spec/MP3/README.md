# 評語
Demo: 16
Testcase: 49
Report: 16.3
Feedback:
trace code 部分 : stackallocate, semaphore, switch 可以解釋更詳細, 可以解釋三個 state 的轉換.
# 建議
個人與朋友討論後，感覺是 L1ReadyList 的實作部分有問題，所以我把他的程式碼幹來了，他說他實作有拿滿，是 NachOS_MP3_correct 那個，具體應該是差在 L1 compare 的地方，我寫錯了，應該是要比 remain burst time ，至於是否還有其他錯誤你可以再看看，或者直接看他的就好

雖然我實作錯爛了，但我還是留個紀錄給你，你就自己決定要不要看看吧，看看也能避免犯一樣的錯，倒也不錯吧
# 其他小技巧
寫 code 完後會跑一個自我檢測的檔案，但如果有問題直接跑，會顯示錯誤但難以辨認是哪個 case 錯了，這時我提供下面一個方法

我們當時跑的是 hw3_all.sh 的腳本驗證的，在裡面有一行程式如下，是用來比較你程式輸出與答案是否一致的指令

diff ".tmp/$testcase.txt" "./hw3_ans/${testcase}_ans.txt"

反正就是在 diff 這行程式的最後面加上 -y 就能夠同時輸出，你跑的跟正確答案並列，比較容易辨認錯在哪，改完應該類似下面這樣

diff ".tmp/$testcase.txt" "./hw3_ans/${testcase}_ans.txt" -y
# 評語
報告裡提到的「P(): 能夠讓 thread sleep，禁止進行其他 thread，直到接到 callback」這句不太正確。一個 thread 進入 sleep 狀態之後，會釋放 CPU 給其他 thread 運行，直到收到 callback 後再繼續。
demo: 14
testcase: 45
report: 29

**目前 github 上的 report 裡沒對上面的評語部分做修改，你要參考的話自己要注意跟做修正**

**test case 被扣分好像是因為我當初做 close 函式沒有用 delete 把檔案刪除(目前在 github 上的 report 跟 code 都有做修改了)，還是什麼東西有點忘記了，你可能要注意一下，自行評估正確性**
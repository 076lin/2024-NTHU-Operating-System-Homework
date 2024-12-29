# 評語
Demo: 15
Report: 13.0
Testcase: 60
Feedback: 缺少問題回答

**目前在 github 上的 report 在最後有補問題的部分，自行斟酌正確性**
# 其他提醒
在實作中，有一項要求是實作 MemoryLimitException 在 AddrSpace::Load 中

if(numPages > AddrSpace::remain){
    kernel->machine->RaiseException(MemoryLimitException, 0);
    return FALSE;
}

**上方的程式碼一定要放在下方程式碼的上面，不然你 ASSERT 先呼叫了，你的 NachOS 就會停止，輸出就不會有 MemoryLimitException 的報錯**

ASSERT(numPages <= NumPhysPages);

這個提醒是我有個朋友被扣分後跟我講的，補在 README 這裡提醒一下你
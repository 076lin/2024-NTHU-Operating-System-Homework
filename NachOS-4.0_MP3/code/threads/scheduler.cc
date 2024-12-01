// scheduler.cc
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would
//	end up calling FindNextToRun(), and that would put us in an
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "scheduler.h"

#include "copyright.h"
#include "debug.h"
#include "main.h"
static int compareL1(Thread* t1, Thread* t2){
  if ( t1->getBurstTime() > t2->getBurstTime() ) return 1;
  else if ( t1->getBurstTime() < t2->getBurstTime() ) return -1;
  else return t1->getID() < t2->getID() ? -1 : 1;
  return 0;
}
static int compareL2(Thread* t1, Thread* t2){
  if ( t1->getPriority() > t2->getPriority() ) return -1;
  else if( t1->getPriority() < t2->getPriority() ) return 1;
  else return t1->getID() < t2->getID() ? -1 : 1;
  return 0;
}
//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------

Scheduler::Scheduler() {
    L1ReadyList = new SortedList<Thread *>(compareL1);
    L2ReadyList = new SortedList<Thread *>(compareL2);
    L3ReadyList = new List<Thread *>;
    toBeDestroyed = NULL;
}

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler() {
    delete L1ReadyList;
    delete L2ReadyList;
    delete L3ReadyList;
}

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void Scheduler::ReadyToRun(Thread *thread) {
  ASSERT(kernel->interrupt->getLevel() == IntOff);
  DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());
  // cout << "Putting thread on ready list: " << thread->getName() << endl ;
  thread->setStatus(READY);
  thread->setStartWaitingTime(kernel->stats->totalTicks);
  int PriorityOfThread = thread->getPriority();
  if(PriorityOfThread >= 100 && PriorityOfThread <= 149){
    if( !kernel->scheduler->L1ReadyList->IsInList(thread) ){
      DEBUG('z', "[A] Tick[" << kernel->stats->totalTicks  << "]: Thread [" << thread->getID() << "] is inserted into queue L[1]");
      L1ReadyList->Insert(thread);
    }
  }else if(PriorityOfThread >= 50 && PriorityOfThread <= 99) {
    if( !L2ReadyList->IsInList(thread) ){
      DEBUG('z', "[A] Tick[" << kernel->stats->totalTicks  << "]: Thread [" << thread->getID() << "] is inserted into queue L[2]");
      L2ReadyList->Insert(thread);
    }
  }else if (PriorityOfThread >= 0 && PriorityOfThread <= 49) {
    if( !L3ReadyList->IsInList(thread) ){
      DEBUG('z', "[A] Tick[" << kernel->stats->totalTicks  << "]: Thread [" << thread->getID() << "] is inserted into queue L[3]");
      L3ReadyList->Append(thread);
    }
  }
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun() {
  ASSERT(kernel->interrupt->getLevel() == IntOff);
  if( !L1ReadyList->IsEmpty() ){
    Thread *returnthread = L1ReadyList->RemoveFront();
    returnthread->setStartExecutionTime(kernel->stats->totalTicks);
    DEBUG('z', "[B] Tick [" << kernel->stats->totalTicks  << "]: Thread [" << returnthread->getID() << "] is removed from queue L[1]");
    return returnthread;
  }else if ( !L2ReadyList->IsEmpty() ){
    Thread *returnthread = L2ReadyList->RemoveFront();
    returnthread->setStartExecutionTime(kernel->stats->totalTicks);
    DEBUG('z', "[B] Tick [" << kernel->stats->totalTicks  << "]: Thread [" << returnthread->getID() << "] is removed from queue L[2]");
    return returnthread;
  }else if ( !L3ReadyList->IsEmpty() ){
    Thread *returnthread = L3ReadyList->RemoveFront();
    returnthread->setStartExecutionTime(kernel->stats->totalTicks);
    DEBUG('z', "[B] Tick [" << kernel->stats->totalTicks  << "]: Thread [" << returnthread->getID() << "] is removed from queue L[3]");
    return returnthread;
  }
  return NULL;
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void Scheduler::Run(Thread *nextThread, bool finishing) {
    Thread *oldThread = kernel->currentThread;

    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing) {  // mark that we need to delete current thread
        ASSERT(toBeDestroyed == NULL);
        toBeDestroyed = oldThread;
    }

    if (oldThread->space != NULL) {  // if this thread is a user program,
        oldThread->SaveUserState();  // save the user's CPU registers
        oldThread->space->SaveState();
    }

    oldThread->CheckOverflow();  // check if the old thread
                                 // had an undetected stack overflow

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running

    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
    DEBUG('z',"[E] Tick ["<<kernel->stats->totalTicks<<"]: Thread ["<<nextThread->getID()<<"] is now selected for execution, thread ["
    <<oldThread->getID()<<"] is replaced, and it has executed ["<<oldThread->getExecutionTime(kernel->stats->totalTicks)<<"] ticks");
    // This is a machine-dependent assembly language routine defined
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    SWITCH(oldThread, nextThread);

    // we're back, running oldThread

    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

    CheckToBeDestroyed();  // check if thread we were running
                           // before this one has finished
                           // and needs to be cleaned up

    if (oldThread->space != NULL) {     // if there is an address space
        oldThread->RestoreUserState();  // to restore, do it.
        oldThread->space->RestoreState();
    }
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void Scheduler::CheckToBeDestroyed() {
    if (toBeDestroyed != NULL) {
        delete toBeDestroyed;
        toBeDestroyed = NULL;
    }
}

//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void Scheduler::Print() {
    cout << "Ready list contents:\n";
    L1ReadyList->Apply(ThreadPrint);
    L2ReadyList->Apply(ThreadPrint);
    L3ReadyList->Apply(ThreadPrint);
}

bool Scheduler::ShouldPreempt(Thread* now){
    int p = now->getPriority();
    if(p > 99){
        if(L1ReadyList->IsEmpty()) return false;
        else if(compareL1(now, L1ReadyList->Front()) < 0) return false;
    }else if(p > 49){
        if(!L1ReadyList->IsEmpty()) return true;
	else return false;
    }
    return true;
}


void Scheduler::updatePriority(){
  Statistics *stats = kernel->stats;
  int oldPriority;
  int newPriority;
  int nowTimerTicks = kernel->stats->totalTicks;
  if(!L1ReadyList->IsEmpty()){
    ListIterator<Thread *> *iter1 = new ListIterator<Thread *>(L1ReadyList);
    for( ; !iter1->IsDone(); iter1->Next() ){
      ASSERT( iter1->Item()->getStatus() == READY);
      int waitingtime = kernel->stats->totalTicks - iter1->Item()->getStartWaitingTime();
      if(waitingtime > 1500 && iter1->Item()->getID() > 0 ){
        oldPriority = iter1->Item()->getPriority();
        newPriority = oldPriority + 10;
        if (newPriority > 149) newPriority = 149;
        iter1->Item()->setPriority(newPriority);
        DEBUG('z',"[C] Tick ["<<kernel->stats->totalTicks<<"]: Thread ["<<iter1->Item()->getID()<<"] changes its priority from ["
        <<oldPriority<<"] to ["<<newPriority<<"]");
        L1ReadyList->Remove(iter1->Item());
        ReadyToRun(iter1->Item());
      }
    }
    delete iter1;
  }
  if(!L2ReadyList->IsEmpty()){
    ListIterator<Thread *> *iter2 = new ListIterator<Thread *>(L2ReadyList);
    for( ; !iter2->IsDone(); iter2->Next() ){
      ASSERT( iter2->Item()->getStatus() == READY);
      int waitingtime = kernel->stats->totalTicks - iter2->Item()->getStartWaitingTime();
      if(waitingtime > 1500 && iter2->Item()->getID() > 0 ){
        oldPriority = iter2->Item()->getPriority();
        newPriority = oldPriority + 10;
        iter2->Item()->setPriority(newPriority);
        DEBUG('z',"[C] Tick ["<<kernel->stats->totalTicks<<"]: Thread ["<<iter2->Item()->getID()<<"] changes its priority from ["
        <<oldPriority<<"] to ["<<newPriority<<"]");
        L2ReadyList->Remove(iter2->Item());
        ReadyToRun(iter2->Item());
      }
    }
    delete iter2;
  }
  if(!L3ReadyList->IsEmpty()){
    ListIterator<Thread *> *iter3 = new ListIterator<Thread *>(L3ReadyList);
    for( ; !iter3->IsDone(); iter3->Next() ){
      ASSERT( iter3->Item()->getStatus() == READY);
      int waitingtime = kernel->stats->totalTicks - iter3->Item()->getStartWaitingTime();
      if( waitingtime > 1500 && iter3->Item()->getID() > 0 ){
        oldPriority = iter3->Item()->getPriority();
        newPriority = oldPriority + 10;
        iter3->Item()->setPriority(newPriority);
        DEBUG('z',"[C] Tick ["<<kernel->stats->totalTicks<<"]: Thread ["<<iter3->Item()->getID()<<"] changes its priority from ["
        <<oldPriority<<"] to ["<<newPriority<<"]");
        L3ReadyList->Remove(iter3->Item());
        ReadyToRun(iter3->Item());
      }
    }
    delete iter3;
  }
}

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

static int
L1Compare(Thread *x, Thread *y) {
    double x_t = x->getRemainingTime(kernel->stats->totalTicks);
    double y_t = y->getRemainingTime(kernel->stats->totalTicks);
    if (x_t < y_t) {
        return -1;
    } else if (x_t > y_t) {
        return 1;
    } else {
        return (x->getID() < y->getID() ? -1 : 1);
    }
}

static int
L2Compare(Thread *x, Thread *y) {
    if (x->getPriority() > y->getPriority()) {
        return -1;
    } else if (x->getPriority() < y->getPriority()) {
        return 1;
    } else {
        return (x->getID() < y->getID() ? -1 : 1);
    }
}

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------

Scheduler::Scheduler() {
    // readyList = new List<Thread *>;
    L1 = new SortedList<Thread *>(L1Compare);
    L2 = new SortedList<Thread *>(L2Compare);
    L3 = new List<Thread *>;

    toBeDestroyed = NULL;
}

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler() {
    // delete readyList;
    delete L3;
    delete L2;
    delete L1;
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
    // readyList->Append(thread);
    InsertThreadToReadyList(thread);
    thread->setStartReadyTime(kernel->stats->totalTicks);

    DEBUG(dbgSche, "[A] Tick [" << kernel->stats->totalTicks << "]: Thread ["
                                << thread->getID() << "] is inserted into queue L[" << thread->getQueueLevel() << "]");
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
    // if (readyList->IsEmpty()) {
    //     return NULL;
    // } else {
    //     return readyList->RemoveFront();
    // }
    Thread *t = NULL;
    if (!L1->IsEmpty()) {
        t = L1->RemoveFront();
    } else if (!L2->IsEmpty()) {
        t = L2->RemoveFront();
    } else if (!L3->IsEmpty()) {
        t = L3->RemoveFront();
    }
    if (t != NULL) {
        DEBUG(dbgSche, "[B] Tick [" << kernel->stats->totalTicks << "]: Thread ["
                                    << t->getID() << "] is removed from queue L[" << t->getQueueLevel() << "]");
    }
    return t;
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

    nextThread->setStartRunTime(kernel->stats->totalTicks);

    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());

    DEBUG(dbgSche, "[E] Tick [" << kernel->stats->totalTicks
                                << "]: Thread [" << nextThread->getID() << "] is now selected for execution, thread ["
                                << oldThread->getID() << "] is replaced, and it has executed [" << oldThread->getTempT() << "] ticks");
    
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
    cout << "L1 contents: ";
    L1->Apply(ThreadPrint);
    cout << "L2 contents: ";
    L2->Apply(ThreadPrint);
    cout << "L3 contents: ";
    L3->Apply(ThreadPrint);
    cout << '\n';
}

void Scheduler::InsertThreadToReadyList(Thread *t) {
    if (t->getPriority() < 50) {
        L3->Append(t);
    } else if (t->getPriority() < 100) {
        L2->Insert(t);
    } else if (t->getPriority() < 150) {
        L1->Insert(t);
    }
}

void Scheduler::Aging() {
    List<Thread *> *tempList = new List<Thread *>;

    while (!L3->IsEmpty())
        tempList->Append(L3->RemoveFront());
    while (!L2->IsEmpty())
        tempList->Append(L2->RemoveFront());
    while (!L1->IsEmpty())
        tempList->Append(L1->RemoveFront());

    while (!tempList->IsEmpty()) {
        Thread *t = tempList->RemoveFront();
        int oldLevel = t->getQueueLevel();

        if (kernel->stats->totalTicks - t->getStartReadyTime() > 1500 && t->getPriority() < 149) {
            int oldPriority = t->getPriority();
            t->IncPriority();
            t->setStartReadyTime(kernel->stats->totalTicks);
            DEBUG(dbgSche, "[C] Tick [" << kernel->stats->totalTicks << "]: Thread [" << t->getID()
                                        << "] changes its priority from [" << oldPriority << "] to [" << t->getPriority() << "]");
        }
        InsertThreadToReadyList(t);  // no need to call ReadyToRun() since the thread is already in READY state
        if (oldLevel != t->getQueueLevel()) {
            DEBUG(dbgSche, "[B] Tick [" << kernel->stats->totalTicks << "]: Thread ["
                                        << t->getID() << "] is removed from queue L[" << oldLevel << "]");
            DEBUG(dbgSche, "[A] Tick [" << kernel->stats->totalTicks << "]: Thread ["
                                        << t->getID() << "] is inserted into queue L[" << t->getQueueLevel() << "]");
        }
    }
}

bool Scheduler::Preempt() {
    Thread *t = kernel->currentThread;
    Thread *nextThread = NULL;
    int curQueueLevel = t->getQueueLevel();

    if (!L1->IsEmpty()) {
        // cout << L1->Front()->getID() << " : " << L1->Front()->getRemainingTime() << ", " << t->getID() << " : " << t->getRemainingTime() << '\n';
        if ((curQueueLevel == 1 && L1Compare(L1->Front(), t) == -1) || curQueueLevel == 2 || curQueueLevel == 3)
            return TRUE;

    } else if (!L2->IsEmpty() && curQueueLevel == 3) {
        return TRUE;

    } else if (!L3->IsEmpty() && curQueueLevel == 3 && kernel->stats->totalTicks - t->getStartRunTime() >= 100) {
        return TRUE;
    }

    return FALSE;
}

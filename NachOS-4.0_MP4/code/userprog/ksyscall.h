/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__
#define __USERPROG_KSYSCALL_H__

#include "kernel.h"

#include "synchconsole.h"

void SysHalt()
{
	kernel->interrupt->Halt();
}

int SysAdd(int op1, int op2)
{
	return op1 + op2;
}

#ifdef FILESYS_STUB
int SysCreate(char *filename)
{
	// return value
	// 1: success
	// 0: failed
	return kernel->interrupt->CreateFile(filename);
}
#endif
int sysCreate(char *name, int size){
	return kernel->fileSystem->Create0(name, size);
}
OpenFileId sysOpen(char *name){
	return kernel->fileSystem->Open0(name);
}
int sysRead(char *buf, int size, OpenFileId id){
	return kernel->fileSystem->Read0(buf, size, id);
}
int sysWrite(char *buf, int size, OpenFileId id){
	return kernel->fileSystem->Write0(buf, size, id);
}
int sysClose(OpenFileId id){
	return kernel->fileSystem->Close0(id);
}
#endif /* ! __USERPROG_KSYSCALL_H__ */

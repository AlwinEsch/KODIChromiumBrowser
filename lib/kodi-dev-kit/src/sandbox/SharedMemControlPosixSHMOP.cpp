/*
 *  Copyright (C) 2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "SharedMemControlPosix.h"

#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <iostream>
#include <fstream>

namespace kodi
{
namespace sandbox
{

CSharedMemControlPosix::CSharedMemControlPosix(bool child, const std::string& identifier, size_t size/* = SHARED_DEFAULT_MEM_SIZE*/)
  : CSharedMemControl(child, identifier, size)
{
  fprintf(stderr, "-CSharedMemControlPosix----------------------- %s\n", GetIdentifier().c_str());
  m_filePath = "/tmp/" + GetIdentifier();
  std::fstream fs;
  fs.open(m_filePath, std::ios::out);
  fs.close();
}

CSharedMemControlPosix::~CSharedMemControlPosix()
{
  if (m_creator)
  {
    fprintf(stderr, "-~CSharedMemControlPosix------remove----------------- %s\n", GetIdentifier().c_str());
    remove(m_filePath.c_str());
  }
  fprintf(stderr, "-~CSharedMemControlPosix----------------------- %s\n", GetIdentifier().c_str());
}

bool CSharedMemControlPosix::Create(bool initial)
{
  m_creator = initial;

  key_t key;
  if ((key = ftok(m_filePath.c_str(), 'R')) == -1)
  {
    m_errorText = "ftok() failed: " + std::string(strerror(errno));
    return false;
  }

  fprintf(stderr, "-~CSharedMemControlPosix----------------------- %s %i\n", GetIdentifier().c_str(), key);

  int fd = shmget(key, sizeof(apiShareData), IPC_CREAT | 0666);
//   int fd = shm_open(GetIdentifier().c_str(), initial ? O_CREAT | O_TRUNC | O_RDWR : O_RDWR, 0666);
  if (fd == -1)
  {
    m_errorText = "Shared memory failed to open";
    return false;
  }

  m_fd = fd;

//   if (initial)
//   {
//     if (ftruncate(m_fd, sizeof(apiShareData)) == -1)
//     {
//       m_errorText = "Shared memory size set failed";
//       Destroy();
//       return false;
//     }
//   }

  apiShareData* mem = static_cast<apiShareData*>(shmat(m_fd, nullptr, 0));

//   apiShareData* mem = static_cast<apiShareData*>(mmap(nullptr, sizeof(apiShareData),
//                                                       PROT_READ | PROT_WRITE,
//                                                       MAP_SHARED | MAP_POPULATE, fd, 0));
  if (mem == MAP_FAILED)
  {
    m_errorText = "Shared memory size set failed";
    Destroy();
    return false;
  }

  m_sharedMem = mem;

  if (initial)
  {
    memset(mem, 0, sizeof(apiShareData));

    if (sem_init(&m_sharedMem->sem_segment_caller, 1, 0) == -1)
    {
      m_errorText = "Semaphores caller init failed";
      Destroy();
      return false;
    }
    if (sem_init(&m_sharedMem->sem_segment_target, 1, 0) == -1)
    {
      m_errorText = "Semaphores target init failed";
      Destroy();
      return false;
    }

    m_sharedMem->inited = true;
  }


  return true;
}

bool CSharedMemControlPosix::Destroy()
{
  sem_destroy(&m_sharedMem->sem_segment_caller);
  sem_destroy(&m_sharedMem->sem_segment_target);
  m_sharedMem->inited = false;

  munmap(m_sharedMem, sizeof(apiShareData));
  m_sharedMem = nullptr;

  int fd = shmdt(m_sharedMem);
  //int fd = shm_unlink(GetIdentifier().c_str());
  if (fd == -1)
  {
    m_errorText = "Shared memory failed to unlink";
    return false;
  }

  m_fd = -1;

  return true;
}

bool CSharedMemControlPosix::Lock_Caller()
{
  int err = sem_wait(&m_sharedMem->sem_segment_caller);
  if (err != 0)
  {
    m_errorText = "Caller lock failed";
    return false;
  }

  return true;
}

void CSharedMemControlPosix::Unlock_Caller()
{
  sem_post(&m_sharedMem->sem_segment_caller);
}

bool CSharedMemControlPosix::Lock_Target()
{
  int err = sem_wait(&m_sharedMem->sem_segment_target);
  if (err != 0)
  {
    m_errorText = "Target lock failed";
    return false;
  }

  return true;
}

void CSharedMemControlPosix::Unlock_Target()
{
  sem_post(&m_sharedMem->sem_segment_target);
}

} /* namespace sandbox */
} /* namespace kodi */

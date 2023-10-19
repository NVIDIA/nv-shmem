#pragma once

#include <boost/interprocess/sync/named_upgradable_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <memory>
#include <shm_common.h>
#include <stdexcept>

using namespace std;

namespace nv {

namespace shmem {

using shmem_write_lock_t = boost::interprocess::scoped_lock<
    boost::interprocess::named_upgradable_mutex>;
using shmem_read_lock_t = boost::interprocess::sharable_lock<
    boost::interprocess::named_upgradable_mutex>;

/**
 * @brief Wrapper class which provides functionality of boost shared memory
 * initialization, cleanup and locks around read operation
 *
 */
class ManagedShmem {
public:
  ManagedShmem(const string &nameSpace, const int opts, size_t maxSize);
  ManagedShmem(const string &nameSpace, const int opts);
  virtual ~ManagedShmem() = default;
  /**
   * @brief Read lock implementation to read values from shared memory.
   *
   */
  void TryReadLock();

protected:
  unique_ptr<boost::interprocess::managed_shared_memory> memory;
  unique_ptr<void_allocator_t> voidAllocator;
  unique_ptr<boost::interprocess::named_upgradable_mutex> memLock;
  const int opts;
  string nameSpace;
};

struct LockAcquisitionException : public runtime_error {
  LockAcquisitionException()
      : runtime_error("Failed to acquire the lock within the timeout") {}
};

struct BadMapException : public runtime_error {
  BadMapException() : runtime_error("Map object is null") {}
};

struct PermissionErrorException : public runtime_error {
  PermissionErrorException() : runtime_error("Permission denied") {}
};

} // namespace shmem
} // namespace nv

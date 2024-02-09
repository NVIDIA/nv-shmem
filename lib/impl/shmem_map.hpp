#pragma once
#include "managed_shmem.hpp"
#include <boost/interprocess/containers/map.hpp>
#include <memory>
#include <shm_common.h>
#include <vector>

using namespace std;

namespace nv {

namespace shmem {

using map_value_type_t = pair<const char_string_t, SensorMapValue>;
using map_value_type_allocator_t =
    boost::interprocess::allocator<map_value_type_t, segment_manager_t>;
using SensorMap =
    boost::interprocess::map<char_string_t, SensorMapValue, less<char_string_t>,
                             map_value_type_allocator_t>;

/** @class Map
 *  @brief Shared Memory Implementation for object type Map. ManagedShmem
 * inherited for shared memory initialization functionality and read lock.
 *  @details This is templated class which implements a map type in shared
 * memory. Requires two template parameters
 *  MapType: Specifies the internal layout and allocator for the map creation
 *  ValueType: Specifies the value type for the object insertion and retrieval
 */
template <class MapType, class ValueType> class Map : public ManagedShmem {
public:
  /** @brief Ctor
   *  @param[in] nameSpace - Unique name of the map
   *  @param[in] opts - Read/Write permissions
   *  @param[in] maxSize - memory size allocation for the map
   */
  Map(const string &nameSpace, const int opts, size_t maxSize);
  /** @brief Ctor
   *  @param[in] nameSpace - Unique name of the map
   *  @param[in] opts - Read permissions
   */
  Map(const string &nameSpace, const int opts);
  /** @brief Dtor, remove the shared map object
   */
  ~Map();

  /** @brief Get all the objects present in the map
   *  @return vector of objects
   */
  vector<ValueType> getAllValues();

  /** @brief Get value of single object
   *  @param[in] key - key of the which which must be retrieved
   *  @param[in] val - object reference where the found object will be
   * copied
   *  @return True if object with matching key is found. False if the key is
   * not found
   */
  bool getValue(const string &key, ValueType &val);

  /** @brief Insert object in the map, if the key is already present then
   * object would be updated
   *  @param[in] key - key of the object to be created
   *  @param[in] val - object to be inserted
   */
  void insert(const string &key, const ValueType &val);

  /** @brief Remove object from the map
   *  @param[in] key - key of the object to be removed
   */
  void erase(const string &key) {
    if (opts & O_CREAT) {
      shmem_write_lock_t lock(*memLock);
      mapImpl->erase(getMapKey(key));
    } else {
      throw PermissionErrorException();
    }
  }

  /** @brief Remove all objects from the map
   */
  void clear(void) {
    if (opts & O_CREAT) {
      shmem_write_lock_t lock(*memLock);
      mapImpl->clear();
    } else {
      throw PermissionErrorException();
    }
  }

  /** @brief Returns number of elements in the map
   */
  auto size(void) { return mapImpl->size(); }

  /** @brief Update value property of the object
   *  @param[in] key - key of the map object
   *  @param[in] val - value property of the object
   *  @return True if value property is updated successfully. False if the key
   * is not found
   */
  bool updateValue(const string &key, const string &val);

  /** @brief Update timestamp property of the object
   *  @param[in] key - key of the map object
   *  @param[in] timestamp - timestamp property of the object
   *  @param[in] timestampStr - datetime in ms format
   *  @return True if timestamp property is updated successfully. False if the
   * key is not found
   */
  bool updateTimestamp(const string &key, const uint64_t timestamp,
                       const string &timestampStr);

  /** @brief Update value property of the object
   *  @param[in] key - key of the map object
   *  @param[in] val - value property of the object
   *  @param[in] timestamp - timestamp property of the object
   *  @param[in] timestampStr - datetime in ms format
   *  @return True if value property is updated successfully. False if the key
   * is not found
   */
  bool updateValueAndTimeStamp(const string &key, const string &val,
                               const uint64_t timestamp,
                               const string &timestampStr);
  /**
   * @brief Method to get free memory available in the namespace.
   *
   * @return size_t - free memory available in bytes
   */
  size_t getFreeSize() {
    size_t freeSize = memory->get_free_memory();
    return freeSize;
  }

private:
  /** @brief Get the key in shared mem allocator format
   *  @param[in] key - key in string format
   */
  char_string_t getMapKey(const string &key) {
    char_string_t mapKey(*voidAllocator);
    mapKey = key;
    return mapKey;
  }

  /** @brief Boost Internal implementation reference */
  boost::interprocess::offset_ptr<MapType> mapImpl;
};

} // namespace shmem
} // namespace nv

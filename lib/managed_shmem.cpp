/*
 * SPDX-FileCopyrightText: Copyright (c) 2023-2024 NVIDIA CORPORATION &
 * AFFILIATES. All rights reserved. SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "impl/managed_shmem.hpp"

#include "boost/date_time/posix_time/posix_time_types.hpp"

#include <boost/interprocess/containers/map.hpp>
#include <phosphor-logging/lg2.hpp>

#include <chrono>
#include <stdexcept>

using namespace std;
using namespace nv::shmem;

ManagedShmem::ManagedShmem(const string& nameSpace, const int opts,
                           size_t maxSize) : opts(opts), nameSpace(nameSpace)
{
    if (!boost::interprocess::shared_memory_object::remove(
            string(nameSpace).c_str()))
    {
        lg2::info(
            "SHMEMDEBUG: Shared memory namespace {SHM_NAMESPACE} does not exist. "
            "Remove is skipped.",
            "SHM_NAMESPACE", nameSpace);
    }
    memory = make_unique<boost::interprocess::managed_shared_memory>(
        boost::interprocess::open_or_create, nameSpace.c_str(), maxSize);

    if (!boost::interprocess::named_upgradable_mutex::remove(
            string(nameSpace + "lock").c_str()))
    {
        lg2::info(
            "SHMEMDEBUG: Shared memory namespace lock {SHM_NAMESPACE_LOCK} "
            "does not "
            "exist. Remove lock is skipped.",
            "SHM_NAMESPACE_LOCK", nameSpace + "lock");
    }
    memLock = make_unique<boost::interprocess::named_upgradable_mutex>(
        boost::interprocess::create_only, string(nameSpace + "lock").c_str());
    memLock->unlock();

    voidAllocator =
        make_unique<void_allocator_t>(memory->get_segment_manager());
}

ManagedShmem::ManagedShmem(const string& nameSpace, const int opts) :
    opts(opts), nameSpace(nameSpace)
{
    memory = make_unique<boost::interprocess::managed_shared_memory>(
        boost::interprocess::open_only, nameSpace.c_str());
    memLock = make_unique<boost::interprocess::named_upgradable_mutex>(
        boost::interprocess::open_only, string(nameSpace + "lock").c_str());
    voidAllocator =
        make_unique<void_allocator_t>(memory->get_segment_manager());
}

void ManagedShmem::TryReadLock()
{
    boost::posix_time::ptime abs_time =
        boost::posix_time::microsec_clock::universal_time() +
        boost::posix_time::seconds(1);
    shmem_read_lock_t lock(*memLock, abs_time);
    if (!lock)
    {
        throw LockAcquisitionException();
    }
}

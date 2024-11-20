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

#include "config.h"

#include "impl/error_logger.hpp"

#include <phosphor-logging/lg2.hpp>

// Singleton accessor
ErrorLogger& ErrorLogger::getInstance()
{
    static ErrorLogger instance;
    return instance;
}

void ErrorLogger::logError(const std::string& errorMessage)
{
    // Get the current time
    auto currentTime = getCurrentTime();

    // Check if this error has been logged recently
    auto it = errorLogTimes.find(errorMessage);
    if (it != errorLogTimes.end())
    {
        auto lastLogTime = it->second;
        auto timeSinceLastLog =
            std::chrono::duration_cast<std::chrono::seconds>(currentTime -
                                                             lastLogTime)
                .count();

        // If the error was logged less than LOG_INTERVAL_SECONDS ago, skip
        // logging
        if (timeSinceLastLog < LOG_INTERVAL_SECONDS)
        {
            return;
        }
    }
    else
    {
        if (errorLogTimes.size() >= MAX_LOG_ENTRIES)
        {
            return;
        }
        // Add the new error message to the map with the current time
        errorLogTimes[errorMessage] = currentTime;
    }

    // Log the error and update the last log time
    lg2::error("{ERROR_STRING}", "ERROR_STRING", errorMessage);
}

// Function to get the current time
std::chrono::steady_clock::time_point ErrorLogger::getCurrentTime() const
{
    return std::chrono::steady_clock::now();
}

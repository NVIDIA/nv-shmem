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

#pragma once
#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>

class ErrorLogger
{
  public:
    // Singleton accessor
    static ErrorLogger& getInstance();

    /**
     * @brief Logs an error message with suppression if it has already been
     * logged recently. This method logs the error only if the same error
     * message hasn't been logged within the last LOG_INTERVAL_SECONDS seconds.
     *
     * @param[in] errorMessage - The error message string to log
     */
    void logError(const std::string& errorMessage);

  private:
    ErrorLogger() = default;
    ~ErrorLogger() = default;
    ErrorLogger(const ErrorLogger&) = delete;
    ErrorLogger& operator=(const ErrorLogger&) = delete;

    // Map to store the timestamp of the last log for each error message
    std::unordered_map<std::string, std::chrono::steady_clock::time_point>
        errorLogTimes;

    // Function to get the current timestamp
    std::chrono::steady_clock::time_point getCurrentTime() const;
};

// Macro to simplify logging
#define LOG_ERROR(message) ErrorLogger::getInstance().logError(message)

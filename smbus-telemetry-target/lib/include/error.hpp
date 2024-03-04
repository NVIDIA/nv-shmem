/**
 * Copyright (c) 2023,2024 NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#pragma once

// Error Codes
namespace ErrorCode
{

constexpr int \
    SucessCode         = 0x0000, \
    ConfigFileNotFound = 0x0100, \
    InvalidConfigData  = 0x0101, \
    FaildToLoadCSV     = 0x0102, \
    SMBusUpdateDataNotFound = 0x0200, \
    SMBusSysfsPathNotFound = 0x0201;
    
} // ErrorCode
/*
// Copyright (c) 2019 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#pragma once

enum class IPMINetfnMultiNodeCmd
{
    cmdGetMultiNodeRole = 0x33,
    cmdGetMultiNodeId = 0x36,
    cmdGetMultiNodePresence = 0x63,
};

static constexpr const char* multiNodeObjPath =
    "/xyz/openbmc_project/MultiNode/Status";
static constexpr const char* multiNodeIntf =
    "xyz.openbmc_project.Chassis.MultiNode";

enum class NodeRole : uint8_t
{
    single,
    master,
    slave,
    arbitrating
};
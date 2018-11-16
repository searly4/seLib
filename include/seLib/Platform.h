#pragma once
/*
   Copyright 2018 by Scott Early

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifdef __cplusplus
namespace seLib {
namespace Platform {

#if defined(NRF52)
#define SELIB_PLATFORM NRF52
#include <seLib/experimental/Platform_NRF52.h>
#elif defined(SELIB_PLATFORM)
#include <Platform_#SELIB_PLATFORM#.h>
#endif
namespace seLib {
namespace Platform {

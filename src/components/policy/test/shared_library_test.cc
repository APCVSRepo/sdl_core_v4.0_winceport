/* Copyright (c) 2014, Ford Motor Company
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ford Motor Company nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#if defined(OS_WIN32) || defined(OS_WINCE)
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include "gtest/gtest.h"

namespace test {
namespace components {
namespace policy {

::testing::AssertionResult IsError(void* error) {
  if (error) {
    return ::testing::AssertionSuccess() << static_cast<const char*>(error);
  } else {
    return ::testing::AssertionFailure() << error;
  }
}

TEST(SharedLibraryTest, FullTest_OpenLibrarySetSymbolCloseLibrary_ExpectActsWithoutErrors) {
#if defined(OS_WIN32)
    //Arrange
    const std::string kLib = "..\\src\\policy\\Policy.dll";
    HINSTANCE handle = LoadLibrary(kLib.c_str());

    //Assert
    EXPECT_FALSE(IsError((void*)GetLastError()));
    ASSERT_TRUE(handle);

    //Act
    const std::string kSymbol = "CreateManager";
    void* symbol = GetProcAddress(handle, kSymbol.c_str());

    //Assert
    EXPECT_FALSE(IsError((void*)GetLastError()));
    EXPECT_TRUE(symbol);
#elif defined(OS_WINCE)
    //Arrange
    const std::wstring kLib = L"..\\src\\policy\\Policy.dll";
    HINSTANCE handle = LoadLibrary(kLib.c_str());

    //Assert
    EXPECT_FALSE(IsError((void*)GetLastError()));
    ASSERT_TRUE(handle);

    //Act
    const std::wstring kSymbol = L"CreateManager";
    void* symbol = GetProcAddress(handle, kSymbol.c_str());

    //Assert
    EXPECT_FALSE(IsError((void*)GetLastError()));
    EXPECT_TRUE(symbol);
#else
  //Arrange
  const std::string kLib = "../src/policy/libPolicy.so";
  void* handle = dlopen(kLib.c_str(), RTLD_LAZY);

  //Assert
  EXPECT_FALSE(IsError(dlerror()));
  ASSERT_TRUE(handle);

  //Act
  const std::string kSymbol = "CreateManager";
  void* symbol = dlsym(handle, kSymbol.c_str());

  //Assert
  EXPECT_FALSE(IsError(dlerror()));
  EXPECT_TRUE(symbol);

  //Act
  int ret = dlclose(handle);

  //Assert
  EXPECT_FALSE(ret);
  EXPECT_FALSE(IsError(dlerror()));
#endif
}

}  // namespace policy
}  // namespace components
}  // namespace test

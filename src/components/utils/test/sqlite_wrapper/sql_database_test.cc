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

#include "gtest/gtest.h"
#include "sqlite_wrapper/sql_error.h"
#include "sqlite_wrapper/sql_database.h"

#if defined(OS_WIN32) || defined(OS_WINCE)
#include "utils/file_system.h"
#endif

using ::utils::dbms::SQLError;
using ::utils::dbms::SQLDatabase;

namespace test {
namespace components {
namespace utils {
namespace dbms {

#ifdef OS_WINCE
bool chmod(const std::string& fileName, int iFlag)
{
	//gy_int32 iTemp;
	std::string absFileName;
	std::wstring abswFileName;

	int fileAttri = 0;
	if (iFlag < 1)
	{
		printf("FUNCTION: %s, LINE: %d, iFlag = %d\n", __FUNCTION__, __LINE__, iFlag);
		return false;
	}
	absFileName = Global::RelativePathToAbsPath(fileName);
	abswFileName = Global::StringToWString(absFileName);
	fileAttri = GetFileAttributesW(abswFileName.c_str());
	if(fileAttri == -1)
	{
		int iError = GetLastError();
		printf("FUNCTION: %s, LINE: %d, iError = %d\n", __FUNCTION__, __LINE__, iError);
		return false;
	}

	if (iFlag == 0x0400)
	{
		fileAttri |= 0x0001;
	}
	if (SetFileAttributesW(abswFileName.c_str(), fileAttri) != 0)
	{
		return true;
	}
	int iError = GetLastError();
	printf("FUNCTION: %s, LINE: %d, iError = %d\n", __FUNCTION__, __LINE__, iError);
	return false;
}
#endif

::testing::AssertionResult IsError(SQLError error) {
  if (error.number() != ::utils::dbms::OK) {
    return ::testing::AssertionSuccess() << error.text();
  } else {
    return ::testing::AssertionFailure() << error.text();
  }
}

TEST(SQLDatabaseTest, OpenCloseMemory_OpenAndCloseDB_ActsWithoutError) {

  //arrange
  SQLDatabase db;
  bool ret = db.Open();

  //assert
  EXPECT_FALSE(IsError(db.LastError()));
  ASSERT_TRUE(ret);

  //act
  db.Close();

  //assert
  EXPECT_FALSE(IsError(db.LastError()));
}

TEST(SQLDatabaseTest, OpenCloseFile_OpenAndCloseSpecifiedDB_ActsWithoutError) {

  //arrange
  SQLDatabase db("test-database");
  bool ret = db.Open();

  //assert
  EXPECT_FALSE(IsError(db.LastError()));
  ASSERT_TRUE(ret);

  //act
  db.Close();

  //assert
  EXPECT_FALSE(IsError(db.LastError()));

#if defined(OS_WIN32) || defined(OS_WINCE)
  file_system::DeleteFileWindows("test-database.sqlite");
#else
  remove("test-database.sqlite");
#endif
}

TEST(SQLDatabaseTest, OpenDBTwice_NoError) {

  //arrange
  SQLDatabase db;
  bool ret = db.Open();

  //assert
  EXPECT_FALSE(IsError(db.LastError()));
  ASSERT_TRUE(ret);

  //act
  ret = db.Open();

  //assert
  EXPECT_FALSE(IsError(db.LastError()));
  ASSERT_TRUE(ret);

  db.Close();
}

TEST(SQLDatabaseTest, CloseDBTwice_NoError) {

  //arrange
  SQLDatabase db;
  bool ret = db.Open();

  //assert
  EXPECT_FALSE(IsError(db.LastError()));
  ASSERT_TRUE(ret);

  //act
  db.Close();

  //assert
  EXPECT_FALSE(IsError(db.LastError()));

  //act
  db.Close();

  //assert
  EXPECT_FALSE(IsError(db.LastError()));
}

TEST(SQLDatabaseTest, Close_DBWasNotOpened_NoError) {

  //act
  SQLDatabase db;
  db.Close();

  //assert
  EXPECT_FALSE(IsError(db.LastError()));
}

TEST(SQLDatabaseTest, CommitTransaction_StartAndCommitTransaction_ExpectActsWithoutError) {

  //arrange
  SQLDatabase db;

  //assert
  ASSERT_TRUE(db.Open());
  EXPECT_TRUE(db.BeginTransaction());
  EXPECT_FALSE(IsError(db.LastError()));
  EXPECT_TRUE(db.CommitTransaction());
  EXPECT_FALSE(IsError(db.LastError()));

  db.Close();
}

TEST(SQLDatabaseTest, RollbackTransaction_StartAndRollbackTransaction_ExpectActsWithoutError) {

  //arrange
  SQLDatabase db;

  //assert
  ASSERT_TRUE(db.Open());
  EXPECT_TRUE(db.BeginTransaction());
  EXPECT_FALSE(IsError(db.LastError()));
  EXPECT_TRUE(db.RollbackTransaction());
  EXPECT_FALSE(IsError(db.LastError()));

  db.Close();
}

TEST(SQLDatabaseTest, FailedCommitTransaction_CommitTransactionWithoutBeginning_ExpectError) {

  //arrange
  SQLDatabase db;

  //assert
  ASSERT_TRUE(db.Open());
  EXPECT_FALSE(db.CommitTransaction());
  EXPECT_TRUE(IsError(db.LastError()));

  db.Close();
}

TEST(SQLDatabaseTest, FailedRollbackTransaction_RollbackTransactionWithoutBeginning_ExpectError) {

  //arrange
  SQLDatabase db;

  //assert
  ASSERT_TRUE(db.Open());
  EXPECT_FALSE(db.RollbackTransaction());
  EXPECT_TRUE(IsError(db.LastError()));

  db.Close();
}

TEST(SQLDatabaseTest, BadTransaction_BeginTransitionWithoutOpenDB_ExpectError) {

  //arrange
  SQLDatabase db;

  //assert
  EXPECT_FALSE(db.BeginTransaction());
  EXPECT_TRUE(IsError(db.LastError()));
}

TEST(SQLDatabaseTest, IsReadWrite_FirstOpenDBIsRWSecondIsNot) {

  //arrange
  SQLDatabase db("test-database");

  //assert
  ASSERT_TRUE(db.Open());
  EXPECT_TRUE(db.IsReadWrite());
  db.Close();
#if defined(OS_WIN32)
  chmod("test-database.sqlite", 0x00400);
#elif defined(OS_WINCE)
  // TODO:
  // chmod("test-database.sqlite", 0x00400);
  chmod("test-database.sqlite", 0x00400);
#else
  chmod("test-database.sqlite", S_IRUSR);
#endif
  //assert
  ASSERT_TRUE(db.Open());
  EXPECT_FALSE(db.IsReadWrite());

  db.Close();
#if defined(OS_WIN32) || defined(OS_WINCE)
  file_system::DeleteFileWindows("test-database.sqlite");
#else
  remove("test-database.sqlite");
#endif
}

}  // namespace dbms
}  // namespace utils
}  // namespace components
}  // namespace test

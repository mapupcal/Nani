#include <gtest/gtest.h>
#include "runtime/directory.h"
#include <filesystem>
using namespace nani::runtime;

class DirectoryTest : public ::testing::Test
{
protected:
	void SetUp() override
	{

	}

	void TearDown() override
	{

	}
};

TEST_F(DirectoryTest, FromFileUsesParentDirectory)
{
	const auto cwd = std::filesystem::current_path();
	const auto filePath = (cwd / "dummy.txt").u8string();
	const Directory dir = Directory::FromFile(filePath);

	EXPECT_TRUE(dir.IsAbsolute());
	EXPECT_TRUE(std::filesystem::equivalent(
		std::filesystem::path(dir.AbsolutePath()),
		cwd));
	EXPECT_TRUE(dir.FilePath(u8"assets/styles.xml").ends_with(u8"assets/styles.xml"));
}

TEST_F(DirectoryTest, FromFileEmptyFallsBackToCwd)
{
	const Directory dir = Directory::FromFile(u8"");
	EXPECT_TRUE(dir.AbsolutePath() == std::filesystem::absolute(std::filesystem::current_path()).u8string());
}

TEST_F(DirectoryTest, Basic)
{
	{
		Directory dir(u8".");
		EXPECT_TRUE(dir.Exists());
		EXPECT_FALSE(dir.IsAbsolute());
		EXPECT_TRUE(dir.AbsolutePath() == std::filesystem::absolute(std::filesystem::current_path()).u8string());
		EXPECT_TRUE(dir.CreateDirectory(u8"./test"));
		EXPECT_TRUE(dir.Exists(u8"test"));
		EXPECT_TRUE(dir.Exists(u8"./test"));
		EXPECT_TRUE(dir.CD(u8"test"));
		EXPECT_TRUE(dir.CD(u8".."));
		EXPECT_TRUE(dir.AbsolutePath() == std::filesystem::absolute(std::filesystem::current_path()).u8string());
		EXPECT_TRUE(dir.RemoveDirectory(u8"test"));
		EXPECT_FALSE(dir.CD(u8"test"));

		EXPECT_TRUE(dir.CreateDirectory(u8"./test"));
		EXPECT_TRUE(dir.CD(u8"test"));
		EXPECT_TRUE(dir.CreateDirectory(u8"./test1"));
		EXPECT_TRUE(dir.CreateDirectory(u8"./test2"));
		auto paths = dir.EnumDirectories();
		EXPECT_EQ(paths.size(), 2);
		EXPECT_TRUE(dir.CD(u8".."));
		EXPECT_TRUE(dir.RemoveDirectory(u8"test"));

		EXPECT_TRUE(dir.CreateDirectory(u8"./test"));
		EXPECT_TRUE(dir.CD(u8"test"));
		EXPECT_TRUE(dir.RemoveRecursively());
		EXPECT_FALSE(dir.Exists());
		EXPECT_TRUE(dir.Exists(u8".."));
		EXPECT_TRUE(dir.CD(u8".."));
	}
}

#include <gtest/gtest.h>
#include "runtime/directory.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

using namespace nani::runtime;
namespace fs = std::filesystem;

namespace
{
	std::atomic<uint64_t> g_tempSeq{ 0 };

	std::u8string ToU8(const fs::path& path)
	{
		return path.u8string();
	}

	bool SamePath(const std::u8string& lhs, const std::u8string& rhs)
	{
		std::error_code ec;
		const fs::path a(lhs);
		const fs::path b(rhs);
		if (fs::exists(a, ec) && fs::exists(b, ec) && !ec)
			return fs::equivalent(a, b, ec);

		ec.clear();
		const fs::path ca = fs::weakly_canonical(a, ec);
		const fs::path cb = fs::weakly_canonical(b, ec);
		return ca == cb;
	}

	void WriteFile(const fs::path& path, const std::string& content = "x")
	{
		fs::create_directories(path.parent_path());
		std::ofstream out(path, std::ios::binary);
		ASSERT_TRUE(static_cast<bool>(out)) << path.string();
		out << content;
	}

	bool ContainsPath(const std::vector<std::u8string>& paths, const fs::path& expected)
	{
		return std::any_of(paths.begin(), paths.end(), [&](const std::u8string& item) {
			return SamePath(item, ToU8(expected));
		});
	}

	class CwdGuard
	{
	public:
		explicit CwdGuard(const fs::path& next)
			: m_previous(fs::current_path())
		{
			fs::current_path(next);
		}

		~CwdGuard()
		{
			std::error_code ec;
			fs::current_path(m_previous, ec);
		}

	private:
		fs::path m_previous;
	};
}

class DirectoryTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
		m_root = fs::temp_directory_path() / "nani_directory_utest" /
			(std::to_string(stamp) + "_" + std::to_string(g_tempSeq.fetch_add(1)));
		std::error_code ec;
		fs::remove_all(m_root, ec);
		ASSERT_TRUE(fs::create_directories(m_root, ec)) << ec.message();
	}

	void TearDown() override
	{
		std::error_code ec;
		fs::remove_all(m_root, ec);
	}

	fs::path RootPath(const fs::path& relative = {}) const
	{
		return relative.empty() ? m_root : (m_root / relative);
	}

	Directory RootDir() const
	{
		return Directory(ToU8(m_root));
	}

	fs::path m_root;
};

TEST_F(DirectoryTest, DefaultConstructorPointsToCwd)
{
	const Directory dir;
	const auto cwd = fs::current_path();

	EXPECT_TRUE(dir.IsAbsolute());
	EXPECT_TRUE(dir.Exists());
	EXPECT_TRUE(SamePath(dir.AbsolutePath(), ToU8(cwd)));
}

TEST_F(DirectoryTest, RelativeConstructorReportsRelativeAndResolvesAbsolute)
{
	Directory dir(u8".");
	EXPECT_FALSE(dir.IsAbsolute());
	EXPECT_TRUE(dir.Exists());
	EXPECT_TRUE(SamePath(dir.AbsolutePath(), ToU8(fs::current_path())));
}

TEST_F(DirectoryTest, AbsoluteConstructorKeepsAbsolutePath)
{
	const Directory dir = RootDir();
	EXPECT_TRUE(dir.IsAbsolute());
	EXPECT_TRUE(dir.Exists());
	EXPECT_TRUE(SamePath(dir.AbsolutePath(), ToU8(m_root)));
}

TEST_F(DirectoryTest, FromFileReturnsParentOfAbsoluteFile)
{
	const fs::path file = RootPath("nested/demo.txt");
	WriteFile(file);

	const Directory dir = Directory::FromFile(ToU8(file));
	EXPECT_TRUE(dir.IsAbsolute());
	EXPECT_TRUE(SamePath(dir.AbsolutePath(), ToU8(file.parent_path())));
}

TEST_F(DirectoryTest, FromFileResolvesRelativeFileAgainstCwd)
{
	WriteFile(RootPath("rel.txt"));
	const CwdGuard cwd(m_root);

	const Directory dir = Directory::FromFile(u8"rel.txt");
	EXPECT_TRUE(SamePath(dir.AbsolutePath(), ToU8(m_root)));
}

TEST_F(DirectoryTest, FromFileEmptyFallsBackToCwd)
{
	const Directory dir = Directory::FromFile(u8"");
	EXPECT_TRUE(SamePath(dir.AbsolutePath(), ToU8(fs::absolute(fs::current_path()))));
}

TEST_F(DirectoryTest, ExistsForSelfAndRelativeChild)
{
	Directory dir = RootDir();
	EXPECT_TRUE(dir.Exists());
	EXPECT_FALSE(dir.Exists(u8"missing"));

	ASSERT_TRUE(dir.CreateDirectory(u8"child"));
	EXPECT_TRUE(dir.Exists(u8"child"));
	EXPECT_TRUE(dir.Exists(u8"./child"));
}

TEST_F(DirectoryTest, ExistsAcceptsAbsolutePath)
{
	const fs::path child = RootPath("abs_child");
	fs::create_directories(child);

	Directory dir = RootDir();
	EXPECT_TRUE(dir.Exists(ToU8(child)));
	EXPECT_FALSE(dir.Exists(ToU8(RootPath("nope"))));
}

TEST_F(DirectoryTest, CreateDirectoryRelativeIsIdempotent)
{
	Directory dir = RootDir();
	EXPECT_TRUE(dir.CreateDirectory(u8"a/b"));
	EXPECT_TRUE(dir.Exists(u8"a"));
	EXPECT_TRUE(dir.Exists(u8"a/b"));
	EXPECT_TRUE(dir.CreateDirectory(u8"a/b"));
}

TEST_F(DirectoryTest, CreateDirectoryAbsolute)
{
	const fs::path target = RootPath("absolute_created");
	Directory dir = RootDir();
	EXPECT_TRUE(dir.CreateDirectory(ToU8(target)));
	EXPECT_TRUE(fs::is_directory(target));
	EXPECT_TRUE(dir.CreateDirectory(ToU8(target)));
}

TEST_F(DirectoryTest, CDRelativeAndAbsolute)
{
	Directory dir = RootDir();
	ASSERT_TRUE(dir.CreateDirectory(u8"one/two"));

	EXPECT_TRUE(dir.CD(u8"one"));
	EXPECT_TRUE(SamePath(dir.AbsolutePath(), ToU8(RootPath("one"))));

	EXPECT_TRUE(dir.CD(u8"two"));
	EXPECT_TRUE(SamePath(dir.AbsolutePath(), ToU8(RootPath("one/two"))));

	EXPECT_TRUE(dir.CD(u8".."));
	EXPECT_TRUE(SamePath(dir.AbsolutePath(), ToU8(RootPath("one"))));

	EXPECT_TRUE(dir.CD(ToU8(m_root)));
	EXPECT_TRUE(SamePath(dir.AbsolutePath(), ToU8(m_root)));
}

TEST_F(DirectoryTest, CDFailsForMissingPath)
{
	Directory dir = RootDir();
	const std::u8string before = dir.AbsolutePath();

	EXPECT_FALSE(dir.CD(u8"does_not_exist"));
	EXPECT_TRUE(SamePath(dir.AbsolutePath(), before));
}

TEST_F(DirectoryTest, FilePathJoinsBaseAndName)
{
	const Directory dir = RootDir();
	const std::u8string joined = dir.FilePath(u8"assets/styles.xml");

	EXPECT_TRUE(SamePath(joined, ToU8(RootPath("assets/styles.xml"))));
	EXPECT_NE(joined.find(u8"assets/styles.xml"), std::u8string::npos);
}

TEST_F(DirectoryTest, RemoveFileDeletesExistingAndIgnoresMissing)
{
	Directory dir = RootDir();
	WriteFile(RootPath("keep.txt"), "1");
	WriteFile(RootPath("drop.txt"), "2");

	EXPECT_TRUE(dir.RemoveFile(u8"drop.txt"));
	EXPECT_FALSE(fs::exists(RootPath("drop.txt")));
	EXPECT_TRUE(fs::exists(RootPath("keep.txt")));

	EXPECT_FALSE(dir.RemoveFile(u8"drop.txt"));
}

TEST_F(DirectoryTest, RemoveDirectoryDeletesSubtree)
{
	Directory dir = RootDir();
	ASSERT_TRUE(dir.CreateDirectory(u8"tree/leaf"));
	WriteFile(RootPath("tree/leaf/a.txt"));

	EXPECT_TRUE(dir.RemoveDirectory(u8"tree"));
	EXPECT_FALSE(dir.Exists(u8"tree"));
	EXPECT_FALSE(dir.RemoveDirectory(u8"tree"));
}

TEST_F(DirectoryTest, RemoveRecursivelyDeletesCurrentDirectory)
{
	const fs::path doomed = RootPath("doomed");
	fs::create_directories(doomed / "nested");
	WriteFile(doomed / "nested" / "f.txt");

	Directory dir(ToU8(doomed));
	EXPECT_TRUE(dir.Exists());
	EXPECT_TRUE(dir.RemoveRecursively());
	EXPECT_FALSE(fs::exists(doomed));
	EXPECT_FALSE(dir.Exists());
}

TEST_F(DirectoryTest, EnumDirectoriesListsImmediateChildrenOnly)
{
	Directory dir = RootDir();
	ASSERT_TRUE(dir.CreateDirectory(u8"d1"));
	ASSERT_TRUE(dir.CreateDirectory(u8"d2"));
	ASSERT_TRUE(dir.CreateDirectory(u8"d1/nested"));
	WriteFile(RootPath("file.txt"));

	const auto dirs = dir.EnumDirectories();
	EXPECT_EQ(dirs.size(), 2u);
	EXPECT_TRUE(ContainsPath(dirs, RootPath("d1")));
	EXPECT_TRUE(ContainsPath(dirs, RootPath("d2")));
	EXPECT_FALSE(ContainsPath(dirs, RootPath("d1/nested")));
}

TEST_F(DirectoryTest, EnumFilesFiltersByExtension)
{
	Directory dir = RootDir();
	WriteFile(RootPath("a.txt"));
	WriteFile(RootPath("b.md"));
	WriteFile(RootPath("c.xml"));
	WriteFile(RootPath("d.bin"));
	ASSERT_TRUE(dir.CreateDirectory(u8"subdir"));
	WriteFile(RootPath("subdir/nested.txt"));

	const auto all = dir.EnumFiles({});
	EXPECT_EQ(all.size(), 4u);

	const auto txt = dir.EnumFiles({ u8".txt" });
	EXPECT_EQ(txt.size(), 1u);
	EXPECT_TRUE(ContainsPath(txt, RootPath("a.txt")));

	const auto multi = dir.EnumFiles({ u8".txt", u8".xml" });
	EXPECT_EQ(multi.size(), 2u);
	EXPECT_TRUE(ContainsPath(multi, RootPath("a.txt")));
	EXPECT_TRUE(ContainsPath(multi, RootPath("c.xml")));
}

TEST_F(DirectoryTest, EnumOnMissingDirectoryReturnsEmpty)
{
	const Directory missing(ToU8(RootPath("gone")));
	EXPECT_TRUE(missing.EnumFiles({}).empty());
	EXPECT_TRUE(missing.EnumDirectories().empty());
}

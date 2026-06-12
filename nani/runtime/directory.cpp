#include "directory.h"
#include <filesystem>
using namespace std;
namespace nani::runtime
{
	Directory::Directory(const u8string_view& basePath)
		: m_basePath(basePath)
	{

	}

	Directory::Directory()
	{
		auto cwd = filesystem::current_path();
		m_basePath = cwd.u8string();
	}

	Directory::~Directory()
	{

	}

	bool Directory::Exists() const
	{
		error_code ec;
		return filesystem::exists(m_basePath, ec) && filesystem::is_directory(m_basePath, ec);
	}

	bool Directory::Exists(const u8string_view& dirPath) const
	{
		Directory testAbsoluteDir(dirPath);
		if (testAbsoluteDir.IsAbsolute())
			return testAbsoluteDir.Exists();

		u8string absPath = AbsolutePath();
		absPath.append(u8"/");
		absPath.append(dirPath);
		Directory dir(absPath);
		return dir.Exists();
	}

	bool Directory::CD(const u8string_view& dirPath)
	{
		if (!Exists(dirPath))
			return false;

		Directory testAbsoluteDir(dirPath);
		if (testAbsoluteDir.IsAbsolute())
		{
			if (testAbsoluteDir.Exists())
			{
				m_basePath = testAbsoluteDir.AbsolutePath();
				return true;
			}
			return false;
		}

		m_basePath.append(u8"/");
		m_basePath.append(dirPath);
		return true;
	}

	bool Directory::CreateDirectory(const u8string_view& dirPath)
	{
		Directory testAbsoluteDir(dirPath);
		if (testAbsoluteDir.IsAbsolute())
		{
			if (testAbsoluteDir.Exists())
				return true;
			error_code ec;
			return filesystem::create_directories(dirPath, ec);
		}

		u8string absPath = AbsolutePath();
		absPath.append(u8"/");
		absPath.append(dirPath);
		Directory dir(absPath);
		if (dir.Exists())
			return true;

		error_code ec;
		return filesystem::create_directories(dir.AbsolutePath(), ec);
	}

	bool Directory::IsAbsolute() const
	{
		filesystem::path path(m_basePath);
		return path.is_absolute();
	}

	u8string Directory::AbsolutePath() const
	{
		if (IsAbsolute())
			return m_basePath;

		error_code ec;
		return filesystem::weakly_canonical(m_basePath, ec).u8string();
	}

	bool Directory::RemoveFile(const u8string_view& fileName)
	{
		u8string filePath = FilePath(u8string(fileName));
		error_code ec;
		return filesystem::remove(filePath, ec);
	}

	bool Directory::RemoveDirectory(const u8string_view& dirPath)
	{
		Directory dir(AbsolutePath());
		if (!dir.CD(dirPath))
			return false;
		error_code ec;
		return filesystem::remove_all(dir.AbsolutePath(), ec);
	}

	bool Directory::RemoveRecursively()
	{
		error_code ec;
		return filesystem::remove_all(AbsolutePath(), ec);
	}

	u8string Directory::FilePath(const u8string& fileName) const
	{
		return AbsolutePath().append(u8"/").append(fileName);
	}

	vector<u8string> Directory::EnumFiles(const vector<u8string>& extensions) const
	{
		return vector<u8string>();
	}

	vector<u8string> Directory::EnumDirectories() const
	{
		return vector<u8string>();
	}
}
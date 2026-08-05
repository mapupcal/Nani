#pragma once
#include "defs.h"
namespace nani::runtime
{
	class NANI_RUNTIME_API Directory
	{
	public:
		Directory(const std::u8string_view& basePath);
		Directory();
		~Directory();

	public:
		// Directory containing the given file path (absolute or relative to cwd).
		// Empty / unresolvable paths fall back to the current working directory.
		static Directory FromFile(const std::u8string_view& filePath);

		bool Exists() const;
		bool Exists(const std::u8string_view& dirPath) const;
		bool CD(const std::u8string_view& dirPath);
		bool CreateDirectory(const std::u8string_view& dirPath);
		bool IsAbsolute() const;
		std::u8string AbsolutePath() const;
		bool RemoveFile(const std::u8string_view& fileName);
		bool RemoveDirectory(const std::u8string_view& dirPath);
		bool RemoveRecursively();
		std::u8string FilePath(const std::u8string& fileName) const;

		std::vector<std::u8string> EnumFiles(const std::vector<std::u8string>& extensions) const;
		std::vector<std::u8string> EnumDirectories() const;

	private:
		std::u8string m_basePath;
	};
}
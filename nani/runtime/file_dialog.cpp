#include "file_dialog.h"
#include <nfd.hpp>
namespace nani::runtime
{
	namespace
	{
		FileDialog::DialogResult ConverNFDResult(nfdresult_t result)
		{
			if (result == NFD_OKAY)
				return FileDialog::Okay;
			if (result == NFD_CANCEL)
				return FileDialog::Cancel;
			return FileDialog::Error;
		}
	}

	FileDialog::DialogResult FileDialog::OpenSaveFileDialog(const std::vector<FilterItem>& filters, const std::u8string_view& defaultFileName, const std::u8string_view& defaultDirectory)
	{
		m_results.clear();

		NFD::Guard nfdGuard;

		std::vector<nfdu8filteritem_t> nfdFilters(filters.size());
		for (int idx = 0; idx < filters.size(); idx++)
		{
			nfdFilters[idx].name = reinterpret_cast<const char*>(filters[idx].name.c_str());
			nfdFilters[idx].spec = reinterpret_cast<const char*>(filters[idx].spec.c_str());
		}
		std::u8string directory(defaultDirectory);
		const nfdu8char_t* nfdDefaultDirectory = reinterpret_cast<const char*>(directory.c_str());
		std::u8string fileName(defaultFileName);
		const nfdu8char_t* nfdDefaultFileName = reinterpret_cast<const char*>(fileName.c_str());

		NFD::UniquePathU8 nfdPath;
		auto result = NFD::SaveDialog(nfdPath, nfdFilters.data(), nfdFilters.size(), nfdDefaultDirectory, nfdDefaultFileName);
		if (result == NFD_OKAY)
		{
			std::u8string path = reinterpret_cast<const char8_t*>(nfdPath.get());
			m_results.push_back(path);
		}
		return ConverNFDResult(result);
	}

	FileDialog::DialogResult FileDialog::OpenSelectFileDialog(const std::vector<FilterItem>& filters, const std::u8string_view& defaultDirectory, bool bMulti)
	{
		m_results.clear();

		NFD::Guard nfdGuard;

		std::vector<nfdu8filteritem_t> nfdFilters(filters.size());
		for (int idx = 0; idx < filters.size(); idx++)
		{
			nfdFilters[idx].name = reinterpret_cast<const char*>(filters[idx].name.c_str());
			nfdFilters[idx].spec = reinterpret_cast<const char*>(filters[idx].spec.c_str());
		}
		std::u8string directory(defaultDirectory);
		const nfdu8char_t* nfdDefaultDirectory = reinterpret_cast<const char*>(directory.c_str());

		if (bMulti)
		{
			NFD::UniquePathSet nfdPathSet;
			auto result = NFD::OpenDialogMultiple(nfdPathSet, nfdFilters.data(), nfdFilters.size(), nfdDefaultDirectory);
			if (result == NFD_OKAY)
			{
				nfdpathsetsize_t nfdPathSetSize = 0;
				NFD::PathSet::Count(nfdPathSet, nfdPathSetSize);
				for (int idx = 0; idx < nfdPathSetSize; idx++)
				{
					NFD::UniquePathSetPathU8 nfdPath;
					NFD::PathSet::GetPath(nfdPathSet, idx, nfdPath);
					std::u8string path = reinterpret_cast<const char8_t*>(nfdPath.get());
					m_results.push_back(path);
				}
			}
			return ConverNFDResult(result);
		}
		else
		{
			NFD::UniquePathU8 nfdPath;
			auto result = NFD::OpenDialog(nfdPath, nfdFilters.data(), nfdFilters.size(), nfdDefaultDirectory);
			if (result == NFD_OKAY)
			{
				std::u8string path = reinterpret_cast<const char8_t*>(nfdPath.get());
				m_results.push_back(path);
			}
			return ConverNFDResult(result);
		}
	}

	FileDialog::DialogResult FileDialog::OpenSelectDirectoryDialog(const std::u8string_view& defaultDirectory, bool bMulti)
	{
		m_results.clear();
		NFD::Guard nfdGuard;

		std::u8string directory(defaultDirectory);
		const nfdu8char_t* nfdDefaultDirectory = reinterpret_cast<const char*>(directory.c_str());

		if (bMulti)
		{
			NFD::UniquePathSet nfdPathSet;
			auto result = NFD::PickFolderMultiple(nfdPathSet, nfdDefaultDirectory);
			if (result == NFD_OKAY)
			{
				nfdpathsetsize_t nfdPathSetSize = 0;
				NFD::PathSet::Count(nfdPathSet, nfdPathSetSize);
				for (int idx = 0; idx < nfdPathSetSize; idx++)
				{
					NFD::UniquePathSetPathU8 nfdPath;
					NFD::PathSet::GetPath(nfdPathSet, idx, nfdPath);
					std::u8string path = reinterpret_cast<const char8_t*>(nfdPath.get());
					m_results.push_back(path);
				}
			}
			return ConverNFDResult(result);
		}
		else
		{
			NFD::UniquePathU8 nfdPath;
			auto result = NFD::PickFolder(nfdPath, nfdDefaultDirectory);
			if (result == NFD_OKAY)
			{
				std::u8string path = reinterpret_cast<const char8_t*>(nfdPath.get());
				m_results.push_back(path);
			}
			return ConverNFDResult(result);
		}
	}

	std::vector<std::u8string> FileDialog::GetResults() const
	{
		return m_results;
	}
}
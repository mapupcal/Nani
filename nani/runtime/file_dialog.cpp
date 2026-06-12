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

	FileDialog::DialogResult FileDialog::OpenSaveFileDialog(const std::vector<FilterItem>& filters, const std::string_view& defaultFileName, const std::string_view& defaultDirectory)
	{
		NFD::Guard nfdGuard;

		std::vector<nfdu8filteritem_t> nfdFilters(filters.size());
		for (int idx = 0; idx < filters.size(); idx++)
		{
			nfdFilters[idx].name = filters[idx].name.c_str();
			nfdFilters[idx].spec = filters[idx].spec.c_str();
		}
		std::string directory(defaultDirectory);
		const nfdu8char_t* nfdDefaultDirectory = directory.c_str();
		std::string fileName(defaultFileName);
		const nfdu8char_t* nfdDefaultFileName = fileName.c_str();

		NFD::UniquePathU8 nfdPath;
		auto result = NFD::SaveDialog(nfdPath, nfdFilters.data(), nfdFilters.size(), nfdDefaultDirectory, nfdDefaultFileName);
		if (result == NFD_OKAY)
		{
			std::string path = nfdPath.get();
			m_results.push_back(path);
		}
		return ConverNFDResult(result);
	}

	FileDialog::DialogResult FileDialog::OpenSelectFileDialog(const std::vector<FilterItem>& filters, const std::string_view& defaultDirectory, bool bMulti)
	{
		m_results.clear();

		NFD::Guard nfdGuard;

		std::vector<nfdu8filteritem_t> nfdFilters(filters.size());
		for (int idx = 0; idx < filters.size(); idx++)
		{
			nfdFilters[idx].name = filters[idx].name.c_str();
			nfdFilters[idx].spec = filters[idx].spec.c_str();
		}
		std::string directory(defaultDirectory);
		const nfdu8char_t* nfdDefaultDirectory = directory.c_str();

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
					std::string path = nfdPath.get();
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
				std::string path = nfdPath.get();
				m_results.push_back(path);
			}
			return ConverNFDResult(result);
		}
	}

	FileDialog::DialogResult FileDialog::OpenSelectDirectoryDialog(const std::string_view& defaultDirectory, bool bMulti)
	{
		m_results.clear();
		NFD::Guard nfdGuard;

		std::string directory(defaultDirectory);
		const nfdu8char_t* nfdDefaultDirectory = directory.c_str();

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
					std::string path = nfdPath.get();
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
				std::string path = nfdPath.get();
				m_results.push_back(path);
			}
			return ConverNFDResult(result);
		}
	}

	std::vector<std::string> FileDialog::GetResults() const
	{
		return m_results;
	}
}
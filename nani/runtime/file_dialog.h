#pragma once
#include "defs.h"
namespace nani::runtime
{
	class NANI_RUNTIME_API FileDialog
	{
	public:
		enum DialogResult
		{
			Error,
			Okay,
			Cancel
		};

		struct FilterItem
		{
			std::string name;
			std::string spec;
		};

	public:
		DialogResult OpenSaveFileDialog(const std::vector<FilterItem>& filters, const std::string_view& defaultFileName = std::string_view(), const std::string_view& defaultDirectory = std::string_view());
		DialogResult OpenSelectFileDialog(const std::vector<FilterItem>& filters, const std::string_view& defaultDirectory = std::string_view(), bool bMulti = false);
		DialogResult OpenSelectDirectoryDialog(const std::string_view& defaultDirectory = std::string_view(), bool bMulti = false);
		std::vector<std::string> GetResults() const;

	private:
		std::vector<std::string> m_results;
	};
}
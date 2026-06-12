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
			std::u8string  name;
			std::u8string  spec;
		};

	public:
		DialogResult OpenSaveFileDialog(const std::vector<FilterItem>& filters, const std::u8string_view& defaultFileName = std::u8string_view(), const std::u8string_view& defaultDirectory = std::u8string_view());
		DialogResult OpenSelectFileDialog(const std::vector<FilterItem>& filters, const std::u8string_view& defaultDirectory = std::u8string_view(), bool bMulti = false);
		DialogResult OpenSelectDirectoryDialog(const std::u8string_view& defaultDirectory = std::u8string_view(), bool bMulti = false);
		std::vector<std::u8string> GetResults() const;

	private:
		std::vector<std::u8string> m_results;
	};
}
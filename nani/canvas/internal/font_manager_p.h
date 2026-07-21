#pragma once
#include "defs.h"

namespace nani::canvas::internal
{
	class FontManagerPrivate
	{
	public:
		static FontManagerPrivate* Instance();

	public:
		std::vector<std::u8string> DefaultFamilies() const;
		std::vector<std::u8string> AvailableFamilies() const;
		bool RegisterFont(const std::u8string_view& fontFilePath);
		bool UnRegisterFont(const std::u8string_view& fontFilePath);
		void ClearCache();

		std::shared_ptr<const FontMetricsPrivate> GetMetrics(const text::Font& font);

	private:
		FontManagerPrivate() = default;
		~FontManagerPrivate() = default;
	};
}
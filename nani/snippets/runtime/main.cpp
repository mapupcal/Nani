#include "runtime/file_dialog.h"
#include <locale>
#include <codecvt>
#include <string>

using namespace nani::runtime;

int main(int argc, char** argv)
{
	FileDialog dialog;
	std::vector<FileDialog::FilterItem> filters = {
		{.name = u8"PlainText Files", .spec = u8"txt"},
		{.name = u8"Source Files", .spec = u8"c,cpp"},
		{.name = u8"Header Files", .spec = u8"h,hpp"}
	};
	//Single File.
	auto result = dialog.OpenSelectFileDialog(filters);

	if (result == FileDialog::Okay)
	{
		for (auto path : dialog.GetResults())
			printf("Select file path : %s", reinterpret_cast<const char*>(path.c_str()));
	}

	//Multi Files
	result = dialog.OpenSelectFileDialog(filters, std::u8string_view(), true);
	if (result == FileDialog::Okay)
	{
		for (auto path : dialog.GetResults())
			printf("Select file path : %s", reinterpret_cast<const char*>(path.c_str()));
	}

	//Single folder
	result = dialog.OpenSelectDirectoryDialog(std::u8string_view());
	if (result == FileDialog::Okay)
	{
		for (auto path : dialog.GetResults())
			printf("Select folder path : %s", reinterpret_cast<const char*>(path.c_str()));
	}

	//Multi folders
	result = dialog.OpenSelectDirectoryDialog(std::u8string_view(), true);
	if (result == FileDialog::Okay)
	{
		for (auto path : dialog.GetResults())
			printf("Select folder path : %s", reinterpret_cast<const char*>(path.c_str()));
	}

	//Save file.
	result = dialog.OpenSaveFileDialog(filters, u8"test.txt", std::u8string_view());
	if (result == FileDialog::Okay)
	{
		for (auto path : dialog.GetResults())
			printf("Save file path : %s", reinterpret_cast<const char*>(path.c_str()));
	}
}


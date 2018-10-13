#include "Helpers.h"
#include <string>
#include <iostream>
#include <experimental/filesystem>
using namespace std::experimental::filesystem;

std::string GetFileNameOnlyFromPath(std::string path)
{
	auto loc = path.find_last_of("\\");
	path.erase(path.begin(), path.begin() + loc + 1);
	return path;
}

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

std::vector<std::pair<std::string, std::string>> GetPathsAndNamesToFilesMatching(std::string match, std::string dir)
{
	std::vector<std::pair<std::string, std::string>> paths;
	std::vector<std::pair<std::string, std::string>> matchingPaths;
	for (const auto & p : directory_iterator(dir))
	{
		std::string filename = GetFileNameOnlyFromPath(ws2s(p.path().c_str()));
		paths.push_back(std::make_pair(ws2s(p.path().c_str()), filename));
	}

	for (auto& it = paths.begin(); it != paths.end(); it++)
	{
		if (it->first.find(match) != std::string::npos)
			matchingPaths.push_back(*it);
	}

	return matchingPaths;
}
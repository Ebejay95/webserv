#include "../server/server.hpp"

// Internal trimming
std::string trim(const std::string& str)
{
	size_t first = str.find_first_not_of(" \t");
	if (first == std::string::npos) return "";
	size_t last = str.find_last_not_of(" \t");
	return str.substr(first, last - first + 1);
}


// Parsing Constants
std::vector<std::string> parseOptionsToVector(const std::string& opts)
{
	std::vector<std::string> result;
	std::istringstream stream(opts);
	std::string opt;
	while (std::getline(stream, opt, ','))
	{
		result.push_back(trim(opt));
	}
	return result;
}

// Expand EnV Variables in Bash Style for Config Values
std::string expandEnvironmentVariables(const std::string& value, char** env)
{
	if (!env) return value;

	std::string result = value;
	size_t pos = 0;
	size_t end;

	while ((pos = result.find('$', pos)) != std::string::npos)
	{
		if (pos + 1 >= result.length())
			break;
		end = pos + 1;
		while (end < result.length() &&
			(std::isalnum((unsigned char)result[end]) || result[end] == '_'))
			end++;

		std::string varName = result.substr(pos + 1, end - (pos + 1));
		bool found = false;
		for (char** envVar = env; *envVar != nullptr; envVar++)
		{
			std::string envStr(*envVar);
			size_t equalPos = envStr.find('=');
			if (equalPos != std::string::npos)
			{
				if (envStr.substr(0, equalPos) == varName)
				{
					std::string replacer = envStr.substr(equalPos + 1);
					result.replace(pos, end - pos, replacer);
					pos += replacer.length();
					found = true;
					break;
				}
			}
		}
		if (!found)
			pos = end;
	}
	return result;
}

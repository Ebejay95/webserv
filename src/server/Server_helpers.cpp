#include "Server.hpp"

// Adds a server name entry to the /etc/hosts file
bool Server::addServerNameToHosts(const std::string &server_name)
{
	const std::string hosts_file = "/etc/hosts";
	std::ifstream infile(hosts_file);
	std::string line;

	// Check if the server_name already exists in /etc/hosts
	while (std::getline(infile, line))
	{
		if (line.find(server_name) != std::string::npos)
			return true;
	}

	// Add server_name to /etc/hosts
	std::ofstream outfile(hosts_file, std::ios::app);
	if (!outfile.is_open())
		throw std::runtime_error("Failed to open /etc/hosts");
	outfile << "127.0.0.1 " << server_name << "\n";
	outfile.close();
	Logger::yellow("Added " + server_name + " to /etc/hosts file");
	added_server_names.push_back(server_name);
	return true;
}

// Removes previously added server names from the /etc/hosts file
void Server::removeAddedServerNamesFromHosts()
{
	const std::string hosts_file = "/etc/hosts";
	std::ifstream infile(hosts_file);
	if (!infile.is_open()) {
		throw std::runtime_error("Failed to open /etc/hosts");
	}

	std::vector<std::string> lines;
	std::string line;
	while (std::getline(infile, line))
	{
		bool shouldRemove = false;
		for (const auto &name : added_server_names)
		{
			if (line.find(name) != std::string::npos)
			{
				Logger::yellow("Remove " + name + " from /etc/host file");
				shouldRemove = true;
				break;
			}
		}
		if (!shouldRemove)
			lines.push_back(line);
	}
	infile.close();
	std::ofstream outfile(hosts_file, std::ios::trunc);
	if (!outfile.is_open())
		throw std::runtime_error("Failed to open /etc/hosts for writing");
	for (const auto &l : lines)
		outfile << l << "\n";
	outfile.close();
	added_server_names.clear();
}

// Normalizes a given path, ensuring it starts with '/' and removing trailing slashes
std::string Server::normalizePath(const std::string& path) {
    std::string normalized;
    if (path.empty()) {
        return "/";
    }

    if (path[0] != '/') {
        normalized = "/" + path;
    } else {
        normalized = path;
    }

    std::string result;
    bool lastWasSlash = false;

    for (char c : normalized) {
        if (c == '/') {
            if (!lastWasSlash) {
                result += c;
            }
            lastWasSlash = true;
        } else {
            result += c;
            lastWasSlash = false;
        }
    }

    return result;
}

bool isPathMatch(const std::vector<std::string>& requestSegments,
                        const std::vector<std::string>& locationSegments) {
    // Location path cannot be longer than request path
    if (locationSegments.size() > requestSegments.size()) {
        return false;
    }

    // Compare each segment
    for (size_t i = 0; i < locationSegments.size(); ++i) {
        if (locationSegments[i] != requestSegments[i]) {
            return false;
        }
    }

    return true;
}
// Matches a request path to a location block
bool Server::matchLoc(const std::vector<Location>& locations, const std::string& rawPath, Location& bestMatch) {
    // Normalize the request path
	Logger::errorLog("rawPath: " + rawPath);
    std::string normalizedPath = normalizePath(rawPath);
	Logger::errorLog("normalizedPath: " + normalizedPath);

    // Split path into segments
    std::vector<std::string> requestSegments = splitPathLoc(normalizedPath);

    size_t bestMatchLength = 0;
    bool foundMatch = false;

    // Iterate through all locations to find best match
    for (const Location& loc : locations) {
        std::vector<std::string> locationSegments = splitPathLoc(loc.path);

		for (auto it : locationSegments)
		{
			Logger::errorLog("locationSegment: " + it);
		}
		for (auto blub : requestSegments)
		{
			Logger::errorLog("requestSegment: " + blub);
		}
        // Check if this location is a potential match
        if (isPathMatch(requestSegments, locationSegments)) {
            // If we found a match that's longer than our current best match
            if (locationSegments.size() > bestMatchLength) {
                bestMatchLength = locationSegments.size();
                bestMatch = loc;
                foundMatch = true;
            }
        }
    }

	if (foundMatch == false )
	{
		Logger::errorLog("mamamamam: ");
	}
    return foundMatch;
}

// Concatenates a root path with a sub-path, ensuring correct path separators
std::string Server::concatenatePath(const std::string& root, const std::string& path)
{
	if (root.empty())
		return path;
	if (path.empty())
		return root;

	if (root.back() == '/' && path.front() == '/')
		return root + path.substr(1);
	else if (root.back() != '/' && path.front() != '/')
		return root + '/' + path;
	else
		return root + path;
}

// Extracts the directory part of a given file path
std::string Server::getDirectory(const std::string& path)
{
	size_t lastSlash = path.find_last_of("/\\");
	if (lastSlash != std::string::npos)
		return path.substr(0, lastSlash);
	return "";
}

// Converts a request type enum to a string representation
std::string Server::requestTypeToString(RequestType type)
{
	switch(type)
	{
		case RequestType::INITIAL: return "INITIAL";
		case RequestType::STATIC: return "STATIC";
		case RequestType::CGI: return "CGI";
		case RequestType::ERROR: return "ERROR";
		default: return "UNKNOWN";
	}
}

// Checks if a file is readable
bool Server::fileReadable(const std::string& path)
{
	struct stat st;
	return (stat(path.c_str(), &st) == 0 && (st.st_mode & S_IRUSR));
}

// Checks if a file is executable
bool Server::fileExecutable(const std::string& path)
{
	struct stat st;
	return (stat(path.c_str(), &st) == 0 && (st.st_mode & S_IXUSR));
}

// Checks if a directory is readable
bool Server::dirReadable(const std::string& path)
{
	struct stat st;
	return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode) && (st.st_mode & S_IRUSR));
}

// Checks if a directory is writable
bool Server::dirWritable(const std::string& path)
{
	struct stat st;
	return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode) && (st.st_mode & S_IWUSR));
}

// Verifies access permissions for a given path based on the request context
bool Server::checkAccessRights(Context &ctx, std::string path)
{
    struct stat path_stat;
        Logger::errorLog("checkAccessRights");
    if (stat(path.c_str(), &path_stat) != 0)
        return updateErrorStatus(ctx, 404, "Not found");

    if (S_ISDIR(path_stat.st_mode))
	{
        if (ctx.location_inited && ctx.location.autoindex == "on")
		{
            std::string dirPath = getDirectory(path) + '/';
            if (!dirReadable(dirPath))
                return updateErrorStatus(ctx, 403, "Forbidden");
            ctx.doAutoIndex = dirPath;
            ctx.type = STATIC;
            return true;
        }
		else
            return updateErrorStatus(ctx, 403, "Forbidden");
    }

    if (!fileReadable(path))
        return updateErrorStatus(ctx, 403, "Forbidden");

    if (ctx.type == CGI && !fileExecutable(path))
        return updateErrorStatus(ctx, 500, "CGI script not executable");

    if (ctx.method == "POST")
	{
        std::string uploadDir = getDirectory(path);
        if (!dirWritable(uploadDir))
            return updateErrorStatus(ctx, 403, "Forbidden");
    }

    if (path.length() > 4096)
        return updateErrorStatus(ctx, 414, "URI Too Long");
    return true;
}

// Determines whether the requested HTTP method is allowed in the location block
bool isMethodAllowed(Context& ctx)
{

	bool isAllowed = false;
	std::string reason;

	if (ctx.method == "GET" && ctx.location.allowGet)
	{
		isAllowed = true;
		reason = "GET method allowed for this location";
	}
	else if (ctx.method == "POST" && ctx.location.allowPost)
	{
		isAllowed = true;
		reason = "POST method allowed for this location";
	}
	else if (ctx.method == "DELETE" && ctx.location.allowDelete)
	{
		isAllowed = true;
		reason = "DELETE method allowed for this location";
	}
	else
		reason = ctx.method + " method not allowed for this location";
	return isAllowed;
}

// Detects if the request contains multipart/form-data content
bool detectMultipartFormData(Context &ctx)
{
	if (ctx.headers["Content-Type"] == "Content-Type: multipart/form-data")
	{
		ctx.is_multipart = true;
		return true;
	}
	ctx.is_multipart = false;
	return false;
}

// Approves a file extension based on CGI configuration or static file handling rules
std::string Server::approveExtention(Context& ctx, std::string path_to_check)
{
    std::string approvedIndex;

    size_t dot_pos = path_to_check.find_last_of('.');

    if (dot_pos != std::string::npos)
    {
        std::string extension = path_to_check.substr(dot_pos + 1);
        if (ctx.type != CGI || (extension == ctx.location.cgi_filetype && ctx.type == CGI))
        {
            approvedIndex = path_to_check;
        }
        else
        {
			updateErrorStatus(ctx, 500, "Internal Server Error");
            return "";
        }
    }
    else if (ctx.type == CGI && ctx.method != "POST")
    {
        updateErrorStatus(ctx, 400, "Bad Request: No file extension found");
        return "";
    }
    else if (!ctx.location.return_url.empty())
	{
        return path_to_check;
    }
    else
	{
		for (const auto &conf : configData)
		{
			if (conf.server_fd == ctx.server_fd)
				printServerBlock((ServerBlock&)conf);
		}
		Logger::errorLog("approveExtention " + ctx.method + " " + path_to_check + " " + ctx.location.return_url);
        updateErrorStatus(ctx, 404, "Not found");
        return "";
    }
    return approvedIndex;
}

// Resets the context, clearing request data and restoring initial values
bool Server::resetContext(Context& ctx)
{
    ctx.cookies.clear();
    ctx.setCookies.clear();
	ctx.input_buffer.clear();
	ctx.headers.clear();
	ctx.method.clear();
	ctx.path.clear();
	ctx.version.clear();
	ctx.headers_complete = false;
	ctx.content_length = 0;
	ctx.error_code = 0;
	ctx.req.parsing_phase = RequestBody::PARSING_HEADER;
	ctx.req.current_body_length = 0;
	ctx.req.expected_body_length = 0;
	ctx.req.received_body.clear();
	ctx.req.chunked_state.processing = false;
	ctx.req.is_upload_complete = false;
	ctx.type = RequestType::INITIAL;
	return true;
}

// Determines the request type based on the server configuration and updates the context
bool Server::determineType(Context& ctx, std::vector<ServerBlock> configs)
{
	ServerBlock* conf = nullptr;
	for (auto& config : configs)
	{
			Logger::errorLog("asdasdadada ");
		if (config.server_fd == ctx.server_fd)
		{
			conf = &config;
			break;
		}
	}

	if (!conf)
		return updateErrorStatus(ctx, 500, "Internal Server Error");
	Logger::errorLog(ctx.path);
	Location bestMatch;
	if (matchLoc(conf->locations, ctx.path, bestMatch)) {
		Logger::errorLog("in match: ");
		Logger::errorLog(bestMatch.return_url);
		Logger::errorLog(bestMatch.return_code);
		ctx.port = conf->port;
		ctx.name = conf->name;
		ctx.root = conf->root;
		ctx.index = conf->index;
		ctx.errorPages = conf->errorPages;
		ctx.timeout = conf->timeout;
		ctx.location = bestMatch;
		ctx.location_inited = true;
		if (!isMethodAllowed(ctx))
			return updateErrorStatus(ctx, 405, "Method (" + ctx.method + ") not allowed");
		if (bestMatch.cgi != "")
			ctx.type = CGI;
		else
			ctx.type = STATIC;
		parseAccessRights(ctx);
		logContext(ctx);
		return true;
	}
	return (updateErrorStatus(ctx, 500, "Internal Server Error"));
}

// Retrieves the maximum allowed body size for a request from the server configuration
void Server::getMaxBodySizeFromConfig(Context& ctx, std::vector<ServerBlock> configs)
{
	ServerBlock* conf = nullptr;

	for (auto& config : configs)
	{

		if (config.server_fd == ctx.server_fd)
		{
			conf = &config;
			break;
		}
	}
	if (!conf)
		return;

	for (auto loc : conf->locations)
	{
		Location bestMatch;
		if (matchLoc(conf->locations, ctx.path, bestMatch))
		{
			ctx.client_max_body_size = loc.client_max_body_size;
			if (ctx.client_max_body_size != -1 && bestMatch.client_max_body_size == -1)
				ctx.location.client_max_body_size = ctx.client_max_body_size;
			return;
		}
	}
}

#include "Server.hpp"

int Server::runEventLoop(int epoll_fd, std::vector<ServerBlock> &configs)
{
const int max_events = 64;
struct epoll_event events[max_events];
const int timeout_ms = 500;
Logger::file("\n");
int count = 4000;
while (count != 0)
{
	int n = epoll_wait(epoll_fd, events, max_events, timeout_ms);
	//Logger::file("Events count: " + std::to_string(n));

	if (n < 0)
	{
		Logger::file("Epoll error: " + std::string(strerror(errno)));
		if (errno == EINTR) continue;
		break;
	}
	else if (n == 0)
	{
			checkAndCleanupTimeouts();
			continue;
		}

		for (int i = 0; i < n; i++)
		{
			int incoming_fd = events[i].data.fd; // could be server_fd when new connection, client_fd when existing
			uint32_t ev = events[i].events;
			Logger::file("New Event:\nfd=" + std::to_string(incoming_fd) + " events=" + std::to_string(ev) + "\n");
			dispatchEvent(epoll_fd, incoming_fd, ev, configs);
		}
		//checkAndCleanupTimeouts();
		count--;
	}
	close(epoll_fd);
	return EXIT_SUCCESS;
}

bool Server::dispatchEvent(int epoll_fd, int incoming_fd, uint32_t ev, std::vector<ServerBlock> &configs)
{
	int client_fd;
	int server_fd;

	Logger::file("dispatchEvent:");
	if (ev & EPOLLIN)
    Logger::file("EPOLLIN event: " + std::to_string(ev) + " triggered (data available to read)");
	if (ev & EPOLLOUT)
		Logger::file("EPOLLOUT event: " + std::to_string(ev) + " triggered (ready for writing)");
	if (ev & EPOLLERR)
		Logger::file("EPOLLERR event: " + std::to_string(ev) + " detected (error on fd)");
	if (ev & EPOLLHUP)
		Logger::file("EPOLLHUP event: " + std::to_string(ev) + " detected (connection closed)");

	// 1. Check if this is an existing client connection
	logRequestBodyMapFDs();
	if (globalFDS.request_state_map.find(incoming_fd) != globalFDS.request_state_map.end())
	{
		client_fd = incoming_fd;  // Clarify that fd is a client connection
		Logger::file("Found Client_fd: " + std::to_string(client_fd) + " in globalFDS.request_state_map");
		Context& ctx = globalFDS.request_state_map[client_fd];
		ctx.last_activity = std::chrono::steady_clock::now();

		// Handle errors
		if (ev & (EPOLLHUP | EPOLLERR))
		{
			Logger::file("Connection Error on client_fd: " + std::to_string(client_fd));
			// TODO: Implement cleanup
			return true;
		}

		// Handle based on request type
		while (true)
		{
			switch (ctx.type)
			{
				case INITIAL:
					Logger::file("Handling INITIAL request for client_fd: " + std::to_string(client_fd));
					if (ev & EPOLLIN)
					{
						parseRequest(ctx);
						determineType(ctx, configs);
						Logger::file("Type after Parse: " + requestTypeToString(ctx.type) + "\n");
						// Continue to re-evaluate the updated type
						continue;
					}
					break;

				case STATIC:
					Logger::file("Handling STATIC request for client_fd: " + std::to_string(client_fd));
					staticHandler(ctx, ev);
					// TODO: Implement static file handling
					continue;

				case CGI:
					Logger::file("Handling CGI request for client_fd: " + std::to_string(client_fd));
					// TODO: Implement CGI handling
					continue;

				case ERROR:
					Logger::file("Handling ERROR request for client_fd: " + std::to_string(client_fd));
					errorsHandler(ctx, ev);
					return true;

				default:
					//Logger::file("Unknown request type for client_fd: " + std::to_string(client_fd));
					return true;
			}

			// Break the loop once the request has been handled
			break;
		}
	}

	// 2. Check if this is a server socket (new connection)
	if (findServerBlock(configs, incoming_fd))
	{
		server_fd = incoming_fd;  // Clarify that fd is a server socket
		Logger::file("New connection on server_fd: " + std::to_string(server_fd) + " epoll_fd: " + std::to_string(epoll_fd));
		return handleNewConnection(epoll_fd, server_fd);
	}

	// 3. Unknown fd - this shouldn't happen
	Logger::file("ERROR: Unknown fd: " + std::to_string(incoming_fd));
	delFromEpoll(epoll_fd, incoming_fd);
	return false;
}

bool Server::isMethodAllowed(Context& ctx) {

	bool isAllowed = false;
	std::string reason;

	if (ctx.method == "GET" && ctx.location->allowGet) {
		isAllowed = true;
		reason = "GET method allowed for this location";
	}
	else if (ctx.method == "POST" && ctx.location->allowPost) {
		isAllowed = true;
		reason = "POST method allowed for this location";
	}
	else if (ctx.method == "DELETE" && ctx.location->allowDelete) {
		isAllowed = true;
		reason = "DELETE method allowed for this location";
	}
	else if (ctx.method == "COOKIE" && ctx.location->allowCookie) {
		isAllowed = true;
		reason = "COOKIE method allowed for this location";
	}
	else {
		reason = ctx.method + " method not allowed for this location";
	}
	return isAllowed;
}

void Server::parseAcessRights(Context& ctx)
{
    Logger::file("parseAcessRights");
	ctx.access_flag = FILE_EXISTS;
	checkAccessRights(ctx, ctx.root);
	std::string rootAndLocation = concatenatePath(ctx.root, ctx.location->path);
	std::string aprovedIndex;
    size_t dot_pos = ctx.location->default_file.find_last_of('.');
    if (dot_pos != std::string::npos)
    {
        std::string extension = ctx.location->default_file.substr(dot_pos);
        if ((extension == ctx.location->cgi_filetype) || (ctx.location->cgi_filetype.empty() && extension == "html"))
        {
            aprovedIndex = ctx.location->default_file;
        }
        else
        {
			ctx.error_code = 500;
			ctx.type = ERROR;
			ctx.error_message = "Internal Server Error";
        }
    }
	if (!rootAndLocation.empty() && rootAndLocation.back() != '/')
    {
        rootAndLocation += "/";
    }
	std::string rootAndLocationIndex = rootAndLocation + aprovedIndex;
	ctx.access_flag = FILE_EXISTS;
	checkAccessRights(ctx, rootAndLocationIndex);
	ctx.access_flag = FILE_READ;
	checkAccessRights(ctx, rootAndLocationIndex);
	ctx.access_flag = FILE_EXECUTE;
	checkAccessRights(ctx, rootAndLocationIndex);
	// error pages extance read -> std fallbacks
	// root/location exitance and access for current HTTP Method: flag ob autoindex wird...
	// root/location->index exitance and access for current HTTP Method and CGI Type: flag ob autoindex wird...
	// bei static root/location/upload_store access w

}

void Server::checkAccessRights(Context &ctx, std::string path)
{
    Logger::file("[INFO] Checking access rights for: " + path);
    if (access(path.c_str(), ctx.access_flag))
    {
        if (ctx.location->autoindex == "on")
        {
            std::string dirPath = getDirectory(path);
            Logger::file("[INFO] Autoindex enabled, checking directory: " + dirPath);
            if (!access(dirPath.c_str(), R_OK))
            {
                ctx.error_code = 403;
                ctx.type = ERROR;
                ctx.error_message = "Forbidden";
                Logger::file("[ERROR] Autoindex forbidden for: " + dirPath);
                return;
            }
            ctx.doAutoIndex = dirPath;
            ctx.type = STATIC;
            Logger::file("[INFO] Autoindexing directory: " + dirPath);
            return;
        }

        std::string error_message;
        switch (errno)
        {
            case ENOENT:
                error_message = "File does not exist";
                ctx.error_code = 404;
                break;
            case EACCES:
                error_message = "Permission denied";
                ctx.error_code = 403;
                break;
            case EROFS:
                error_message = "Read-only file system";
                ctx.error_code = 500;
                break;
            case ENOTDIR:
                error_message = "A component of the path is not a directory";
                ctx.error_code = 500;
                break;
            default:
                error_message = "Unknown error: " + std::to_string(errno);
                ctx.error_code = 500;
                break;
        }
        ctx.type = ERROR;
        ctx.error_message = error_message;
        Logger::file("[ERROR] Access denied for " + path + " Reason: " + error_message);
    }
    else
    {
        Logger::file("[INFO] Access granted for: " + path);
    }
}


// Bestimmt Request-Typ aus Konfiguration
void Server::determineType(Context& ctx, const std::vector<ServerBlock>& configs)
{
	const ServerBlock* conf = nullptr;

	// Finde passende Konfiguration
	for (const auto& config : configs) {
		if (config.server_fd == ctx.server_fd)
		{
			conf = &config;
			break;
		}
	}

	if (!conf)
	{
		ctx.type = ERROR;
		ctx.error_code = 500;
        ctx.error_message = "Internal Server Error";
		return;
	}

	// path und index file muss da sein. Pfad aufloesen..
	// autoindex
	// uploadstore

	for (const auto& loc : conf->locations)
	{
		if (matchLoc(loc, ctx.path))
		{
			ctx.port = conf->port;
			ctx.name = conf->name;
			ctx.root = conf->root;
			ctx.index = conf->index;
			ctx.errorPages = conf->errorPages;
			ctx.client_max_body_size = conf->client_max_body_size;
			ctx.timeout = conf->timeout;
			ctx.location = &loc;
			if (!isMethodAllowed(ctx))
			{
				ctx.type = ERROR;
				ctx.error_code = 405;
        		ctx.error_message = "Method not allowed";
				return;
			}
			if (loc.cgi != "")
				ctx.type = CGI;
			else
				ctx.type = STATIC;
			parseAcessRights(ctx);
			return;
		}
	}

	ctx.type = ERROR;
	ctx.error_code = 500;
    ctx.error_message = "Internal Server Error";
}

// enum RequestType {
// 	STATIC,
// 	CGI,
// 	ERROR
// };

// struct Context
// {
// 	RequestType type;
// 	std::string method;
// 	std::string path;
// 	std::map<std::string, std::string> headers;
// 	std::string body;
// 	std::chrono::steady_clock::time_point last_activity;
// 	static constexpr std::chrono::seconds TIMEOUT_DURATION{5};

// 	std::string location_path;
// 	std::string requested_path;
// };


void Server::parseRequest(Context& ctx) {
	char buffer[8192];
	ssize_t bytes = read(ctx.client_fd, buffer, sizeof(buffer));

	if (bytes <= 0) return;

	std::string request(buffer, bytes);
	std::istringstream stream(request);
	std::string line;

	// Parse request line
	if (std::getline(stream, line)) {
		std::istringstream request_line(line);
		request_line >> ctx.method >> ctx.path >> ctx.version;
	}

	// Parse headers
	while (std::getline(stream, line) && !line.empty() && line != "\r") {
		size_t colon = line.find(':');
		if (colon != std::string::npos) {
			std::string key = line.substr(0, colon);
			std::string value = line.substr(colon + 2); // Skip ": "
			ctx.headers[key] = value;
		}
	}

	// Parse body if present
	if (ctx.headers.count("Content-Length")) {
		size_t length = std::stoi(ctx.headers["Content-Length"]);
		std::vector<char> body_buffer(length);
		stream.read(body_buffer.data(), length);
		ctx.body = std::string(body_buffer.data(), length);
	}

	logContext(ctx, "Parsed");
}


void Server::checkAndCleanupTimeouts() {
auto now = std::chrono::steady_clock::now();

for (auto it = globalFDS.request_state_map.begin(); it != globalFDS.request_state_map.end();) {
	Context& ctx = it->second;

	auto duration = std::chrono::duration_cast<std::chrono::seconds>(
		now - ctx.last_activity
	).count();

	if (duration > Context::TIMEOUT_DURATION.count()) {
		if (ctx.type == CGI) {
			//killTimeoutedCGI(ctx);
		}

		modEpoll(globalFDS.epoll_fd, ctx.client_fd, EPOLLOUT);
		// Cleanup context
		it = globalFDS.request_state_map.erase(it);
	} else {
		++it;
	}
}
}

void Server::killTimeoutedCGI(RequestBody &req) {
	if (req.cgi_pid > 0) {
		kill(req.cgi_pid, SIGTERM);
		//Logger::file(std::to_string(req.associated_conf->timeout));
		std::this_thread::sleep_for(std::chrono::microseconds(req.associated_conf->timeout));
		int status;
		pid_t result = waitpid(req.cgi_pid, &status, WNOHANG);
		if (result == 0) {
			kill(req.cgi_pid, SIGKILL);
			waitpid(req.cgi_pid, &status, 0);
		}
		req.cgi_pid = -1;
	}
	if (req.cgi_in_fd != -1) {
		epoll_ctl(globalFDS.epoll_fd, EPOLL_CTL_DEL, req.cgi_in_fd, NULL);
		close(req.cgi_in_fd);
		globalFDS.svFD_to_clFD_map.erase(req.cgi_in_fd);
		req.cgi_in_fd = -1;
	}
	if (req.cgi_out_fd != -1) {
		epoll_ctl(globalFDS.epoll_fd, EPOLL_CTL_DEL, req.cgi_out_fd, NULL);
		close(req.cgi_out_fd);
		globalFDS.svFD_to_clFD_map.erase(req.cgi_out_fd);
		req.cgi_out_fd = -1;
	}
	if (req.pipe_fd != -1) {
		epoll_ctl(globalFDS.epoll_fd, EPOLL_CTL_DEL, req.pipe_fd, NULL);
		close(req.pipe_fd);
		req.pipe_fd = -1;
	}
}

void Server::logContext(const Context& ctx, const std::string& event)
{
    std::string log = "Context [" + event + "]:\n";

    log += "FDs: epoll=" + (ctx.epoll_fd != -1 ? std::to_string(ctx.epoll_fd) : "[empty]");
    log += " server=" + (ctx.server_fd != -1 ? std::to_string(ctx.server_fd) : "[empty]");
    log += " client=" + (ctx.client_fd != -1 ? std::to_string(ctx.client_fd) : "[empty]") + "\n";

    log += "Type: " + requestTypeToString(ctx.type) + "\n";

    log += "Method: " + (!ctx.method.empty() ? ctx.method : "[empty]") + "\n";
    log += "Path: " + (!ctx.path.empty() ? ctx.path : "[empty]") + "\n";
    log += "Version: " + (!ctx.version.empty() ? ctx.version : "[empty]") + "\n";

    log += "Headers:\n";
    if (ctx.headers.empty())
    {
        log += "[empty]\n";
    }
    else
    {
        for (const auto& header : ctx.headers)
        {
            log += header.first + ": " + header.second + "\n";
        }
    }

    log += "Body: " + (!ctx.body.empty() ? ctx.body : "[empty]") + "\n";

    log += "Location Path: " + (!ctx.location_path.empty() ? ctx.location_path : "[empty]") + "\n";
    log += "Requested Path: " + (!ctx.requested_path.empty() ? ctx.requested_path : "[empty]") + "\n";

    log += std::string("Location: ") + (ctx.location ? "Defined" : "[empty]") + "\n";


    log += "Last Activity: " +
           std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                              ctx.last_activity.time_since_epoch())
                              .count()) +
           " seconds since epoch";

    Logger::file(log);
}


std::string Server::requestTypeToString(RequestType type)
{
	switch(type) {
		case RequestType::INITIAL: return "INITIAL";
		case RequestType::STATIC: return "STATIC";
		case RequestType::CGI: return "CGI";
		case RequestType::ERROR: return "ERROR";
		default: return "UNKNOWN";
	}
}

std::string Server::normalizePath(const std::string& raw)
{
	if (raw.empty()) return "/";
	std::string path = (raw[0] == '/') ? raw : "/" + raw;
	if (path.size() > 1 && path.back() == '/')
		path.pop_back();
	return path;
}

bool Server::matchLoc(const Location& loc, std::string rawPath)
{
	std::string path = normalizePath(rawPath);
	if (path == loc.path)
		return true;
	return false;
}

void Server::logRequestBodyMapFDs()
{
    Logger::file("Logging all FDs in request_state_map:");

    if (globalFDS.request_state_map.empty())
    {
        Logger::file("[empty]");
        return;
    }

    std::string log = "Active FDs: ";
    for (const auto& pair : globalFDS.request_state_map)
    {
        log += std::to_string(pair.first) + " ";
    }

    Logger::file(log);
}

std::string Server::concatenatePath(const std::string& root, const std::string& path)
{
    if (root.empty())
        return path;
    if (path.empty())
        return root;

    if (root.back() == '/' && path.front() == '/')
    {
        return root + path.substr(1);  // Entferne das führende '/'
    }
    else if (root.back() != '/' && path.front() != '/')
    {
        return root + '/' + path;  // Füge einen '/' hinzu
    }
    else
    {
        return root + path;  // Einfach zusammenfügen
    }
}

std::string Server::getDirectory(const std::string& path) {
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return path.substr(0, lastSlash);
    }
    return "";
}
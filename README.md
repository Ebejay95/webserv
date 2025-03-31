# Webserv

## Introduction
Webserv is a custom HTTP server implementation written in C++98. This project aims to understand the HTTP protocol at a deeper level by building a functioning web server from scratch, capable of serving static websites, handling file uploads, and executing CGI scripts.

> "This is when you finally understand why a URL starts with HTTP"

## Features
- HTTP/1.1 compatible server
- Fully configurable via configuration files
- Non-blocking I/O operations
- Multiple server and port listening
- HTTP methods: GET, POST, DELETE
- Static file serving
- Directory listing
- File uploads
- Custom error pages
- CGI script execution
- URL routing and redirection

## Requirements
- C++98 compatible compiler
- Make

## Build Instructions
Clone the repository and compile with make:

```bash
git clone <repository-url>
cd webserv
make
```

## Usage
Run the server with an optional configuration file:

```bash
./webserv [configuration_file]
```

If no configuration file is provided, the server will use a default configuration path.

## Configuration File Format
The configuration file follows a structure inspired by NGINX. Example:

```
server {
    listen      8080;
    server_name example.com;
    error_page  404 /404.html;
    client_max_body_size 10M;

    location / {
        root /var/www/html;
        index index.html;
        methods GET POST;
        autoindex on;
    }

    location /upload {
        root /var/www/uploads;
        methods POST;
        upload_store /var/www/uploads;
    }

    location /api {
        root /var/www/api;
        methods GET POST DELETE;
        cgi_pass .php /usr/bin/php-cgi;
    }

    location /redirect {
        return 301 https://example.com;
    }
}

server {
    listen      8081;
    server_name admin.example.com;
    # Further configuration...
}
```

### Configuration Options:

#### Server Level
- `listen` - Port number to listen on
- `server_name` - Domain name for the server
- `error_page` - Custom error pages
- `client_max_body_size` - Maximum allowed size of client request body

#### Location Level
- `root` - Root directory for file serving
- `index` - Default file to serve for directory requests
- `methods` - Allowed HTTP methods
- `autoindex` - Enable/disable directory listing
- `cgi_pass` - Configure CGI execution for specific file extensions
- `upload_store` - Directory for uploaded files
- `return` - Configure HTTP redirections

## Implementation Details

### Core Components
- HTTP Parser: Parses and validates HTTP requests
- Request Handler: Routes requests to appropriate handlers
- Configuration Parser: Reads and validates server configuration
- Socket Manager: Handles non-blocking I/O with poll/select/epoll/kqueue
- CGI Handler: Executes and manages CGI scripts
- MIME Type Manager: Determines content types for responses
- Error Handler: Generates appropriate error responses
- Logger: Records server activity and errors

### Technical Constraints
- Non-blocking I/O with select/poll/epoll/kqueue
- No execve of another web server
- No blocking operations
- Single poll (or equivalent) for all I/O operations
- No errno checking after read/write operations
- Proper error handling and resource management
- Compatible with modern web browsers

## Testing
The server has been tested with:
- Browser compatibility tests
- Stress tests using concurrent connections
- Compliance tests for HTTP/1.1 specifications
- File upload and download tests
- CGI execution tests
- Error handling tests

## References
- [RFC 7230](https://tools.ietf.org/html/rfc7230) - HTTP/1.1 Message Syntax and Routing
- [RFC 7231](https://tools.ietf.org/html/rfc7231) - HTTP/1.1 Semantics and Content
- [RFC 3875](https://tools.ietf.org/html/rfc3875) - The Common Gateway Interface (CGI) Version 1.1

## License
This project is licensed under the [Your License] - see the LICENSE file for details.

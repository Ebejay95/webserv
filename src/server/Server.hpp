/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jeberle <jeberle@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/29 12:40:21 by fkeitel           #+#    #+#             */
/*   Updated: 2024/12/06 09:09:07 by jeberle          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <sys/epoll.h>     // For epoll
#include <fcntl.h>         // For fcntl (to set non-blocking mode)
#include "../requests/RequestHandler.hpp"
#include "../utils/ConfigHandler.hpp"
#include <fcntl.h>         // For fcntl (to set non-blocking mode)

#define MAX_EVENTS 100

struct ServerBlock;

class Server
{
private:
	std::map<int, const ServerBlock*> serverBlockConfigs;  // Map client FDs to configurations
	const std::vector<ServerBlock>* configs = nullptr; // Pointer to server configurations
	struct epoll_event changes[MAX_EVENTS];             // Events to register with epoll
	struct epoll_event events[MAX_EVENTS];              // Array to store triggered events
	int epoll_fd = -1;                                  // epoll instance
	int num_fds = 0;                                    // Number of triggered events
	int client_fd = -1;                                 // Current client file descriptor
	std::set<int> activeFds;                            // Track active file descriptors

public:
	int create_server_socket(int port);                 // Create a server socket for the given port
	void start(const std::vector<ServerBlock>& configs); // Start the server
	int handleServerEvent(int serv_fd, const ServerBlock& serverConfig); // Handle server events
	void process_events(int num_events);                // Process triggered events
	void close_everything(void);                        // Close all FDs and epoll instance
	std::vector<ServerBlock>::const_iterator define_config(int fd, const std::vector<ServerBlock>& configList);

};

#endif

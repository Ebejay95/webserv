/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helper.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jeberle <jeberle@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/01 10:32:07 by fkeitel           #+#    #+#             */
/*   Updated: 2024/12/11 12:25:19 by jeberle          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "helper.hpp"

// Helper function to set a socket to non-blocking mode
//  Purpose of Non-Blocking Mode
//	By default sockets in C/C++ are blocking. This means that if operation -
//		(like read, accept) cannot proceed immediately, program will pause -
//		(block) until the operation completes.
//	In non-blocking mode, socket returns control to program immediately -
//		if oper. can't proceed, allow program handle other tasks in meantime.
void setNonBlocking(int fd)
{
	int flags;

	// Get the current flags of the file descriptor
	flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK); // Set fd to non-blocking mode
}

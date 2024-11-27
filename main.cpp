/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jeberle <jeberle@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/26 12:58:21 by jeberle           #+#    #+#             */
/*   Updated: 2024/11/27 14:02:16 by jeberle          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./Logger.hpp"

int main() {
	Logger::red("This is a simple red message");
	Logger::cyan() << "This is a " << "streamed cyan " << "message";
	Logger::magenta() << "This is a magenta message";
	return 0;
}

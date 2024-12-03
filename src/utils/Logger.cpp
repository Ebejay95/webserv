/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jeberle <jeberle@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/27 12:39:19 by jeberle           #+#    #+#             */
/*   Updated: 2024/11/28 13:13:00 by jeberle          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./Logger.hpp"

void Logger::red(const std::string &message, bool newline, size_t length) {
	log(formatMessage(message, length), RED, newline);
}

void Logger::green(const std::string &message, bool newline, size_t length) {
	log(formatMessage(message, length), GREEN, newline);
}

void Logger::blue(const std::string &message, bool newline, size_t length) {
	log(formatMessage(message, length), BLUE, newline);
}

void Logger::yellow(const std::string &message, bool newline, size_t length) {
	log(formatMessage(message, length), YELLOW, newline);
}

void Logger::cyan(const std::string &message, bool newline, size_t length) {
	log(formatMessage(message, length), CYAN, newline);
}

void Logger::magenta(const std::string &message, bool newline, size_t length) {
	log(formatMessage(message, length), MAGENTA, newline);
}

void Logger::white(const std::string &message, bool newline, size_t length) {
	log(formatMessage(message, length), WHITE, newline);
}

std::string Logger::formatMessage(const std::string &message, size_t length) {
	if (message.length() >= length) {
		return message;
	}
	return message + std::string(length - message.length(), ' ');
}

void Logger::log(const std::string &message, const std::string &color, bool newline) {
	std::cout << color << message << RESET;
	if (newline)
		std::cout << std::endl;
}

void Logger::error(const std::string &message, bool newline) {
	std::cerr << RED << message << RESET;
	if (newline)
		std::cerr << std::endl;
}

Logger::StreamLogger::StreamLogger(const std::string &colorParam, bool newlineParam, bool useCerrParam)
	: color(colorParam), newline(newlineParam), useCerr(useCerrParam) {}

Logger::StreamLogger::~StreamLogger() {
	flush();
}

void Logger::StreamLogger::flush() {
	if (useCerr)
		std::cerr << color << buffer.str() << RESET;
	else
		std::cout << color << buffer.str() << RESET;

	if (newline) {
		if (useCerr)
			std::cerr << std::endl;
		else
			std::cout << std::endl;
	}
}

Logger::StreamLogger &Logger::red() {
	static StreamLogger logger(RED);
	return logger;
}

Logger::StreamLogger &Logger::green() {
	static StreamLogger logger(GREEN);
	return logger;
}

Logger::StreamLogger &Logger::blue() {
	static StreamLogger logger(BLUE);
	return logger;
}

Logger::StreamLogger &Logger::yellow() {
	static StreamLogger logger(YELLOW);
	return logger;
}

Logger::StreamLogger &Logger::cyan() {
	static StreamLogger logger(CYAN);
	return logger;
}

Logger::StreamLogger &Logger::magenta() {
	static StreamLogger logger(MAGENTA);
	return logger;
}

Logger::StreamLogger &Logger::white() {
	static StreamLogger logger(WHITE);
	return logger;
}

Logger::StreamLogger &Logger::error() {
	static StreamLogger logger(RED, true, true);
	return logger;
}
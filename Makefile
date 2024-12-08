# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jeberle <jeberle@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/11/26 12:56:51 by jeberle           #+#    #+#              #
#    Updated: 2024/12/09 15:30:22 by jeberle          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#------------------------------------------------------------------------------#
#--------------                       PRINT                       -------------#
#------------------------------------------------------------------------------#


BLACK := \033[90m
RED := \033[31m
GREEN := \033[32m
YELLOW := \033[33m
BLUE := \033[34m
MAGENTA := \033[35m
CYAN := \033[36m
X := \033[0m

SUCCESS := \n\
$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)\n\
$(X)\n\
█     █  ███████  ██████   ███████  ███████  ███████  █     █$(X)\n\
█     █  █        █     █  █        █        █     █  █     █$(X)\n\
█  █  █  ███████  ██████   ███████  ███████  ███████   █   █ $(X)\n\
█ █ █ █  █        █     █        █  █        █   █      █ █  $(X)\n\
$(BLACK)_$(X)█   █   ███████  ██████   ███████  ███████  █    ██     █   $(X)\n\
$(X) Wir muessen noch alle config werte im req handler abfangen!!!!!!\n\
$(X) Und 404 Pages !!!!!!\n\
$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)$(GREEN)█$(X)$(YELLOW)█$(X)\n\

#------------------------------------------------------------------------------#
#--------------                      GENERAL                      -------------#
#------------------------------------------------------------------------------#

NAME=webserv

#------------------------------------------------------------------------------#
#--------------                       FLAGS                       -------------#
#------------------------------------------------------------------------------#

CC=c++
CFLAGS=-Wall -Wextra -Werror -Wshadow -std=c++11
LDFLAGS=

ifeq ($(DEBUG), 1)
	CFLAGS += -fsanitize=address -g
endif

DEPFLAGS=-MMD -MP

#------------------------------------------------------------------------------#
#--------------                        DIR                        -------------#
#------------------------------------------------------------------------------#

OBJ_DIR := ./obj
DEP_DIR := $(OBJ_DIR)/.deps
INC_DIRS := .
SRC_DIRS := .

vpath %.cpp $(SRC_DIRS)
vpath %.h $(INC_DIRS)
vpath %.d $(DEP_DIR)

#------------------------------------------------------------------------------#
#--------------                        SRC                        -------------#
#------------------------------------------------------------------------------#

SRCS=	src/main.cpp \
		src/utils/Logger.cpp \
		src/utils/ConfigHandler.cpp \
		src/utils/Sanitizer.cpp \
		src/server/Server.cpp \
		src/requests/RequestHandler.cpp \
		src/helpers/helper.cpp

#------------------------------------------------------------------------------#
#--------------                      OBJECTS                      -------------#
#------------------------------------------------------------------------------#

OBJECTS := $(addprefix $(OBJ_DIR)/, $(SRCS:%.cpp=%.o))

#------------------------------------------------------------------------------#
#--------------                      COMPILE                      -------------#
#------------------------------------------------------------------------------#

.PHONY: all clean fclean re

all: $(NAME)

-include $(OBJECTS:.o=.d)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(NAME): $(OBJECTS)
	@$(CC) -o $@ $^ $(LDFLAGS)
	@echo "$(SUCCESS)"

container:
	@if ! docker ps | grep -q webserv; then \
		echo "$(YELLOW)container not up, build environment$(X)"; \
		docker-compose up --build -d; \
	else \
		echo "$(YELLOW)container already running.. skip it's creation and try build webserv...$(X)"; \
	fi
	@docker exec -it webserv bash

prune:
	@if docker ps -a | grep -q webserv; then \
		echo "$(RED)Removing existing container...$(X)"; \
		docker stop webserv && docker rm webserv; \
	else \
		echo "$(YELLOW)No container named 'webserv' to remove.$(X)"; \
	fi
	@echo "$(GREEN)All done!$(X)"

clean:
	@rm -rf $(OBJ_DIR)
	@echo "$(RED)objects deleted$(X)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(RED)binaries deleted$(X)"

re: fclean all
CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic -Iinc -g3
CXXFLAGS += -fsanitize=address

SRCDIR = src/

SRCFIL = main.cpp BlockOBJ.cpp ConfigParser.cpp Engine.cpp Location.cpp Http.cpp Server.cpp\
		Request.cpp ConnectionManager.cpp Cgi.cpp Response.cpp CGIinfo.cpp

SRCS = $(addprefix $(SRCDIR),$(SRCFIL))

OBJS = $(SRCS:.cpp=.o)

NAME = webserv

all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJS)

fclean: clean
	@rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re

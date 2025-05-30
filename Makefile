CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic -I./inc/ -g3 -fsanitize=address
SRCDIR = ./src/
SRCFIL = main.cpp BlockOBJ.cpp ConfigParser.cpp Engine.cpp Location.cpp Http.cpp Server.cpp\
		Request.cpp ConnectionManager.cpp Cgi.cpp
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
	@rm -rf a.out

fclean: clean
	@rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re

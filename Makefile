CXX			=	g++

IFLAGS 		=	-I./include -I./imgui

CXXFLAGS	=	-W -Wall -Wextra -Wno-switch $(IFLAGS) -std=c++17 -g3

LDFLAGS		=	-lsfml-graphics -lsfml-window -lsfml-audio -lsfml-network -lsfml-system -lGL -lGLU

SRC_LIBS 	=	$(wildcard ./imgui/*.cpp)

SRC			=	$(SRC_LIBS) \
				main.cpp

OBJ 		=	$(SRC:.cpp=.o)

NAME 		=	bin

all:
	make -C . bin -j

bin: $(OBJ)
	$(CXX) -o $(NAME) $(OBJ) $(LDFLAGS)

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re:
	make -C . fclean -j
	make -C . -j

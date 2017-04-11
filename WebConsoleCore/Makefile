TYPE=shared# shared, static or exec
BUILD=debug# debug or release
EXEC=WebConsoleCore
CXXFLAGS=-Wall -std=c++11 -DLINUX -DSSLSOCKET
CXXFLAGS_debug=-g
CXXFLAGS_release=-s -O2
LDFLAGS=-ldl -lpthread -L../mbedtls-2.2.1/library -lmbedtls -lmbedx509 -lmbedcrypto
LDFLAGS_debug=
LDFLAGS_release=
INCLUDES=-I../mbedtls-2.2.1/include

srcdir=./src
hdir=./src
objdir=./obj
bindir=./bin
CXX=g++
AR=ar

CXXFLAGS+= $(CXXFLAGS_$(BUILD)) -I$(hdir) $(INCLUDES)
ifeq ($(TYPE),shared)
	CXXFLAGS+= -fPIC
endif
LDFLAGS+= $(LDFLAGS_$(BUILD))
SRC=$(wildcard $(srcdir)/*.cpp)
OBJ=$(addprefix $(objdir)/,$(notdir $(SRC:.cpp=.o)))

all: $(EXEC)
	@echo "$(EXEC) built ($(TYPE) $(BUILD))"

$(EXEC): $(OBJ)
ifeq ($(TYPE),static)
	@$(AR) -q $(bindir)/lib$@.a $^
endif
ifeq ($(TYPE),exec)
	@$(CXX) -o $(bindir)/$@ $^ $(LDFLAGS)
endif
ifeq ($(TYPE),shared)
	@$(CXX) -shared -fPIC -o $(bindir)/lib$@.so $^ $(LDFLAGS)
endif
	
#----DEPENDENCES----
#ex: main.o: hello.h

$(objdir)/%.o: $(srcdir)/%.cpp
	@$(CXX) -c $(CXXFLAGS) -o $@ $<

.PHONY: clean mrproper

clean:
	@rm -rf $(objdir)/*.o

mrproper: clean
	@rm -rf $(bindir)/$(EXEC)

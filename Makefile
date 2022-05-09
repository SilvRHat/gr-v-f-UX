# Gr@v:f/UX
# Gavin:Zimmerman
# Makefile


# FILES //
EXE = final
SYS_SRC  = main.c camera.c noise.c
SYS_HDR  = main.h camera.h noise.h
VRS_SRC  = grav-flx.c
VRS_HDR  = grav-flx.h
MVRE_SRC = objects.c render.c utils.c signal.c
MVRE_HDR = objects.h render.h utils.h signal.h graphics.h MvRE.h 


# SETUP
CC = gcc
DFLG = -DDEVMODE


# CROSS-PLATFORM SETUP //
#  Msys/MinGW
ifeq "$(OS)" "Windows_NT"
CFLG = -O3 -Wall -DUSEGLEW -DGLFW
LIBS = -lglfw3 -lglew32 -lglu32 -lopengl32 -lm
CLEAN = rm -f *.exe *.o *.a
else

#  OSX
ifeq "$(shell uname)" "Darwin"
CFLG = -O3 -Wall -Wno-deprecated-declarations -DGLFW
LIBS = -lglfw -framework OpenGL -framework IOKit

#  Linux/Unix/Solaris
else
CFLG = -O3 -Wall -DGLFW
LIBS = -lglfw -lGL -lm
endif

#  OSX/Linux/Unix/Solaris
CLEAN = rm -f $(EXE) *.o *.a
endif


# COMPILATION / LINKING //
all: $(EXE)
SRC = $(SYS_SRC) $(VRS_SRC) $(MVRE_SRC)
HDR = $(SYS_HDR) $(VRS_HDR) $(MVRE_HDR)
SRCO = $(SRC:.c=.o)

$(EXE): $(SRCO)
	$(CC) $(CFLG) $(DFLG) -o $@ $^ $(LIBS)

%.o: %.c $(HDR)
	$(CC) $(CFLG) $(DFLG) -c $< -o $@

clean:
	$(CLEAN)
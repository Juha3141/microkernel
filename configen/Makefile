TARGET = configen

OBJ = $(subst .cpp,.obj,$(wildcard *.cpp))

CCOPTIONS = 

clean:
	rm -rf $(TARGET)
	rm -rf *.obj

all: $(OBJ)
	g++ $(OBJ) -o $(TARGET)

%.obj: %.cpp
	g++ -m64 -c $< -o $@ $(CCOPTIONS)

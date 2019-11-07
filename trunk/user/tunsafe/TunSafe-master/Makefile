CXXFLAGS := -I . -O2 -DNDEBUG -DWITH_NETWORK_BSD=1 -ffunction-sections -fdata-sections
LDFLAGS := -Wl,--gc-sections -static -lrt -lpthread -Wl,-Bstatic -latomic

all: tunsafe

tunsafe: tunsafe_amalgam.o
	$(CXX) -o tunsafe tunsafe_amalgam.o $(LDFLAGS)

tunsafe_amalgam.o: tunsafe_amalgam.cpp
	$(CXX) $(CXXFLAGS) -c tunsafe_amalgam.cpp

clean:
	rm -f tunsafe *.o


src = $(wildcard ./*.cpp)
objs = $(patsubst %.cpp,%.o,$(src))

target = client

$(target):$#(objs)
	g++ $(objs) -o $(target)

%.o:%.cpp
	g++ -c $< -o $@

clean:
	rm $(objs) -f
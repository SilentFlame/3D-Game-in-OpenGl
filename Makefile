all: sample3D 

sample2: Sample_GL3_3D.cpp glad.c
	g++ -o sample3D Sample_GL3.cpp glad.c -lGL -lglfw

clean:
	rm  sample3D

ttk_add_base_library(persistenceDiagram
	SOURCES PersistenceDiagram.cpp
	HEADERS PersistenceDiagram.h
	LINK discreteGradient triangulation
		morseSmaleComplex3D ftmTreePP)


CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(debugutils_SRCS 
	DebugDraw.cpp
	DetourDebugDraw.cpp
	RecastDebugDraw.cpp
	RecastDump.cpp
)

SET(debugutils_HDRS
	DebugDraw.h
	DetourDebugDraw.h
	RecastDebugDraw.h
	RecastDump.h
)

INCLUDE_DIRECTORIES(Include 
	../Detour
	../DetourTileCache
	../Recast
)

ADD_LIBRARY(DebugUtils ${debugutils_SRCS} ${debugutils_HDRS})

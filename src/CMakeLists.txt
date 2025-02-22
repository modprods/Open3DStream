cmake_minimum_required(VERSION  3.13)

project(Open3DStreamLib)

#include_directories( ${CMAKE_CURRENT_LIST_DIR}/../include)
#link_directories( ${CMAKE_CURRENT_LIST_DIR}/../lib)		    
            
set(SOURCES 
	o3ds/getTime.cpp
	o3ds/getTime.h
	o3ds/model.cpp	
	o3ds/model.h
	o3ds/base_connector.h
	o3ds/base_connector.cpp
	o3ds/nng_connector.h
	o3ds/nng_connector.cpp
	o3ds/subscriber.cpp
	o3ds/subscriber.h
	o3ds/async_subscriber.cpp
	o3ds/async_subscriber.h
	o3ds/async_publisher.cpp
	o3ds/async_publisher.h
	o3ds/pair.cpp
	o3ds/pair.h
	o3ds/async_pair.cpp
	o3ds/async_pair.h
	o3ds/publisher.h
	o3ds/publisher.cpp
	o3ds/async_request.h
	o3ds/async_request.cpp
	o3ds/request.h
	o3ds/request.cpp
	o3ds/pipeline.h
	o3ds/pipeline.cpp
	o3ds/async_pipeline.h
	o3ds/async_pipeline.cpp
	o3ds/context.h
	o3ds/context.cpp
	o3ds/math.h
	o3ds/math.cpp
	o3ds/transform_component.h
	o3ds/o3ds.h
	o3ds/websocket.h
	o3ds/websocket.cpp
	)
	

set(libs debug flatbuffersd optimized flatbuffers debug nngd optimized nng)
if(WIN32)
set(libs wsock32 ws2_32 ${libs})
endif()

# Shared library
# add_library(open3dstream SHARED ${SOURCES})
#target_link_libraries(open3dstream PRIVATE ${libs}  )
# add_dependencies(open3dstream protocol)


# Static linkable library
add_library(open3dstreamstatic STATIC ${SOURCES})
target_link_libraries(open3dstreamstatic PRIVATE ${libs})        
add_dependencies(open3dstreamstatic protocol)

set(pub_headers
    o3ds/getTime.h
	o3ds/model.h   
	o3ds/base_connector.h
	o3ds/nng_connector.h
	o3ds/subscriber.h
	o3ds/async_subscriber.h
	o3ds/async_publisher.h
	o3ds/pair.h
	o3ds/async_pair.h
	o3ds/publisher.h
	o3ds/async_request.h
	o3ds/request.h
	o3ds/pipeline.h
	o3ds/async_pipeline.h
	o3ds/context.h
	o3ds/math.h
	o3ds/transform_component.h
	o3ds/o3ds.h
	o3ds/websocket.h )


list(JOIN pub_headers ";" pub_headers_str)
set_target_properties(open3dstreamstatic PROPERTIES PUBLIC_HEADER "${pub_headers_str}")

install(TARGETS open3dstreamstatic 
    LIBRARY DESTINATION lib
	PUBLIC_HEADER DESTINATION include/o3ds)



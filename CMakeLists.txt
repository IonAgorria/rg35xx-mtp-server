project(mtpserver)

cmake_minimum_required(VERSION 2.8)

include(FindPkgConfig)
include(GNUInstallDirs)

set(Boost_USE_STATIC_LIBS   ON)
add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_definitions(-DMTP_DEVICE -DMTP_HOST)

set(MTP_VERSION_MAJOR 1)
set(MTP_VERSION_MINOR 0)
set(MTP_VERSION_PATCH 0)

find_package(Boost REQUIRED COMPONENTS thread system filesystem)

set(
    MTP_HEADERS
    include/MtpDatabase.h
    include/MtpDataPacket.h
    include/MtpDebug.h
    include/MtpDevice.h
    include/MtpDeviceInfo.h
    include/MtpEventPacket.h
    include/mtp.h
    include/MtpObjectInfo.h
    include/MtpPacket.h
    include/MtpProperty.h
    include/MtpRequestPacket.h
    include/MtpResponsePacket.h
    include/MtpServer.h
    include/MtpStorage.h
    include/MtpStorageInfo.h
    include/MtpStringBuffer.h
    include/MtpTypes.h
    include/MtpUtils.h
)

set(
    MTP_SRCS
    src/MtpDataPacket.cpp
    src/MtpDebug.cpp
    src/MtpDevice.cpp
    src/MtpDeviceInfo.cpp
    src/MtpEventPacket.cpp
    src/MtpObjectInfo.cpp
    src/MtpPacket.cpp
    src/MtpProperty.cpp
    src/MtpRequestPacket.cpp
    src/MtpResponsePacket.cpp
    src/MtpServer.cpp
    src/MtpStorage.cpp
    src/MtpStorageInfo.cpp
    src/MtpStringBuffer.cpp
    src/MtpUtils.cpp)
    
include_directories(
    include/
    libusbhost/include
    ${Boost_INCLUDE_DIRS}
)

add_library(
    mtpserver STATIC
    ${MTP_SRCS}
)

#enable_testing()

add_subdirectory(libusbhost)
add_subdirectory(server)
#add_subdirectory(tests)

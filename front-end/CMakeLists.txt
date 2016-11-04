cmake_minimum_required(VERSION 2.8.7)

project(front-end)

include(ExternalProject)

ExternalProject_Add(
	qemu-2.3
        DOWNLOAD_COMMAND ""
	UPDATE_COMMAND ""
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/qemu-2.3
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/qemu-2.3/crete_configure.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
	BUILD_COMMAND make -j7
	INSTALL_COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/qemu-2.3/i386-softmmu/crete-qemu-2.3-system-i386 ${CMAKE_BINARY_DIR}/bin/crete-qemu-2.3-system-i386 && ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/qemu-2.3/x86_64-softmmu/crete-qemu-2.3-system-x86_64 ${CMAKE_BINARY_DIR}/bin/crete-qemu-2.3-system-x86_64
	)

add_dependencies(qemu-2.3 crete_test_case)

add_custom_target(qemu-2.3-remake ALL
  COMMAND  make -j7
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/qemu-2.3
  DEPENDS qemu-2.3)
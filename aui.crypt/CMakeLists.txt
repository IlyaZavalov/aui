cmake_minimum_required(VERSION 3.10)

auib_import(OpenSSL https://github.com/Alex2772/openssl-cmake)

aui_module(aui.crypt)
target_link_libraries(aui.crypt PRIVATE aui.core)
target_include_directories(aui.crypt PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(aui.crypt PRIVATE OpenSSL::SSL OpenSSL::Crypto)
# CMake generated Testfile for 
# Source directory: C:/Users/genos/source/repos/FileFlow/fileflow
# Build directory: C:/Users/genos/source/repos/FileFlow/fileflow/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[fileflow_tests]=] "C:/Users/genos/source/repos/FileFlow/fileflow/build/Debug/fileflow_tests.exe")
  set_tests_properties([=[fileflow_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/genos/source/repos/FileFlow/fileflow/CMakeLists.txt;114;add_test;C:/Users/genos/source/repos/FileFlow/fileflow/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[fileflow_tests]=] "C:/Users/genos/source/repos/FileFlow/fileflow/build/Release/fileflow_tests.exe")
  set_tests_properties([=[fileflow_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/genos/source/repos/FileFlow/fileflow/CMakeLists.txt;114;add_test;C:/Users/genos/source/repos/FileFlow/fileflow/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[fileflow_tests]=] "C:/Users/genos/source/repos/FileFlow/fileflow/build/MinSizeRel/fileflow_tests.exe")
  set_tests_properties([=[fileflow_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/genos/source/repos/FileFlow/fileflow/CMakeLists.txt;114;add_test;C:/Users/genos/source/repos/FileFlow/fileflow/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[fileflow_tests]=] "C:/Users/genos/source/repos/FileFlow/fileflow/build/RelWithDebInfo/fileflow_tests.exe")
  set_tests_properties([=[fileflow_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/genos/source/repos/FileFlow/fileflow/CMakeLists.txt;114;add_test;C:/Users/genos/source/repos/FileFlow/fileflow/CMakeLists.txt;0;")
else()
  add_test([=[fileflow_tests]=] NOT_AVAILABLE)
endif()
subdirs("_deps/spdlog-build")
subdirs("_deps/nlohmann_json-build")

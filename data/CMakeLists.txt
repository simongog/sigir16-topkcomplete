SET(base_url "http://xor.di.unipi.it/~rossano")

SET(abs_test_case ${CMAKE_HOME_DIRECTORY}/data/${test_case})
IF(NOT EXISTS ${abs_test_case})
    SET(tc_url ${base_url}/${test_case})
    MESSAGE(${tc_url})
    FILE(DOWNLOAD ${tc_url} ${abs_test_case} SHOW_PROGRESS)
ENDIF()

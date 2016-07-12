SET(base_url "http://xor.di.unipi.it/~rossano")

MESSAGE("test_case=${test_case}")
SET(abs_test_case ${CMAKE_CURRENT_SOURCE_DIR}/${test_case})
MESSAGE("abs_test_case=${abs_test_case}")

IF(NOT EXISTS ${abs_test_case})
    SET(tc_url ${base_url}/${test_case})
    MESSAGE(${tc_url})
    FILE(DOWNLOAD ${tc_url} ${abs_test_case} SHOW_PROGRESS)
ENDIF()
MESSAGE("Download complete; unzipping file...")

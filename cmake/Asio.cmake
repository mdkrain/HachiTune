function(hachitune_setup_asio out_include_dir)
    if(NOT WIN32)
        return()
    endif()

    set(ASIO_SDK_URL "https://download.steinberg.net/sdk_downloads/ASIO-SDK_2.3.4_2025-10-15.zip")
    set(ASIO_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/../third_party/ASIOSDK")
    set(ASIO_SDK_ZIP "${CMAKE_BINARY_DIR}/_deps/asio_sdk.zip")

    file(GLOB_RECURSE ASIO_IASIO_HEADERS "${ASIO_SDK_DIR}/*/iasiodrv.h")
    if(NOT ASIO_IASIO_HEADERS)
        if(NOT EXISTS "${ASIO_SDK_DIR}")
            file(MAKE_DIRECTORY "${ASIO_SDK_DIR}")
        endif()

        message(STATUS "ASIO SDK not found. Downloading from ${ASIO_SDK_URL}")
        file(DOWNLOAD "${ASIO_SDK_URL}" "${ASIO_SDK_ZIP}"
            SHOW_PROGRESS
            STATUS ASIO_DOWNLOAD_STATUS)
        list(GET ASIO_DOWNLOAD_STATUS 0 ASIO_DOWNLOAD_RESULT)
        if(NOT ASIO_DOWNLOAD_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to download ASIO SDK: ${ASIO_DOWNLOAD_STATUS}")
        endif()

        file(ARCHIVE_EXTRACT INPUT "${ASIO_SDK_ZIP}" DESTINATION "${ASIO_SDK_DIR}")
        file(GLOB_RECURSE ASIO_IASIO_HEADERS "${ASIO_SDK_DIR}/*/iasiodrv.h")
    endif()

    list(LENGTH ASIO_IASIO_HEADERS ASIO_HEADER_COUNT)
    if(ASIO_HEADER_COUNT LESS 1)
        message(FATAL_ERROR "ASIO SDK is missing iasiodrv.h. Check ${ASIO_SDK_DIR}")
    endif()

    list(GET ASIO_IASIO_HEADERS 0 ASIO_IASIO_HEADER)
    get_filename_component(ASIO_INCLUDE_DIR "${ASIO_IASIO_HEADER}" DIRECTORY)
    set(${out_include_dir} "${ASIO_INCLUDE_DIR}" PARENT_SCOPE)
endfunction()

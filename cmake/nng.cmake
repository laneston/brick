
# NNG library complie function.
UNSET(LOCAL_NNG_TAR_GZ CACHE)
UNSET(LOCAL_NNG_DIR CACHE)
UNSET(LOCAL_LIBRARIES_PATH CACHE)


# NNG文件夹名称
SET(NNG_DIR_NAME "nng-1.5.2")
# NNG库压缩包名称
SET(NNG_TAR_GZ_NAME "nng.tar.gz")
# NNG远程下载地址URL
SET(NNG_DOWNLOAD_URL "https://codeload.github.com/nanomsg/nng/tar.gz/refs/tags/v1.5.2")
# 设置库远程下载超时时间
SET(DOWNLOAD_NNG_TIMEOUT 300 CACHE STRING "Timeout in seconds when downloading nng_lib.")

MESSAGE(STATUS "Checking ${NNG_TAR_GZ_NAME}...")

# 获取压缩包变量值
FIND_FILE(LOCAL_NNG_TAR_GZ
	NAMES ${NNG_TAR_GZ_NAME}
	HINTS "${PROJECT_SOURCE_DIR}/libraries_path"
	NO_DEFAULT_PATH)

# 获取文件变量值
FIND_FILE(LOCAL_NNG_DIR
	NAMES ${NNG_DIR_NAME}
	HINTS "${PROJECT_SOURCE_DIR}/libraries_path"
	NO_DEFAULT_PATH)

# 获取压缩包路径值
FIND_FILE(LOCAL_LIBRARIES_PATH
	NAMES ""
	HINTS "libraries_path"
	NO_DEFAULT_PATH)




# 判断压缩包是否已下载，如无下载，则进行下载
IF(NOT LOCAL_NNG_TAR_GZ)

	MESSAGE(STATUS "Checking url link ${NNG_DOWNLOAD_URL} ...")
	MESSAGE(STATUS "Downloading ${NNG_TAR_GZ_NAME} to ${LOCAL_LIBRARIES_PATH} ...")
	# 从设定的 URL 地址下载相应的压缩包
	FILE(DOWNLOAD ${NNG_DOWNLOAD_URL}
		${LOCAL_LIBRARIES_PATH}/${NNG_TAR_GZ_NAME}
		TIMEOUT ${DOWNLOAD_NNG_TIMEOUT}
		STATUS ERR
		SHOW_PROGRESS)

	# 判断下载是否存在错误
	IF(ERR EQUAL 0)
		# 如果下载无错误则设置压缩包名称标记下载成功
		SET(LOCAL_NNG_TAR_GZ "${LOCAL_LIBRARIES_PATH}/${NNG_TAR_GZ_NAME}")
	#下载错误输出相关的提示信息
	ELSE()
		MESSAGE(STATUS "Download failed, error: ${ERR}")
		MESSAGE(FATAL_ERROR
		"You can try downloading ${NNG_DOWNLOAD_URL} manually"
		"using curl/wget or a similar tool")
	ENDIF()
	
ENDIF()



IF(LOCAL_NNG_DIR)

	MESSAGE(STATUS "${LOCAL_NNG_DIR} IS EXIST.")

ELSE(LOCAL_NNG_DIR)

	IF(LOCAL_NNG_TAR_GZ)

		# 输出相应的提示信息校验解压路径等
		MESSAGE(STATUS "Checking the direction of ${LOCAL_LIBRARIES_PATH}/${NNG_DIR_NAME}")
		IF(NOT EXISTS "${LOCAL_LIBRARIES_PATH}/${NNG_DIR_NAME}")
			# 输出提示信息准备进行解压缩
			MESSAGE(STATUS "cd ${LOCAL_LIBRARIES_PATH} && tar -zxvf ${NNG_TAR_GZ_NAME}")
			# 获取已下载的压缩包大小做基本的判断
			FILE(SIZE ${LOCAL_NNG_TAR_GZ} NNG_TAR_GZ_SIZE)
			MESSAGE(STATUS "${LOCAL_NNG_TAR_GZ} size is ${NNG_TAR_GZ_SIZE}")

			# 如果压缩包大小为0则输出对应的错误信息
			IF(${NNG_TAR_GZ_SIZE} EQUAL 0)
				# 移除损坏压缩包，重置相应的参数
				FILE(REMOVE ${LOCAL_NNG_TAR_GZ})
				MESSAGE(FATAL_ERROR "${NNG_TAR_GZ_SIZE} is zero length. it had been deleted.")
			# 下载文件无问题，进入解压路径开始解压压缩包
			ELSE()
			
				# 解压文件
				EXECUTE_PROCESS(COMMAND tar -zxvf ${LOCAL_NNG_TAR_GZ} -C ${LOCAL_LIBRARIES_PATH}/
					WORKING_DIRECTORY "${LOCAL_LIBRARIES_PATH}"
					RESULT_VARIABLE 10
					RESULT_VARIABLE RESULT_TAR)
				IF(RESULT_TAR MATCHES 0)
					MESSAGE(STATUS "tar -zxvf ${LOCAL_NNG_TAR_GZ} -C ${LOCAL_LIBRARIES_PATH}/")
				ELSE()
					MESSAGE(FATAL_ERROR "[ERROR] tar -zxvf ${LOCAL_NNG_TAR_GZ} -C ${LOCAL_LIBRARIES_PATH}/ failed.\n")
				ENDIF()
				UNSET(RESULT_TAR)
			ENDIF()

		ENDIF()

	ENDIF()

ENDIF(LOCAL_NNG_DIR)





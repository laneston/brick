
# mqtt library complie function.
UNSET(LOCAL_MQTT_TAR_GZ CACHE)
UNSET(LOCAL_MQTT_DIR CACHE)
UNSET(LOCAL_LIBRARIES_PATH CACHE)


# MQTT文件夹名称
SET(MQTT_DIR_NAME "paho.mqtt.c-1.3.12")
# MQTT库压缩包名称
SET(MQTT_TAR_GZ_NAME "paho.mqtt.c-1.3.12.tar.gz")
# MQTT远程下载地址URL
SET(MQTT_DOWNLOAD_URL "https://codeload.github.com/eclipse/paho.mqtt.c/tar.gz/refs/tags/v1.3.12")
# 设置库远程下载超时时间
SET(DOWNLOAD_MQTT_TIMEOUT 300 CACHE STRING "Timeout in seconds when downloading mqtt_lib.")

MESSAGE(STATUS "Checking ${MQTT_TAR_GZ_NAME}...")

# 判断压缩包是否已经存在
FIND_FILE(LOCAL_MQTT_TAR_GZ
	NAMES ${MQTT_TAR_GZ_NAME}
	HINTS "${PROJECT_SOURCE_DIR}/libraries_path"
	NO_DEFAULT_PATH)

# 判断压缩包在文件夹libraries_path下是否已经解压
FIND_FILE(LOCAL_MQTT_DIR
	NAMES ${MQTT_DIR_NAME}
	HINTS "${PROJECT_SOURCE_DIR}/libraries_path"
	NO_DEFAULT_PATH)

FIND_FILE(LOCAL_LIBRARIES_PATH
	NAMES ""
	HINTS "libraries_path"
	NO_DEFAULT_PATH)

MESSAGE(STATUS "Checking url link ${MQTT_DOWNLOAD_URL} ...")

# 判断压缩包是否存在，路径下不存在，则进行下载
IF(NOT LOCAL_MQTT_TAR_GZ)
	MESSAGE(STATUS "Downloading ${MQTT_TAR_GZ_NAME} to ${LOCAL_LIBRARIES_PATH} ...")
	# 从设定的 URL 地址下载相应的压缩包
	FILE(DOWNLOAD ${MQTT_DOWNLOAD_URL}
		${LOCAL_LIBRARIES_PATH}/${MQTT_TAR_GZ_NAME}
		TIMEOUT ${DOWNLOAD_MQTT_TIMEOUT}
		STATUS ERR
		SHOW_PROGRESS)

	# 判断下载是否存在错误
	IF(ERR EQUAL 0)
		# 如果下载无错误则设置压缩包名称标记下载成功
		SET(LOCAL_MQTT_TAR_GZ "${LOCAL_LIBRARIES_PATH}/${MQTT_TAR_GZ_NAME}")
	#下载错误输出相关的提示信息
	ELSE()
		MESSAGE(STATUS "Download failed, error: ${ERR}")
		MESSAGE(FATAL_ERROR
		"You can try downloading ${MQTT_DOWNLOAD_URL} manually"
		"using curl/wget or a similar tool")
	ENDIF()
	
ENDIF()


# 判断解压文件路径是否存在
IF(LOCAL_MQTT_DIR)
	MESSAGE(STATUS "${LOCAL_NNG_DIR} IS EXIST.")
ELSE(LOCAL_MQTT_DIR)

	IF(LOCAL_MQTT_TAR_GZ)
		# 输出相应的提示信息校验解压路径等
		MESSAGE(STATUS "Checking the direction of ${LOCAL_LIBRARIES_PATH}/${MQTT_DIR_NAME}")
		
		IF(NOT EXISTS "${LOCAL_LIBRARIES_PATH}/${MQTT_DIR_NAME}")
			# 输出提示信息准备进行解压缩
			MESSAGE(STATUS "cd ${LOCAL_LIBRARIES_PATH} && tar -zxvf ${MQTT_TAR_GZ_NAME}")
			# 获取已下载的压缩包大小做基本的判断
			FILE(SIZE ${LOCAL_MQTT_TAR_GZ} MQTT_TAR_GZ_SIZE)
			MESSAGE(STATUS "${LOCAL_MQTT_TAR_GZ} size is ${MQTT_TAR_GZ_SIZE}")

			# 如果压缩包大小为0则输出对应的错误信息
			IF(${MQTT_TAR_GZ_SIZE} EQUAL 0)
				# 移除损坏压缩包，重置相应的参数
				FILE(REMOVE ${LOCAL_MQTT_TAR_GZ})
				MESSAGE(FATAL_ERROR "${MQTT_TAR_GZ_SIZE} is zero length. it had been deleted.")
			# 下载文件无问题，进入解压路径开始解压压缩包
			ELSE()
				# 解压文件
				EXECUTE_PROCESS(COMMAND tar -zxvf ${LOCAL_MQTT_TAR_GZ} -C ${LOCAL_LIBRARIES_PATH}/
								WORKING_DIRECTORY "${LOCAL_LIBRARIES_PATH}"
								RESULT_VARIABLE 10
								RESULT_VARIABLE RESULT_TAR)
				IF(RESULT_TAR MATCHES 0)
					MESSAGE(STATUS "tar -zxvf ${LOCAL_MQTT_TAR_GZ} -C ${LOCAL_LIBRARIES_PATH}/")
				ELSE()
					MESSAGE(FATAL_ERROR "[ERROR] tar -zxvf ${LOCAL_MQTT_TAR_GZ} -C ${LOCAL_LIBRARIES_PATH}/ failed.\n")
				ENDIF()
				UNSET(RESULT_TAR)
			ENDIF()

		ENDIF()

	ENDIF(LOCAL_MQTT_TAR_GZ)

ENDIF(LOCAL_MQTT_DIR)
# ==============================
# generated_files.cmake
# ���ɰ�װ����/ж�س�����Ҫ����Դ�������ļ�
# ����������Ƿ�ɹ�
# ==============================

# --- 1. ����ͷ�ļ� config.h ---
set(CONFIG_H "${CMAKE_CURRENT_LIST_DIR}/../inc/config.h")
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/../inc/config.h.in"
    ${CONFIG_H}
    @ONLY
)

if(NOT EXISTS ${CONFIG_H})
    message(FATAL_ERROR "���������ļ�ʧ��: ${CONFIG_H} ������")
endif()

# --- 2. ��װ����ͨ����Դ�ļ� install.rc ---
set(INSTALL_RC "${CMAKE_CURRENT_LIST_DIR}/../src/install.rc")
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/../src/install.rc.in"
    ${INSTALL_RC}
    @ONLY
)

if(NOT EXISTS ${INSTALL_RC})
    message(FATAL_ERROR "������Դ�ļ�ʧ��: ${INSTALL_RC} ������")
endif()

# --- 3. ��������ʱ��Դ�ļ� online_runtime.rc ---
set(ONLINE_RUNTIME_RC "${CMAKE_CURRENT_LIST_DIR}/../src/online_runtime.rc")
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/../src/online_runtime.rc.in"
    ${ONLINE_RUNTIME_RC}
    @ONLY
)

if(NOT EXISTS ${ONLINE_RUNTIME_RC})
    message(FATAL_ERROR "������Դ�ļ�ʧ��: ${ONLINE_RUNTIME_RC} ������")
endif()

# --- 4. ��������ʱ��Դ�ļ� offline_runtime.rc ---
set(OFFLINE_RUNTIME_RC "${CMAKE_CURRENT_LIST_DIR}/../src/offline_runtime.rc")
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/../src/offline_runtime.rc.in"
    ${OFFLINE_RUNTIME_RC}
    @ONLY
)

if(NOT EXISTS ${OFFLINE_RUNTIME_RC})
    message(FATAL_ERROR "������Դ�ļ�ʧ��: ${OFFLINE_RUNTIME_RC} ������")
endif()
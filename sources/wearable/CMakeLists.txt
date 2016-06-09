CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(wifi CXX C)

SET(RES_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/res/wearable-circle)
SET(SRCS
	src/wearable-circle/app_main.c
	src/wearable-circle/view/base.c
	src/wearable-circle/view/layout_ap_info.c
	src/wearable-circle/view/layout_detail.c
	src/wearable-circle/view/layout_eap_method.c
	src/wearable-circle/view/layout_main.c
	src/wearable-circle/view/layout_password_entry.c
	src/wearable-circle/view/layout_proxy_setting.c
	src/wearable-circle/view/layout_scan.c
	src/wearable-circle/view/layout_static_ip.c
	src/wearable-circle/view/layout_wearable_input.c
	src/wearable-circle/view/layout_wps_method.c
	src/wearable-circle/view/layout_wps_progress.c
	src/wearable-circle/view/popup_change_ap.c
	src/wearable-circle/view/popup_scanning.c
	src/wearable-circle/view/popup_unable_scan.c
	src/wearable-circle/view/toast_popup.c
	src/wearable-circle/view/util/efl_helper.c)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/inc/wearable-circle)

SET(SRCS ${SRCS}
	src/net/wifi_manager.c
	src/net/wifi_generate_pin.c
	src/net/util/wifi_address.c
	src/util/idler.c)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/inc)

SET(ENGINE_PKGCONFIGS
	capi-network-wifi)

SET(PROJECT_NAME "wifi")
SET(PACKAGE ${PROJECT_NAME})
SET(PKGNAME "org.tizen.w-wifi")
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(INSTALL_DIR_APPS "/usr/apps/${PKGNAME}")
SET(INSTALL_DIR_PRE "/usr")

SET(SRCWEARABLE "${CMAKE_SOURCE_DIR}/sources/wearable")

SET(BINDIR "${PREFIX}/apps/${PKGNAME}/bin")
SET(RESDIR "${PREFIX}/apps/${PKGNAME}/res")
SET(SHAREDRESDIR "${PREFIX}/shared/res")
SET(DATADIR "${PREFIX}/data")
SET(LOCALEDIR "${PREFIX}/apps/${PKGNAME}/res/locale")
SET(ICONDIR "${RESDIR}/icons")
SET(DESKTOPICONDIR "${ICONDIR}/default/small")
SET(EDJDIR "${RESDIR}/edje")
SET(TABLEDIR "${SHAREDRESDIR}/tables")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

SET(GC_SECTIONS_FLAGS "-fdata-sections -ffunction-sections -Wl,--gc-sections")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GC_SECTIONS_FLAGS}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GC_SECTIONS_FLAGS}")

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	dlog
	evas
	edje
	vconf
	ecore
	glib-2.0
	efl-assist
	elementary
	bundle
#	ui-extension
	efl-extension
	capi-ui-efl-util
	ecore-input
	capi-network-wifi
	capi-appfw-application
	capi-network-connection)

FIND_LIBRARY(LIB_M m)

ADD_DEFINITIONS(${pkgs_CFLAGS})
ADD_DEFINITIONS("-fpie")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DFACTORYFS=\"$ENV{FACTORYFS}\"")
ADD_DEFINITIONS("-DSLP_DEBUG")
ADD_DEFINITIONS("-DSLP_PROF")

ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")
ADD_DEFINITIONS("-DPACKAGE_NAME=\"${PKGNAME}\"")
ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")

ADD_DEFINITIONS("-DINSTALL_DIR_APPS=\"${INSTALL_DIR_APPS}\"")
ADD_DEFINITIONS("-DSRCWEARABLE=\"${SRCWEARABLE}\"")
ADD_DEFINITIONS("-DBINDIR=\"${BINDIR}\"")
ADD_DEFINITIONS("-DICONDIR=\"${ICONDIR}\"")
ADD_DEFINITIONS("-DEDJDIR=\"${EDJDIR}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
ADD_DEFINITIONS("-DVISIBILITY_DEFAULT=__attribute__ ((visibility(\"default\")))")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} "-pie" ${LIB_M})

ADD_CUSTOM_TARGET(${PROJECT_NAME}.edj
	COMMAND edje_cc -id ${RES_SOURCE_DIR}/images
	${RES_SOURCE_DIR}/edje/${PROJECT_NAME}.edc ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.edj
	DEPENDS ${RES_SOURCE_DIR}/edje/${PROJECT_NAME}.edc
	)

ADD_DEPENDENCIES(${PROJECT_NAME} ${PROJECT_NAME}.edj)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${BINDIR})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.edj DESTINATION ${EDJDIR})


# install application HOME directory
INSTALL(DIRECTORY DESTINATION ${DATADIR})

# desktop icon
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/org.tizen.w-wifi.xml DESTINATION /usr/share/packages)
INSTALL(DIRECTORY ${RES_SOURCE_DIR}/images/ DESTINATION /usr/apps/org.tizen.w-wifi/res/images)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shared/res/tables/ DESTINATION ${TABLEDIR} FILES_MATCHING PATTERN "*.xml")

ADD_SUBDIRECTORY(res/po)

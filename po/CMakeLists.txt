# for i18n

SET(POFILES de_DE.po el_GR.po en.po es_ES.po fr_FR.po it_IT.po ja_JP.po ko_KR.po nl_NL.po pt_PT.po ru_RU.po tr_TR.po zh_CN.po zh_HK.po zh_TW.po)

SET(MSGFMT "/usr/bin/msgfmt")

SET(WIFI_UG_PROJECT_NAME "ug-wifi-efl-UG")
SET(WIFI_QS_PROJECT_NAME "wifi-qs")

FOREACH(pofile ${POFILES})
	SET(pofile ${CMAKE_CURRENT_SOURCE_DIR}/${pofile})
	MESSAGE("PO: ${pofile}")
	GET_FILENAME_COMPONENT(absPofile ${pofile} ABSOLUTE)
	GET_FILENAME_COMPONENT(lang ${absPofile} NAME_WE)
	SET(moFile ${CMAKE_CURRENT_BINARY_DIR}/${lang}.mo)
	ADD_CUSTOM_COMMAND(
			OUTPUT ${moFile}
			COMMAND ${MSGFMT} -o ${moFile} ${absPofile}
			DEPENDS ${absPofile}
	)

	# install for UG
	INSTALL(FILES ${moFile}
			DESTINATION /opt/ug/res/locale/${lang}/LC_MESSAGES RENAME ${WIFI_UG_PROJECT_NAME}.mo)

	# install for QS
	INSTALL(FILES ${moFile}
			DESTINATION /usr/share/locale/${lang}/LC_MESSAGES RENAME ${WIFI_QS_PROJECT_NAME}.mo)

	SET(moFiles ${moFiles} ${moFile})
ENDFOREACH(pofile)

MESSAGE(".mo files: ${moFiles}")
ADD_CUSTOM_TARGET(po ALL DEPENDS ${moFiles})

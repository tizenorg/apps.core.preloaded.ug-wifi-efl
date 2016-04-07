# for i18n

SET(POFILES ca.po en_PH.po fa.po gu.po id.po km.po mk.po nb.po pt_BR.po sl.po th.po vi.po ar.po cs.po en_US.po fi.po he.po is.po kn.po ml.po ne.po pt_PT.po sq.po tl.po zh_CN.po as.po da.po es_ES.po fr.po hi.po it_IT.po ko_KR.po mn_MN.po nl.po ro.po sr.po tr_TR.po zh_HK.po az.po de.po es_US.po fr_CA.po hr.po ja_JP.po lo.po mr.po or.po ru_RU.po sv.po uk.po zh_TW.po bg.po el_GR.po et.po ga.po hu.po ka.po lt.po ms.po pa.po si.po ta.po ur.po bn.po en.po eu.po gl.po hy.po kk.po lv.po my_MM.po my_ZG.po pl.po pl_SP.po sk.po te.po uz.po ky_KG.po tg_TJ.po tk_TM.po be_BY.po bn_BD.po)

SET(MSGFMT "/usr/bin/msgfmt")
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
	INSTALL(FILES ${moFile}
		DESTINATION ${LOCALEDIR}/${lang}/LC_MESSAGES RENAME ${PROJECT_NAME}.mo)
	SET(moFiles ${moFiles} ${moFile})
ENDFOREACH(pofile)

MESSAGE(".mo files: ${moFiles}")
ADD_CUSTOM_TARGET(po ALL DEPENDS ${moFiles})

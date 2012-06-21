# - Find Okteta Kasten libraries, v0, v1 or v2
#
# This module defines
#  LIBOKTETAKASTEN_FOUND - whether the Okteta Kasten libraries were found
#  LIBOKTETAKASTEN_VERSION - version of the Okteta Kasten libraries found
#  LIBOKTETAKASTEN_LIBRARIES - the Okteta Kasten libraries
#  LIBOKTETAKASTEN_INCLUDE_DIRS - the include paths of the Okteta Kasten libraries


if( LIBOKTETAKASTEN_INCLUDE_DIRS AND LIBOKTETAKASTEN_LIBRARIES AND LIBOKTETAKASTEN_VERSION AND
    LIBOKTETAKASTEN_NEEDS_KASTEN_VERSION AND LIBOKTETAKASTEN_NEEDS_OKTETA_VERSION )
    # Already in cache, be silent
    set( OktetaKasten_FIND_QUIETLY TRUE )
endif( LIBOKTETAKASTEN_INCLUDE_DIRS AND LIBOKTETAKASTEN_LIBRARIES AND LIBOKTETAKASTEN_VERSION AND
       LIBOKTETAKASTEN_NEEDS_KASTEN_VERSION AND LIBOKTETAKASTEN_NEEDS_OKTETA_VERSION )


# First search version 2
find_library( LIBOKTETA1KASTEN2CORE_LIBRARY
    NAMES
    kasten2okteta1core
    HINTS
    ${LIB_INSTALL_DIR}
    ${KDE4_LIB_DIR}
)

find_library( LIBOKTETA1KASTEN2GUI_LIBRARY
    NAMES
    kasten2okteta1gui
    HINTS
    ${LIB_INSTALL_DIR}
    ${KDE4_LIB_DIR}
)

find_library( LIBOKTETA1KASTEN2CONTROLLERS_LIBRARY
    NAMES
    kasten2okteta1controllers
    HINTS
    ${LIB_INSTALL_DIR}
    ${KDE4_LIB_DIR}
)


find_path( LIBOKTETA1KASTEN2_INCLUDE_DIR
    NAMES
    bytearraydocument.h
    PATH_SUFFIXES
    kasten2/okteta1
    HINTS
    ${INCLUDE_INSTALL_DIR}
    ${KDE4_INCLUDE_DIR}
)

if( LIBOKTETA1KASTEN2_INCLUDE_DIR AND
    LIBOKTETA1KASTEN2CORE_LIBRARY AND LIBOKTETA1KASTEN2GUI_LIBRARY AND LIBOKTETA1KASTEN2CONTROLLERS_LIBRARY )
    set( LIBOKTETAKASTEN_FOUND  TRUE )
endif( LIBOKTETA1KASTEN2_INCLUDE_DIR AND
    LIBOKTETA1KASTEN2CORE_LIBRARY AND LIBOKTETA1KASTEN2GUI_LIBRARY AND LIBOKTETA1KASTEN2CONTROLLERS_LIBRARY )

if( LIBOKTETAKASTEN_FOUND )
    set( LIBOKTETAKASTEN_VERSION 2 )
    set( LIBOKTETAKASTEN_NEEDS_KASTEN_VERSION 2 )
    set( LIBOKTETAKASTEN_NEEDS_OKTETA_VERSION 1 )
    set( LIBOKTETAKASTEN_LIBRARIES
        ${LIBOKTETA1KASTEN2CORE_LIBRARY}
        ${LIBOKTETA1KASTEN2GUI_LIBRARY}
        ${LIBOKTETA1KASTEN2CONTROLLERS_LIBRARY}
    )
    set( LIBOKTETAKASTEN_INCLUDE_DIRS
        ${LIBOKTETA1KASTEN2_INCLUDE_DIR}
    )
endif( LIBOKTETAKASTEN_FOUND )

# Then search version 1
if( NOT LIBOKTETAKASTEN_FOUND )
    find_library( LIBOKTETA1KASTEN1CORE_LIBRARY
        NAMES
        kasten1okteta1core
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )

    find_library( LIBOKTETA1KASTEN1GUI_LIBRARY
        NAMES
        kasten1okteta1gui
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )

    find_library( LIBOKTETA1KASTEN1CONTROLLERS_LIBRARY
        NAMES
        kasten1okteta1controllers
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )


    find_path( LIBOKTETA1KASTEN1_INCLUDE_DIR
        NAMES
        bytearraydocument.h
        PATH_SUFFIXES
        kasten1/okteta1
        HINTS
        ${INCLUDE_INSTALL_DIR}
        ${KDE4_INCLUDE_DIR}
    )

    if( LIBOKTETA1KASTEN1_INCLUDE_DIR AND
        LIBOKTETA1KASTEN1CORE_LIBRARY AND LIBOKTETA1KASTEN1GUI_LIBRARY AND LIBOKTETA1KASTEN1CONTROLLERS_LIBRARY )
        set( LIBOKTETAKASTEN_FOUND  TRUE )
    endif( LIBOKTETA1KASTEN1_INCLUDE_DIR AND
        LIBOKTETA1KASTEN1CORE_LIBRARY AND LIBOKTETA1KASTEN1GUI_LIBRARY AND LIBOKTETA1KASTEN1CONTROLLERS_LIBRARY )

    if( LIBOKTETAKASTEN_FOUND )
        set( LIBOKTETAKASTEN_VERSION 1 )
        set( LIBOKTETAKASTEN_NEEDS_KASTEN_VERSION 1 )
        set( LIBOKTETAKASTEN_NEEDS_OKTETA_VERSION 1 )
        set( LIBOKTETAKASTEN_LIBRARIES
            ${LIBOKTETA1KASTEN1CORE_LIBRARY}
            ${LIBOKTETA1KASTEN1GUI_LIBRARY}
            ${LIBOKTETA1KASTEN1CONTROLLERS_LIBRARY}
        )
        set( LIBOKTETAKASTEN_INCLUDE_DIRS
            ${LIBOKTETA1KASTEN1_INCLUDE_DIR}
        )
    endif( LIBOKTETAKASTEN_FOUND )
endif( NOT LIBOKTETAKASTEN_FOUND )

# Then search version 0
if( NOT LIBOKTETAKASTEN_FOUND )
    find_library( LIBOKTETAKASTENCORE0_LIBRARY
        NAMES
        oktetakastencore
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )

    find_library( LIBOKTETAKASTENGUI0_LIBRARY
        NAMES
        oktetakastengui
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )

    find_library( LIBOKTETAKASTENCONTROLLERS0_LIBRARY
        NAMES
        oktetakastencontrollers
        HINTS
        ${LIB_INSTALL_DIR}
        ${KDE4_LIB_DIR}
    )


    find_path( LIBOKTETAKASTEN0_INCLUDE_DIR
        NAMES
        bytearraydocument.h
        PATH_SUFFIXES
        kasten
        HINTS
        ${INCLUDE_INSTALL_DIR}
        ${KDE4_INCLUDE_DIR}
    )

    if( LIBOKTETAKASTEN0_INCLUDE_DIR AND
        LIBOKTETAKASTENCORE0_LIBRARY AND LIBOKTETAKASTENGUI0_LIBRARY AND LIBOKTETAKASTENCONTROLLERS0_LIBRARY )
        set( LIBOKTETAKASTEN_FOUND  TRUE )
    endif( LIBOKTETAKASTEN0_INCLUDE_DIR AND
        LIBOKTETAKASTENCORE0_LIBRARY AND LIBOKTETAKASTENGUI0_LIBRARY AND LIBOKTETAKASTENCONTROLLERS0_LIBRARY )

    if( LIBOKTETAKASTEN_FOUND )
        set( LIBOKTETAKASTEN_VERSION 0 )
        set( LIBOKTETAKASTEN_NEEDS_KASTEN_VERSION 0 )
        set( LIBOKTETAKASTEN_NEEDS_OKTETA_VERSION 0 )
        set( LIBOKTETAKASTEN_INCLUDE_DIRS
            ${LIBOKTETAKASTEN0_INCLUDE_DIR}
        )
        set( LIBOKTETAKASTEN_LIBRARIES
            ${LIBOKTETAKASTENCORE0_LIBRARY}
            ${LIBOKTETAKASTENGUI0_LIBRARY}
            ${LIBOKTETAKASTENCONTROLLERS0_LIBRARY}
        )
    endif( LIBOKTETAKASTEN_FOUND )
endif( NOT LIBOKTETAKASTEN_FOUND )


if( LIBOKTETAKASTEN_FOUND )
    if( NOT OktetaKasten_FIND_QUIETLY )
        message( STATUS "Found Okteta Kasten libraries v${LIBOKTETAKASTEN_VERSION}: ${LIBOKTETAKASTEN_LIBRARIES}" )
    endif( NOT OktetaKasten_FIND_QUIETLY )
else( LIBOKTETAKASTEN_FOUND )
    if( LibKasten_FIND_REQUIRED )
        message( FATAL_ERROR "Could not find Okteta Kasten libraries" )
    endif( LibKasten_FIND_REQUIRED )
endif( LIBOKTETAKASTEN_FOUND )

mark_as_advanced(
    LIBOKTETAKASTEN_INCLUDE_DIRS
    LIBOKTETAKASTEN_LIBRARIES
    LIBOKTETAKASTEN_VERSION
    LIBOKTETAKASTEN_NEEDS_KASTEN_VERSION
    LIBOKTETAKASTEN_NEEDS_OKTETA_VERSION
)

## Authors: Marcel Breyer
## Copyright (C): 2024-today All Rights Reserved
## License: This file is released under the MIT license.
##          See the LICENSE.md file in the project root for full license information.
########################################################################################################################

########################################################################################################################
##                                     setup documentation generation with doxygen                                    ##
########################################################################################################################
## use installed doxygen
find_package(Doxygen REQUIRED OPTIONAL_COMPONENTS dot)

## configure doxygen
set(DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/docs")
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/README.md")
set(DOXYGEN_FILE_PATTERNS "*.hpp;")
set(DOXYGEN_STRIP_FROM_PATH "${PROJECT_SOURCE_DIR}")
set(DOXYGEN_ABBREVIATE_BRIEF "")
set(DOXYGEN_QUIET "YES")
set(DOXYGEN_HTML_TIMESTAMP "YES")
set(DOXYGEN_NUM_PROC_THREADS 0)
set(DOXYGEN_WARN_NO_PARAMDOC "YES")
set(DOXYGEN_SORT_MEMBER_DOCS "NO")
set(DOXYGEN_INLINE_INHERITED_MEMB "YES")
set(DOXYGEN_USE_MATHJAX "YES")
set(DOXYGEN_EXCLUDE_SYMBOLS "*_HPP_")

set(DOXYGEN_DOT_IMAGE_FORMAT "svg")
set(DOXYGEN_INTERACTIVE_SVG "YES")
set(DOXYGEN_INCLUDE_GRAPH "NO")
set(DOXYGEN_EXTRACT_PRIVATE "YES")

## enable processing of specific attributes and macros
set(DOXYGEN_ENABLE_PREPROCESSING "YES")
set(DOXYGEN_MACRO_EXPANSION "YES")
set(DOXYGEN_EXPAND_ONLY_PREDEF "YES")
set(DOXYGEN_EXPAND_AS_DEFINED "YES")

set(DOXYGEN_VERBATIM_VARS DOXYGEN_ALIASES)
set(DOXYGEN_ALIASES
        [[license="\par License^^\parblock^^"  ]]
)

## add doxygen as target
doxygen_add_docs(
        doc
        "${PROJECT_SOURCE_DIR}/include;${PROJECT_SOURCE_DIR}/README.md;"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Generating API documentation with Doxygen."
)

## install targets for the documentation
include(GNUInstallDirs)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/docs/html"
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
)

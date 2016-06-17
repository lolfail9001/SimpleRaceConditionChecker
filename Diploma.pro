CONFIG+=rtti_off c++11

CONFIG+=debug

equals(PREFIX, "") {
    PREFIX = /
}
BIN_DIR = $${PREFIX}/bin
BIN_DIR ~= s:/+:/
LIBEXEC_DIR = $${PREFIX}/libexec
LIBEXEC_DIR ~= s:/+:/

CLANG_DIR = /usr

QMAKE_CXX = clang++
QMAKE_LINK = clang++
QMAKE_CXXFLAGS_DEBUG += -O0 -ggdb
QMAKE_CXXFLAGS_RELEASE += -O2 -DNDEBUG -DOPTIMIZE

OBJECTS_DIR = obj
MOC_DIR = moc
UI_DIR = includes
BIN_DIR = bin

include(./clang-check.pri)

INCLUDEPATH += $${CLANG_DIR}/include
unix: QMAKE_CXXFLAGS += -fPIC -pthread -std=c++11
QMAKE_CXXFLAGS += -fvisibility-inlines-hidden -fno-rtti -fno-exceptions
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter -Wno-strict-aliasing
win32: QMAKE_CXXFLAGS_WARN_ON += -Wno-enum-compare
DEFINES += _GNU_SOURCE __STDC_CONSTANT_MACROS __STDC_FORMAT_MACROS __STDC_LIMIT_MACROS

unix: LIBS += -L/usr/lib64

LIBS += -L$${CLANG_DIR}/lib
for(CLANG_LIB, CLANG_LIBS) {
    LIBS += -l$${CLANG_LIB}
}

linux-*: LIBS += -ldl
unix: LIBS += -lz -lncurses
win32: LIBS += -lpsapi -limagehlp -lpthread


HEADERS += \
includes/diplomawindow.hpp \
includes/parser.hpp \
includes/commons.hpp \
includes/analytics.hpp \
includes/analyticsmode.hpp \
includes/reftablemode.hpp

SOURCES += \
src/diplomawindow.cpp \
src/diplomapp.cpp \
src/parser.cpp \
src/analytics.cpp

FORMS += \
forms/diplomawindow.ui



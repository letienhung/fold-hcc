TEMPLATE = subdirs

SUBDIRS += \
    FoldPlugin \
    FoldLib \
    GeometryLib \
    UtilityLib \
    StructureLib \
    MeshUtilityLib \
    CliquerLib

FoldPlugin.depends = UtilityLib GeometryLib

win32:QMAKE_CXXFLAGS += /wd4018

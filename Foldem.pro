TEMPLATE = subdirs

SUBDIRS += \
    FoldPlugin \
    FoldLib \
    GeometryLib \
    UtilityLib \
    StructureLib \
    MeshUtilityLib \
    CliquerLib \
    DemoApp

FoldPlugin.depends = UtilityLib GeometryLib FoldLib
DemoApp.depends = UtilityLib GeometryLib FoldLib

win32:QMAKE_CXXFLAGS += /wd4018

cmake_minimum_required(VERSION 3.16)
project(FarmaciaApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Svg)

add_executable(FarmaciaApp
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/mainwindow.ui
    src/database/dbconfig.h
    src/database/databasemanager.h
    src/database/databasemanager.cpp
    src/database/sesionactual.h
    src/database/sesionactual.cpp
    src/util/passwordutil.h
    src/util/passwordutil.cpp
    src/util/iconos.h
    src/pages/iniciopage.h
    src/pages/iniciopage.cpp
    src/pages/iniciopage.ui
    src/pages/productospage.h
    src/pages/productospage.cpp
    src/pages/productospage.ui
    src/pages/clientespage.h
    src/pages/clientespage.cpp
    src/pages/clientespage.ui
    src/pages/puntoventapage.h
    src/pages/puntoventapage.cpp
    src/pages/puntoventapage.ui
    src/pages/sucursalespage.h
    src/pages/sucursalespage.cpp
    src/pages/sucursalespage.ui
    src/pages/categoriaspage.h
    src/pages/categoriaspage.cpp
    src/pages/categoriaspage.ui
    src/pages/proveedorespage.h
    src/pages/proveedorespage.cpp
    src/pages/proveedorespage.ui
    src/pages/empleadospage.h
    src/pages/empleadospage.cpp
    src/pages/empleadospage.ui
    src/pages/lotespage.h
    src/pages/lotespage.cpp
    src/pages/lotespage.ui
    src/pages/ticketspage.h
    src/pages/ticketspage.cpp
    src/pages/ticketspage.ui
    src/pages/usuariospage.h
    src/pages/usuariospage.cpp
    src/pages/usuariospage.ui
    src/dialogs/productodialog.h
    src/dialogs/productodialog.cpp
    src/dialogs/productodialog.ui
    src/dialogs/clientedialog.h
    src/dialogs/clientedialog.cpp
    src/dialogs/clientedialog.ui
    src/dialogs/sucursaldialog.h
    src/dialogs/sucursaldialog.cpp
    src/dialogs/sucursaldialog.ui
    src/dialogs/categoriadialog.h
    src/dialogs/categoriadialog.cpp
    src/dialogs/categoriadialog.ui
    src/dialogs/proveedordialog.h
    src/dialogs/proveedordialog.cpp
    src/dialogs/proveedordialog.ui
    src/dialogs/empleadodialog.h
    src/dialogs/empleadodialog.cpp
    src/dialogs/empleadodialog.ui
    src/dialogs/lotedialog.h
    src/dialogs/lotedialog.cpp
    src/dialogs/lotedialog.ui
    src/dialogs/devoluciondialog.h
    src/dialogs/devoluciondialog.cpp
    src/dialogs/devoluciondialog.ui
    src/dialogs/ticketdetalledialog.h
    src/dialogs/ticketdetalledialog.cpp
    src/dialogs/ticketdetalledialog.ui
    src/dialogs/logindialog.h
    src/dialogs/logindialog.cpp
    src/dialogs/logindialog.ui
    src/dialogs/registrodialog.h
    src/dialogs/registrodialog.cpp
    src/dialogs/registrodialog.ui
    src/dialogs/usuariodialog.h
    src/dialogs/usuariodialog.cpp
    src/dialogs/usuariodialog.ui
    resources/resources.qrc
)

target_include_directories(FarmaciaApp PRIVATE src)
target_link_libraries(FarmaciaApp PRIVATE Qt6::Widgets Qt6::Sql Qt6::Svg)

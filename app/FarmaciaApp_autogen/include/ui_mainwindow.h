/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayoutRaiz;
    QWidget *barraTitulo;
    QHBoxLayout *barraTituloLayout;
    QLabel *labelTituloApp;
    QSpacerItem *spacerTitulo;
    QPushButton *btnMinimizar;
    QPushButton *btnCerrarApp;
    QHBoxLayout *horizontalLayout;
    QListWidget *listMenu;
    QStackedWidget *stackedWidget;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(900, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayoutRaiz = new QVBoxLayout(centralwidget);
        verticalLayoutRaiz->setSpacing(0);
        verticalLayoutRaiz->setObjectName("verticalLayoutRaiz");
        verticalLayoutRaiz->setContentsMargins(0, 0, 0, 0);
        barraTitulo = new QWidget(centralwidget);
        barraTitulo->setObjectName("barraTitulo");
        barraTitulo->setMinimumHeight(36);
        barraTitulo->setMaximumHeight(36);
        barraTituloLayout = new QHBoxLayout(barraTitulo);
        barraTituloLayout->setSpacing(0);
        barraTituloLayout->setObjectName("barraTituloLayout");
        barraTituloLayout->setContentsMargins(10, 0, 0, 0);
        labelTituloApp = new QLabel(barraTitulo);
        labelTituloApp->setObjectName("labelTituloApp");

        barraTituloLayout->addWidget(labelTituloApp);

        spacerTitulo = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        barraTituloLayout->addItem(spacerTitulo);

        btnMinimizar = new QPushButton(barraTitulo);
        btnMinimizar->setObjectName("btnMinimizar");
        btnMinimizar->setMinimumSize(QSize(46, 36));
        btnMinimizar->setMaximumSize(QSize(46, 36));
        btnMinimizar->setFlat(true);

        barraTituloLayout->addWidget(btnMinimizar);

        btnCerrarApp = new QPushButton(barraTitulo);
        btnCerrarApp->setObjectName("btnCerrarApp");
        btnCerrarApp->setMinimumSize(QSize(46, 36));
        btnCerrarApp->setMaximumSize(QSize(46, 36));
        btnCerrarApp->setFlat(true);

        barraTituloLayout->addWidget(btnCerrarApp);


        verticalLayoutRaiz->addWidget(barraTitulo);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        listMenu = new QListWidget(centralwidget);
        listMenu->setObjectName("listMenu");
        listMenu->setMinimumWidth(225);
        listMenu->setMaximumWidth(225);
        listMenu->setFrameShape(QFrame::NoFrame);
        listMenu->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        horizontalLayout->addWidget(listMenu);

        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName("stackedWidget");

        horizontalLayout->addWidget(stackedWidget);


        verticalLayoutRaiz->addLayout(horizontalLayout);

        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Farmacia", nullptr));
        labelTituloApp->setText(QCoreApplication::translate("MainWindow", "Farmacia", nullptr));
        btnMinimizar->setText(QString());
        btnCerrarApp->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

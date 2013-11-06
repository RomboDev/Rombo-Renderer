/********************************************************************************
** Form generated from reading UI file 'rombo.ui'
**
** Created
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ROMBO_H
#define UI_ROMBO_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RomboClass
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *RomboClass)
    {
        if (RomboClass->objectName().isEmpty())
            RomboClass->setObjectName(QString::fromUtf8("RomboClass"));
        RomboClass->resize(800, 600);
        centralwidget = new QWidget(RomboClass);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        RomboClass->setCentralWidget(centralwidget);
        menubar = new QMenuBar(RomboClass);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 21));
        RomboClass->setMenuBar(menubar);
        statusbar = new QStatusBar(RomboClass);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        RomboClass->setStatusBar(statusbar);

        retranslateUi(RomboClass);

        QMetaObject::connectSlotsByName(RomboClass);
    } // setupUi

    void retranslateUi(QMainWindow *RomboClass)
    {
        RomboClass->setWindowTitle(QApplication::translate("RomboClass", "MainWindow", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class RomboClass: public Ui_RomboClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ROMBO_H

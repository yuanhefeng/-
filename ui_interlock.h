/********************************************************************************
** Form generated from reading UI file 'interlock.ui'
**
** Created by: Qt User Interface Compiler version 5.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INTERLOCK_H
#define UI_INTERLOCK_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InterLock
{
public:
    QPushButton *pushButton;
    QPushButton *pushButton_2;

    void setupUi(QWidget *InterLock)
    {
        if (InterLock->objectName().isEmpty())
            InterLock->setObjectName(QStringLiteral("InterLock"));
        InterLock->resize(400, 300);
        QIcon icon;
        icon.addFile(QStringLiteral("app.ico"), QSize(), QIcon::Normal, QIcon::Off);
        InterLock->setWindowIcon(icon);
        pushButton = new QPushButton(InterLock);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(60, 130, 75, 23));
        pushButton_2 = new QPushButton(InterLock);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(240, 130, 75, 23));

        retranslateUi(InterLock);

        QMetaObject::connectSlotsByName(InterLock);
    } // setupUi

    void retranslateUi(QWidget *InterLock)
    {
        InterLock->setWindowTitle(QApplication::translate("InterLock", "InterLock", Q_NULLPTR));
        pushButton->setText(QApplication::translate("InterLock", "\347\273\247\347\224\265\345\231\250\345\220\270\350\265\267", Q_NULLPTR));
        pushButton_2->setText(QApplication::translate("InterLock", "\347\273\247\347\224\265\345\231\250\350\220\275\344\270\213", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class InterLock: public Ui_InterLock {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INTERLOCK_H

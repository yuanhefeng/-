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
        pushButton->setText(QApplication::translate("InterLock", "\350\256\276\347\275\256\350\277\233\350\267\257", Q_NULLPTR));
        pushButton_2->setText(QApplication::translate("InterLock", "\345\217\226\346\266\210\350\277\233\350\267\257", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class InterLock: public Ui_InterLock {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INTERLOCK_H

/********************************************************************************
** Form generated from reading UI file 'MainDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINDIALOG_H
#define UI_MAINDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainDialog
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QPushButton *btnImage0;
    QPushButton *btnImage1;
    QPushButton *btnImage2;
    QPushButton *btnImage3;
    QPushButton *btnImage4;
    QPushButton *btnImage5;
    QPushButton *btnImage6;
    QPushButton *btnImage7;
    QPushButton *btnImage8;
    QPushButton *btnImage9;
    QPushButton *btnClose;
    QCheckBox *chkSyncWindows;

    void setupUi(QDialog *MainDialog)
    {
        if (MainDialog->objectName().isEmpty())
            MainDialog->setObjectName(QStringLiteral("MainDialog"));
        MainDialog->resize(780, 400);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainDialog->sizePolicy().hasHeightForWidth());
        MainDialog->setSizePolicy(sizePolicy);
        MainDialog->setMinimumSize(QSize(780, 400));
        MainDialog->setMaximumSize(QSize(780, 400));
        MainDialog->setContextMenuPolicy(Qt::PreventContextMenu);
        QIcon icon;
        icon.addFile(QStringLiteral("loupe.ico"), QSize(), QIcon::Normal, QIcon::Off);
        MainDialog->setWindowIcon(icon);
#ifndef QT_NO_WHATSTHIS
        MainDialog->setWhatsThis(QStringLiteral(""));
#endif // QT_NO_WHATSTHIS
        MainDialog->setModal(false);
        verticalLayoutWidget = new QWidget(MainDialog);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(10, 10, 761, 342));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        btnImage0 = new QPushButton(verticalLayoutWidget);
        btnImage0->setObjectName(QStringLiteral("btnImage0"));

        verticalLayout->addWidget(btnImage0);

        btnImage1 = new QPushButton(verticalLayoutWidget);
        btnImage1->setObjectName(QStringLiteral("btnImage1"));

        verticalLayout->addWidget(btnImage1);

        btnImage2 = new QPushButton(verticalLayoutWidget);
        btnImage2->setObjectName(QStringLiteral("btnImage2"));

        verticalLayout->addWidget(btnImage2);

        btnImage3 = new QPushButton(verticalLayoutWidget);
        btnImage3->setObjectName(QStringLiteral("btnImage3"));

        verticalLayout->addWidget(btnImage3);

        btnImage4 = new QPushButton(verticalLayoutWidget);
        btnImage4->setObjectName(QStringLiteral("btnImage4"));

        verticalLayout->addWidget(btnImage4);

        btnImage5 = new QPushButton(verticalLayoutWidget);
        btnImage5->setObjectName(QStringLiteral("btnImage5"));

        verticalLayout->addWidget(btnImage5);

        btnImage6 = new QPushButton(verticalLayoutWidget);
        btnImage6->setObjectName(QStringLiteral("btnImage6"));

        verticalLayout->addWidget(btnImage6);

        btnImage7 = new QPushButton(verticalLayoutWidget);
        btnImage7->setObjectName(QStringLiteral("btnImage7"));

        verticalLayout->addWidget(btnImage7);

        btnImage8 = new QPushButton(verticalLayoutWidget);
        btnImage8->setObjectName(QStringLiteral("btnImage8"));

        verticalLayout->addWidget(btnImage8);

        btnImage9 = new QPushButton(verticalLayoutWidget);
        btnImage9->setObjectName(QStringLiteral("btnImage9"));

        verticalLayout->addWidget(btnImage9);

        btnClose = new QPushButton(MainDialog);
        btnClose->setObjectName(QStringLiteral("btnClose"));
        btnClose->setGeometry(QRect(660, 360, 111, 31));
        chkSyncWindows = new QCheckBox(MainDialog);
        chkSyncWindows->setObjectName(QStringLiteral("chkSyncWindows"));
        chkSyncWindows->setGeometry(QRect(10, 370, 300, 17));

        retranslateUi(MainDialog);

        QMetaObject::connectSlotsByName(MainDialog);
    } // setupUi

    void retranslateUi(QDialog *MainDialog)
    {
        MainDialog->setWindowTitle(QApplication::translate("MainDialog", "pxlpeep", 0));
        btnImage0->setText(QApplication::translate("MainDialog", "Open image...", 0));
        btnImage1->setText(QApplication::translate("MainDialog", "Open image...", 0));
        btnImage2->setText(QApplication::translate("MainDialog", "Open image...", 0));
        btnImage3->setText(QApplication::translate("MainDialog", "Open image...", 0));
        btnImage4->setText(QApplication::translate("MainDialog", "Open image...", 0));
        btnImage5->setText(QApplication::translate("MainDialog", "Open image...", 0));
        btnImage6->setText(QApplication::translate("MainDialog", "Open image...", 0));
        btnImage7->setText(QApplication::translate("MainDialog", "Open image...", 0));
        btnImage8->setText(QApplication::translate("MainDialog", "Open image...", 0));
        btnImage9->setText(QApplication::translate("MainDialog", "Open image...", 0));
        btnClose->setText(QApplication::translate("MainDialog", "Exit", 0));
        chkSyncWindows->setText(QApplication::translate("MainDialog", "Synchronize windows", 0));
    } // retranslateUi

};

namespace Ui {
    class MainDialog: public Ui_MainDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINDIALOG_H

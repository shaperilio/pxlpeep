/********************************************************************************
** Form generated from reading UI file 'MainDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.12.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINDIALOG_H
#define UI_MAINDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
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
            MainDialog->setObjectName(QString::fromUtf8("MainDialog"));
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
        icon.addFile(QString::fromUtf8("loupe.ico"), QSize(), QIcon::Normal, QIcon::Off);
        MainDialog->setWindowIcon(icon);
#ifndef QT_NO_WHATSTHIS
        MainDialog->setWhatsThis(QString::fromUtf8(""));
#endif // QT_NO_WHATSTHIS
        MainDialog->setModal(false);
        verticalLayoutWidget = new QWidget(MainDialog);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(10, 10, 761, 362));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        btnImage0 = new QPushButton(verticalLayoutWidget);
        btnImage0->setObjectName(QString::fromUtf8("btnImage0"));

        verticalLayout->addWidget(btnImage0);

        btnImage1 = new QPushButton(verticalLayoutWidget);
        btnImage1->setObjectName(QString::fromUtf8("btnImage1"));

        verticalLayout->addWidget(btnImage1);

        btnImage2 = new QPushButton(verticalLayoutWidget);
        btnImage2->setObjectName(QString::fromUtf8("btnImage2"));

        verticalLayout->addWidget(btnImage2);

        btnImage3 = new QPushButton(verticalLayoutWidget);
        btnImage3->setObjectName(QString::fromUtf8("btnImage3"));

        verticalLayout->addWidget(btnImage3);

        btnImage4 = new QPushButton(verticalLayoutWidget);
        btnImage4->setObjectName(QString::fromUtf8("btnImage4"));

        verticalLayout->addWidget(btnImage4);

        btnImage5 = new QPushButton(verticalLayoutWidget);
        btnImage5->setObjectName(QString::fromUtf8("btnImage5"));

        verticalLayout->addWidget(btnImage5);

        btnImage6 = new QPushButton(verticalLayoutWidget);
        btnImage6->setObjectName(QString::fromUtf8("btnImage6"));

        verticalLayout->addWidget(btnImage6);

        btnImage7 = new QPushButton(verticalLayoutWidget);
        btnImage7->setObjectName(QString::fromUtf8("btnImage7"));

        verticalLayout->addWidget(btnImage7);

        btnImage8 = new QPushButton(verticalLayoutWidget);
        btnImage8->setObjectName(QString::fromUtf8("btnImage8"));

        verticalLayout->addWidget(btnImage8);

        btnImage9 = new QPushButton(verticalLayoutWidget);
        btnImage9->setObjectName(QString::fromUtf8("btnImage9"));

        verticalLayout->addWidget(btnImage9);

        btnClose = new QPushButton(MainDialog);
        btnClose->setObjectName(QString::fromUtf8("btnClose"));
        btnClose->setGeometry(QRect(660, 250, 111, 31));
        chkSyncWindows = new QCheckBox(MainDialog);
        chkSyncWindows->setObjectName(QString::fromUtf8("chkSyncWindows"));
        chkSyncWindows->setGeometry(QRect(10, 260, 191, 17));

        retranslateUi(MainDialog);

        QMetaObject::connectSlotsByName(MainDialog);
    } // setupUi

    void retranslateUi(QDialog *MainDialog)
    {
        MainDialog->setWindowTitle(QApplication::translate("MainDialog", "pxlpeep", nullptr));
        btnImage0->setText(QApplication::translate("MainDialog", "Open image...", nullptr));
        btnImage1->setText(QApplication::translate("MainDialog", "Open image...", nullptr));
        btnImage2->setText(QApplication::translate("MainDialog", "Open image...", nullptr));
        btnImage3->setText(QApplication::translate("MainDialog", "Open image...", nullptr));
        btnImage4->setText(QApplication::translate("MainDialog", "Open image...", nullptr));
        btnImage5->setText(QApplication::translate("MainDialog", "Open image...", nullptr));
        btnImage6->setText(QApplication::translate("MainDialog", "Open image...", nullptr));
        btnImage7->setText(QApplication::translate("MainDialog", "Open image...", nullptr));
        btnImage8->setText(QApplication::translate("MainDialog", "Open image...", nullptr));
        btnImage9->setText(QApplication::translate("MainDialog", "Open image...", nullptr));
        btnClose->setText(QApplication::translate("MainDialog", "Exit", nullptr));
        chkSyncWindows->setText(QApplication::translate("MainDialog", "Synchronize windows", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainDialog: public Ui_MainDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINDIALOG_H

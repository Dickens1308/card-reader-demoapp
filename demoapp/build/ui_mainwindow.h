/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QStackedWidget *stackedWidget;
    QWidget *pageScan;
    QVBoxLayout *layoutScan;
    QLabel *labelScan;
    QWidget *pageProcessing;
    QVBoxLayout *layoutProcessing;
    QLabel *labelProcessing;
    QWidget *pageError;
    QVBoxLayout *layoutError;
    QLabel *labelError;
    QWidget *pageSuccess;
    QVBoxLayout *layoutSuccess;
    QLabel *labelSuccess;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(272, 480);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName(QStringLiteral("stackedWidget"));
        pageScan = new QWidget();
        pageScan->setObjectName(QStringLiteral("pageScan"));
        pageScan->setStyleSheet(QStringLiteral("background-color: #2196F3; color: white;"));
        layoutScan = new QVBoxLayout(pageScan);
        layoutScan->setObjectName(QStringLiteral("layoutScan"));
        layoutScan->setContentsMargins(20, 20, 20, 20);
        labelScan = new QLabel(pageScan);
        labelScan->setObjectName(QStringLiteral("labelScan"));
        labelScan->setAlignment(Qt::AlignCenter);
        labelScan->setWordWrap(true);
        QFont font;
        font.setPointSize(18);
        font.setBold(true);
        font.setWeight(75);
        labelScan->setFont(font);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(labelScan->sizePolicy().hasHeightForWidth());
        labelScan->setSizePolicy(sizePolicy);

        layoutScan->addWidget(labelScan);

        stackedWidget->addWidget(pageScan);
        pageProcessing = new QWidget();
        pageProcessing->setObjectName(QStringLiteral("pageProcessing"));
        pageProcessing->setStyleSheet(QStringLiteral("background-color: #FF9800; color: white;"));
        layoutProcessing = new QVBoxLayout(pageProcessing);
        layoutProcessing->setObjectName(QStringLiteral("layoutProcessing"));
        layoutProcessing->setContentsMargins(20, 20, 20, 20);
        labelProcessing = new QLabel(pageProcessing);
        labelProcessing->setObjectName(QStringLiteral("labelProcessing"));
        labelProcessing->setAlignment(Qt::AlignCenter);
        labelProcessing->setWordWrap(true);
        labelProcessing->setFont(font);
        sizePolicy.setHeightForWidth(labelProcessing->sizePolicy().hasHeightForWidth());
        labelProcessing->setSizePolicy(sizePolicy);

        layoutProcessing->addWidget(labelProcessing);

        stackedWidget->addWidget(pageProcessing);
        pageError = new QWidget();
        pageError->setObjectName(QStringLiteral("pageError"));
        pageError->setStyleSheet(QStringLiteral("background-color: #F44336; color: white;"));
        layoutError = new QVBoxLayout(pageError);
        layoutError->setObjectName(QStringLiteral("layoutError"));
        layoutError->setContentsMargins(20, 20, 20, 20);
        labelError = new QLabel(pageError);
        labelError->setObjectName(QStringLiteral("labelError"));
        labelError->setAlignment(Qt::AlignCenter);
        labelError->setWordWrap(true);
        labelError->setFont(font);
        sizePolicy.setHeightForWidth(labelError->sizePolicy().hasHeightForWidth());
        labelError->setSizePolicy(sizePolicy);

        layoutError->addWidget(labelError);

        stackedWidget->addWidget(pageError);
        pageSuccess = new QWidget();
        pageSuccess->setObjectName(QStringLiteral("pageSuccess"));
        pageSuccess->setStyleSheet(QStringLiteral("background-color: #4CAF50; color: white;"));
        layoutSuccess = new QVBoxLayout(pageSuccess);
        layoutSuccess->setObjectName(QStringLiteral("layoutSuccess"));
        layoutSuccess->setContentsMargins(20, 20, 20, 20);
        labelSuccess = new QLabel(pageSuccess);
        labelSuccess->setObjectName(QStringLiteral("labelSuccess"));
        labelSuccess->setAlignment(Qt::AlignCenter);
        labelSuccess->setWordWrap(true);
        labelSuccess->setFont(font);
        sizePolicy.setHeightForWidth(labelSuccess->sizePolicy().hasHeightForWidth());
        labelSuccess->setSizePolicy(sizePolicy);

        layoutSuccess->addWidget(labelSuccess);

        stackedWidget->addWidget(pageSuccess);

        verticalLayout->addWidget(stackedWidget);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Contactless Reader", Q_NULLPTR));
        labelScan->setText(QApplication::translate("MainWindow", "Weka kadi yako hapa", Q_NULLPTR));
        labelProcessing->setText(QApplication::translate("MainWindow", "Inaendelea...", Q_NULLPTR));
        labelError->setText(QApplication::translate("MainWindow", "Kadi sio sahihi!", Q_NULLPTR));
        labelSuccess->setText(QApplication::translate("MainWindow", "Kadi imesomwa vizuri!", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

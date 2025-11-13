#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMap>
#include <QPair>
#include <QMessageBox>
#include <QClipboard>
#include <QCryptographicHash>
#include <QFile>
#include <QDataStream>


QMainWindow *mainWindow;
QTableWidget *passWordTable;
QPushButton *addButton;
QPushButton *copyButton;
QPushButton *removeButton;
QMap<QString, QPair<QString, QString>> passwords;


void setUpUI();
void addPassword();
void copyPassword();
void removePassword();

bool verifyMasterPassword();

void loadPassword();
void savePassword();


int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    setUpUI();
    if(verifyMasterPassword()){
        loadPassword();
        mainWindow->show();

    }
    return app.exec();
}

void setUpUI(){
    mainWindow = new QMainWindow();
    mainWindow->setMinimumSize(600,400);
    mainWindow->setWindowTitle("Password Manager");

    QWidget *centralWidget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    QHBoxLayout *buttonLayout = new QHBoxLayout;

    passWordTable = new QTableWidget(0,3);
    passWordTable->setHorizontalHeaderLabels({"Source", "Username", "Password"});
    passWordTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    passWordTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    passWordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    addButton = new QPushButton("Add Button");
    copyButton = new QPushButton("Copy Button");
    removeButton = new QPushButton("Remove Button");

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(copyButton);
    buttonLayout->addWidget(removeButton);

    QObject::connect(addButton, &QPushButton::clicked, addPassword);
    QObject::connect(copyButton, &QPushButton::clicked, copyPassword);
    QObject::connect(removeButton, &QPushButton::clicked, removePassword);

    QObject::connect(QApplication::instance(), &QCoreApplication::aboutToQuit, savePassword);

    mainLayout->addWidget(passWordTable);
    mainLayout->addLayout(buttonLayout);

    mainWindow->setCentralWidget(centralWidget);
}


void addPassword(){
    QString service = QInputDialog::getText(nullptr, "Add Password", "Service");
    if(service.isEmpty()) return;

    QString username = QInputDialog::getText(nullptr, "Add Password", "Username");
    QString password = QInputDialog::getText(nullptr, "Add Password", "Password");

    passwords[service] = qMakePair(username, password);

    int row = passWordTable->rowCount();
    passWordTable->insertRow(row);
    passWordTable->setItem(row,0,new QTableWidgetItem(service));
    passWordTable ->setItem(row,1,new QTableWidgetItem(username));
    passWordTable->setItem(row,2, new QTableWidgetItem("*********"));

}

void copyPassword(){
    int currentRow = passWordTable->currentRow();
    if(currentRow >-0){
        QString serivce = passWordTable->item(currentRow, 0)->text();
        QString password = passwords[serivce].second;
        QApplication::clipboard()->setText(password);
        QMessageBox::information(nullptr, "Copy Password", "Your password has been copied");
    }

}

void removePassword(){
    int currentRow = passWordTable->currentRow();
    if(currentRow >=0){
        QString service = passWordTable->item(currentRow, 0)->text();
        passwords.remove(service);
        passWordTable->removeRow(currentRow);
        QMessageBox::information(nullptr, "Remove Password","You have deleted the password succesfully");
    }

}


bool verifyMasterPassword(){
    QString input = QInputDialog::getText(nullptr, "Master Password", "Enter Master Password", QLineEdit::Password);
    if(input.isEmpty()) return false;

    QString storedHash = QCryptographicHash::hash(
        QString("12345").toUtf8(),
        QCryptographicHash::Sha256
        ).toHex();

    QString inputHash = QCryptographicHash::hash(
        QString(input).toUtf8(),
        QCryptographicHash::Sha256
                            ).toHex();

    return inputHash == storedHash;

}

void loadPassword(){

    QFile file("passwords.dat");
    if(file.open(QIODevice::WriteOnly)){
        QDataStream in(&file);
        QString data;
        in >> data;

        QStringList entries = data.split("/");

        for(const QString& entry:entries){
            QStringList parts = entry.split("|");
            if(parts.size() == 3){
                passwords[parts[0]] = qMakePair(parts[1], parts[2]);

                int row = passWordTable->rowCount();
                passWordTable->insertRow(row);
                passWordTable->setItem(row,0,new QTableWidgetItem(parts[0]));
                passWordTable->setItem(row,2,new QTableWidgetItem(parts[1]));
                passWordTable->setItem(row,2,new QTableWidgetItem(parts[2]));


            }
        }
        file.close();
    }


}

void savePassword(){

    QString data;

    for(auto it = passwords.begin(); it != passwords.end();it++){
        data = it.key() + "|" + it.value().first + "|" + it.value().second + "\n";
    }

    QFile file("passwords.dat");
    if(file.open(QIODevice::WriteOnly)){
        QDataStream out(&file);
        out << data;
        file.close();
    }
}


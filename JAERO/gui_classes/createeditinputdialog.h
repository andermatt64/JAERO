#ifndef CREATEEDITINPUTDIALOG_H
#define CREATEEDITINPUTDIALOG_H

#include <QDialog>
#include <QTableWidget>

namespace Ui {
class CreateEditInputDialog;
}

class CreateEditInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateEditInputDialog(QWidget *parent = nullptr, bool isEdit = false);
    ~CreateEditInputDialog();

    QString getFormat() const;
    QString getHost() const;
    QString getPort() const;

    void setFormat(const QString &fmt);
    void setHost(const QString &host);
    void setPort(const QString &port);

    static bool addEntry(QWidget *parent, QTableWidget *output)
    {
        CreateEditInputDialog dialog(parent);
        dialog.setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint);

        if (dialog.exec() == QDialog::Accepted)
        {
            int newRow=output->rowCount();
            output->insertRow(newRow);
            output->setItem(newRow,0,new QTableWidgetItem(dialog.getFormat()));
            output->setItem(newRow,1,new QTableWidgetItem(dialog.getHost()));
            output->setItem(newRow,2,new QTableWidgetItem(dialog.getPort()));

            return true;
        }

        return false;
    }

    static bool editEntry(QWidget *parent, QTableWidget *output, int row)
    {
        CreateEditInputDialog dialog(parent,true);
        dialog.setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint);
        dialog.setFormat(output->item(row,0)->text());
        dialog.setHost(output->item(row,1)->text());
        dialog.setPort(output->item(row,2)->text());

        if (dialog.exec()==QDialog::Accepted)
        {
            output->setItem(row,0,new QTableWidgetItem(dialog.getFormat()));
            output->setItem(row,1,new QTableWidgetItem(dialog.getHost()));
            output->setItem(row,2,new QTableWidgetItem(dialog.getPort()));

            return true;
        }

        return false;
    }


private slots:
    void on_lineEditfeederhost_cursorPositionChanged(int,int);
    void on_lineEditfeederport_cursorPositionChanged(int,int);

private:
    Ui::CreateEditInputDialog *ui;

    void checkHostAndPort();
};

#endif // CREATEEDITINPUTDIALOG_H

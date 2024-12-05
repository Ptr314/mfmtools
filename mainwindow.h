/*
 * English:
 *
 *       This program is free software: you can redistribute it and/or modify
 *       it under the terms of the GNU General Public License as published by
 *       the Free Software Foundation, either version 3 of the License, or
 *       (at your option) any later version.
 *
 *       This program is distributed in the hope that it will be useful,
 *       but WITHOUT ANY WARRANTY; without even the implied warranty of
 *       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *       GNU General Public License for more details.
 *
 *       You should have received a copy of the GNU General Public License
 *       along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *       Author: Panther <http://www.emuverse.ru/wiki/User:Panther>
 *
 * Russian:
 *       Этот файл является свободным программным обеспечением, вы можете
 *       распространять и изменять его на условиях лицензии GNU General Public
 *       License, опубликованной Free Software Foundation, версии 3, или
 *       более поздней, на ваше усмотрение.
 *
 *       Программа распространяется с надеждой, что она окажется полезной,
 *       но БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ, в том числе подразумеваемых гарантий
 *       КОММЕРЧЕСКОЙ ЦЕННОСТИ или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ.
 *       Подробнее смотрите текст лицензии GNU General Public License.
 *
 *       Копия текста лицензии должна поставляться вместе с этим файлом,
 *       в противном случае вы можете получить ее по адресу
 *       <http://www.gnu.org/licenses/>
 *
 *       Автор: Panther <http://www.emuverse.ru/wiki/User:Panther>
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QSettings>
#include <QtDebug>
#include <QTranslator>
#include <QToolButton>
#include <QJsonObject>

#include "libs/disk_formats.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent*);
    void resizeEvent(QResizeEvent* event);
    void showEvent(QShowEvent * event);

protected slots:
    void slotLanguageChanged(QAction* action);

private slots:
    void on_leftDirs_clicked(const QModelIndex &index);

    void on_rightDirs_clicked(const QModelIndex &index);

    void on_btn_dir_left_to_right_clicked();

    void on_btn_convert_left_to_right_clicked();

    void on_btn_dir_right_to_left_clicked();

    void on_leftFilesFilter_activated(int index);

    void on_rightFilesFilter_activated(int index);

    void on_toolButton_2_clicked();

    void on_AnalyzeBtn_clicked();

    void on_actionAbout_triggered();

    void on_leftFiles_clicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;

    //Language routines
    QTranslator m_translator;
    QTranslator m_translatorQt;
    QString m_currLang;
    QString m_langPath;
    void loadLanguage(const QString& rLanguage);
    void createLanguageMenu(void);

    //File lists routines
    QFileSystemModel *leftDirsModel, *leftFilesModel, *rightDirsModel, *rightFilesModel;
    QString leftPath, rightPath;
    void init_lists(void);
    void set_right(QString newPath);
    void set_left(QString newPath);

    void init_dropdowns(void);
    void resize_columns(void);


    QSettings *settings;

    QJsonObject custom_fddf;
    void update_custom_disk(void);

    void process_files(bool do_conversion);

    void load_config(void);
    QJsonArray ConvertJsonHex(QJsonArray jsonArray);

};

#endif // MAINWINDOW_H

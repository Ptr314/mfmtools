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

#include <QMessageBox>
#include <QLibraryInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "libs/config.h"
#include "diskparamsdlg.h"
#include "loggerdlg.h"
#include "ui_aboutdlg.h"

#include "converter.h"
#include "libs/mfm_formats.h"
#include "libs/disk_formats.h"
#include "libs/loader.h"
#include "libs/config.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    settings = new QSettings("mfmtools.ini",  QSettings::IniFormat);

    createLanguageMenu();

    load_config();
    init_lists();
    init_dropdowns();
}

MainWindow::~MainWindow()
{
    QModelIndex index;
    QFileInfo fileInfo;

    QModelIndexList list = ui->leftDirs->selectionModel()->selectedIndexes();
    if (list.size() > 0) {
        index = list.first();
        fileInfo = leftDirsModel->fileInfo(index);
        settings->setValue("folders/left", fileInfo.absoluteFilePath());
    }
    list = ui->rightDirs->selectionModel()->selectedIndexes();
    if (list.size() > 0) {
        index = list.first();
        fileInfo = rightDirsModel->fileInfo(index);
        settings->setValue("folders/right", fileInfo.absoluteFilePath());
    }
    settings->setValue("system/language", m_currLang);
    settings->setValue("folders/left_file_filter", ui->leftFilesFilter->currentIndex());
    settings->setValue("formats/left_format_combo", ui->leftFormatCombo->currentIndex());
    settings->setValue("folders/right_file_filter", ui->rightFilesFilter->currentIndex());
    settings->setValue("formats/right_format_combo", ui->rightFormatCombo->currentIndex());
    QJsonDocument doc(this->custom_fddf);
    settings->setValue("formats/custom", doc.toJson());
    settings->sync();
    delete ui;
}

void MainWindow::init_lists(void){
    leftDirsModel = new QFileSystemModel(this);
    leftDirsModel->setReadOnly(true);
    leftDirsModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    leftDirsModel->setRootPath("");

    ui->leftDirs->setModel(leftDirsModel);

    ui->leftDirs->header()->hideSection(1);
    ui->leftDirs->header()->hideSection(2);
    ui->leftDirs->header()->hideSection(3);

    rightDirsModel = new QFileSystemModel(this);
    rightDirsModel->setReadOnly(true);
    rightDirsModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    rightDirsModel->setRootPath("");

    ui->leftDirs->setModel(leftDirsModel);
    ui->rightDirs->setModel(rightDirsModel);

    ui->leftDirs->header()->hideSection(1);
    ui->leftDirs->header()->hideSection(2);
    ui->leftDirs->header()->hideSection(3);

    ui->rightDirs->header()->hideSection(1);
    ui->rightDirs->header()->hideSection(2);
    ui->rightDirs->header()->hideSection(3);

    leftPath = settings->value("folders/left", "").toString();
    rightPath = settings->value("folders/right", "").toString();

    QModelIndex index = leftDirsModel->index(leftPath);
    ui->leftDirs->expand(index);
    ui->leftDirs->setCurrentIndex(index);
    ui->leftDirs->scrollTo(index);
    ui->leftDirs->resizeColumnToContents(0);

    index = rightDirsModel->index(rightPath);
    ui->rightDirs->expand(index);
    ui->rightDirs->scrollTo(index);
    ui->rightDirs->setCurrentIndex(index);
    ui->rightDirs->resizeColumnToContents(0);

    leftFilesModel = new QFileSystemModel(this);
    leftFilesModel->setReadOnly(true);
    leftFilesModel->setFilter(QDir::Files | QDir::NoDotAndDotDot);

    rightFilesModel = new QFileSystemModel(this);
    rightFilesModel->setReadOnly(true);
    rightFilesModel->setFilter(QDir::Files | QDir::NoDotAndDotDot);


    ui->leftFiles->setModel(leftFilesModel);
    ui->leftFiles->setRootIndex(leftFilesModel->setRootPath(leftPath));

    ui->rightFiles->setModel(rightFilesModel);
    ui->rightFiles->setRootIndex(rightFilesModel->setRootPath(rightPath));

    ui->leftFiles->header()->hideSection(2);

    ui->rightFiles->header()->hideSection(2);

}

void MainWindow::init_dropdowns(void)
{
    ui->leftFilesFilter->clear();
    foreach (const QJsonValue & value, file_formats) {
        QJsonObject obj = value.toObject();
        ui->leftFilesFilter->addItem(
                                QString("%1 (%2)").arg(obj["type_name"].toString()).arg(obj["type_extensions"].toString().replace(";", "; ")),
                                obj["type_extensions"].toString()
        );
    }
    ui->leftFilesFilter->setCurrentIndex(settings->value("folders/left_file_filter", 1).toInt());
    on_leftFilesFilter_activated(ui->leftFilesFilter->currentIndex());

    QJsonDocument doc = QJsonDocument::fromJson(settings->value("formats/custom").toByteArray());
    this->custom_fddf = doc.object();

    ui->leftFormatCombo->clear();

    foreach (const QJsonValue & value, fdd_formats) {
        QJsonObject obj = value.toObject();
        ui->leftFormatCombo->addItem(obj["name"].toString());
    }
    update_custom_disk();
    ui->leftFormatCombo->setCurrentIndex(settings->value("formats/left_format_combo", 0).toInt());

    ui->rightFilesFilter->clear();
    foreach (const QJsonValue & value, target_formats) {
        QJsonObject obj = value.toObject();
        ui->rightFilesFilter->addItem(
                                QString("%1 (%2)").arg(obj["type_name"].toString()).arg(obj["type_extensions"].toString().replace(";", "; ")),
                                obj["type_extensions"].toString()
        );
    }
    ui->rightFilesFilter->setCurrentIndex(settings->value("folders/right_file_filter", 0).toInt());
    on_rightFilesFilter_activated(ui->rightFilesFilter->currentIndex());

    ui->rightFormatCombo->clear();
    foreach (const QJsonValue & value, fdd_track_descriptions) {
        QJsonObject obj = value.toObject();
        ui->rightFormatCombo->addItem(obj["name"].toString());
    }
    ui->rightFormatCombo->setCurrentIndex(settings->value("formats/right_format_combo", 0).toInt());
}


void MainWindow::createLanguageMenu(void)
{
    QActionGroup* langGroup = new QActionGroup(ui->menuLanguage);
    langGroup->setExclusive(true);

    connect(langGroup, SIGNAL (triggered(QAction *)), this, SLOT (slotLanguageChanged(QAction *)));

    // format systems language
    QString defaultLocale = QLocale::system().name(); // e.g. "de_DE"
    defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "de"

    defaultLocale = settings->value("system/language", defaultLocale).toString();

    m_langPath = QApplication::applicationDirPath();
    m_langPath.append("/languages");
    QDir dir(m_langPath);
    QStringList fileNames = dir.entryList(QStringList("app_*.qm"));

    for (int i = 0; i < fileNames.size(); ++i) {
        // get locale extracted by filename
        QString locale;
        locale = fileNames[i]; // "Example_ru.qm"
        locale.truncate(locale.lastIndexOf('.')); // "Example_ru"
        locale.remove(0, locale.indexOf('_') + 1); // "ru"

         QString lang = QLocale::languageToString(QLocale(locale).language());
         QIcon ico(QString("%1/%2.png").arg(m_langPath).arg(locale));

         QAction *action = new QAction(ico, lang, this);
         action->setCheckable(true);
         action->setData(locale);

         ui->menuLanguage->addAction(action);
         langGroup->addAction(action);

         // set default translators and language checked
         if (defaultLocale == locale) {
            action->setChecked(true);
            ui->menuLanguage->setIcon(action->icon());
         }
    }
    loadLanguage(defaultLocale);
}

void MainWindow::slotLanguageChanged(QAction* action)
{
 if(0 != action) {
  // load the language dependant on the action content
  loadLanguage(action->data().toString());
  ui->menuLanguage->setIcon(action->icon());
  //setWindowIcon(action->icon());
 }
}

void switchTranslator(QTranslator& translator, const QString& filename)
{
 // remove the old translator
 qApp->removeTranslator(&translator);

 // load the new translator
 if(translator.load(filename))
  qApp->installTranslator(&translator);
}

void MainWindow::loadLanguage(const QString& rLanguage)
{
 if(m_currLang != rLanguage) {
  m_currLang = rLanguage;
  QLocale locale = QLocale(m_currLang);
  QLocale::setDefault(locale);
  QString languageName = QLocale::languageToString(locale.language());
  switchTranslator(m_translator, QString("%1/app_%2.qm").arg(m_langPath).arg(rLanguage));
  //switchTranslator(m_translatorQt, QString("%1/qtbase_%2.qm").arg(QLibraryInfo::location(QLibraryInfo::TranslationsPath)).arg(rLanguage));
  switchTranslator(m_translatorQt, QString("%1/qtbase_%2.qm").arg(m_langPath).arg(rLanguage));
 }
}

void MainWindow::changeEvent(QEvent* event)
{
    if(0 != event) {
        switch(event->type()) {
            // this event is send when a translator is loaded
            case QEvent::LanguageChange:
                ui->retranslateUi(this);
            break;

            // this event is send when the system language changes
            case QEvent::LocaleChange: {
                QString locale = QLocale::system().name();
                locale.truncate(locale.lastIndexOf('_'));
                loadLanguage(locale);
            }
            break;

            //This is to omit compiler warnings
            default:
            break;
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::on_leftDirs_clicked(const QModelIndex &index)
{
    if (index.isValid()) {
        leftPath = leftDirsModel->fileInfo(index).absoluteFilePath();
        ui->leftFiles->setRootIndex(leftFilesModel->setRootPath(leftPath));
    }
}

void MainWindow::on_rightDirs_clicked(const QModelIndex &index)
{
    if (index.isValid()) {
        rightPath = rightDirsModel->fileInfo(index).absoluteFilePath();
        ui->rightFiles->setRootIndex(rightFilesModel->setRootPath(rightPath));
    }
}

void MainWindow::set_right(QString newPath)
{
    rightPath = newPath;
    QModelIndex index = rightDirsModel->index(rightPath);
    ui->rightDirs->expand(index);
    ui->rightDirs->scrollTo(index);
    ui->rightDirs->setCurrentIndex(index);
    ui->rightFiles->setRootIndex(rightFilesModel->setRootPath(rightPath));
}

void MainWindow::set_left(QString newPath)
{
    leftPath = newPath;
    QModelIndex index = leftDirsModel->index(leftPath);
    ui->leftDirs->expand(index);
    ui->leftDirs->scrollTo(index);
    ui->leftDirs->setCurrentIndex(index);
    ui->leftFiles->setRootIndex(leftFilesModel->setRootPath(leftPath));
}

void MainWindow::on_btn_dir_left_to_right_clicked()
{
    set_right(leftPath);
}

void MainWindow::on_btn_dir_right_to_left_clicked()
{
    set_left(rightPath);
}

void MainWindow::on_btn_convert_left_to_right_clicked()
{
    process_files(true);
}

void MainWindow::on_leftFilesFilter_activated(int index)
{
    QStringList filter = ui->leftFilesFilter->itemData(index).toString().split(";");
    leftFilesModel->setNameFilters(filter);
    leftFilesModel->setNameFilterDisables(false);
}

void MainWindow::on_rightFilesFilter_activated(int index)
{
    QStringList filter = ui->rightFilesFilter->itemData(index).toString().split(";");
    rightFilesModel->setNameFilters(filter);
    rightFilesModel->setNameFilterDisables(false);
    bool is_mfm = ui->rightFilesFilter->itemText(index).contains("MFM");
    ui->rightFormatCombo->setEnabled(is_mfm);
    ui->rightFormatLabel->setEnabled(is_mfm);
}

void MainWindow::on_toolButton_2_clicked()
{
    QJsonObject fddf;

    DiskParamsDlg dlg(this);
    int selected = ui->leftFormatCombo->currentIndex();
    if (selected != 0) {
        fddf = fdd_formats[selected].toObject();
    } else {
        fddf = this->custom_fddf;
    }
    dlg.setValues(&fddf);

    if (dlg.exec() == DLG_RESULT_CHANGED) {
        this->custom_fddf = fddf;
        this->custom_fddf["id"] = 0;
        this->custom_fddf["name"] = "Custom";
        update_custom_disk();
        ui->leftFormatCombo->setCurrentIndex(0);
    }
}

void MainWindow::update_custom_disk(void)
{
    ui->leftFormatCombo->setItemText(0,
        QString(tr("Custom disk type")).append(
            QString("\n(H:%1 T:%2 S:%3 SS:%4)").arg(QString::number(this->custom_fddf["heads"].toInt())).arg(QString::number(this->custom_fddf["tracks"].toInt())).arg(QString::number(this->custom_fddf["sectors"].toInt())).arg(QString::number(this->custom_fddf["sector"].toInt()))
        )
    );

}

void MainWindow::resizeEvent(QResizeEvent* event)
{
   QMainWindow::resizeEvent(event);
   resize_columns();
}

void MainWindow::showEvent(QShowEvent * event)
{
    QMainWindow::showEvent(event);
    resize_columns();
}

void MainWindow::resize_columns()
{
    int w1 = ui->leftFiles->width();
    ui->leftFiles->header()->resizeSection(3,120);
    ui->leftFiles->header()->resizeSection(1,80);
    ui->leftFiles->header()->resizeSection(0,w1-210);
    int w2 = ui->rightFiles->width();
    ui->rightFiles->header()->resizeSection(3,120);
    ui->rightFiles->header()->resizeSection(1,80);
    ui->rightFiles->header()->resizeSection(0,w2-210);
}

void MainWindow::on_AnalyzeBtn_clicked()
{
    process_files(false);
}

void MainWindow::process_files(bool do_conversion)
{
    QModelIndexList list_left, list_right;
    QModelIndex index_left, index_right;
    QFileInfo fileInfo_left, fileInfo_right;
    QJsonObject fddf;
    QJsonObject inp_ft;
    QJsonObject out_ft;
    QJsonObject td;
    uint8_t err;
    QString log;
    bool continue_process;
    bool abort_process = false;
    bool size_yes_to_all = false;
    bool size_no_to_all = false;

    list_left = ui->leftFiles->selectionModel()->selectedRows();
    list_right = ui->rightDirs->selectionModel()->selectedRows();

    bool processing_multiple = list_left.size() > 1;

    int warning_buttons = QMessageBox::Yes | QMessageBox::No;
    if (processing_multiple) warning_buttons |= QMessageBox::Abort | QMessageBox::YesToAll | QMessageBox::NoToAll;

    if (list_left.size()>0 && list_right.size()>0) {
        foreach(index_left, list_left) {
            //index_left = list_left.first();
            fileInfo_left = leftFilesModel->fileInfo(index_left);

            index_right = list_right.first();
            fileInfo_right = rightDirsModel->fileInfo(index_right);

            if (ui->leftFormatCombo->currentIndex() == 0) {
                fddf = this->custom_fddf;
            } else {
                fddf = fdd_formats[ui->leftFormatCombo->currentIndex()].toObject();
            }
            inp_ft = file_formats[ui->leftFilesFilter->currentIndex()].toObject();
            out_ft = target_formats[ui->rightFilesFilter->currentIndex()].toObject();
            td = fdd_track_descriptions[ui->rightFormatCombo->currentIndex()].toObject();

            err = file_convert(fddf, inp_ft, out_ft, fdd_track_formats, fdd_track_variants, td["id"].toString(), fileInfo_left.absoluteFilePath(), fileInfo_right.absoluteFilePath(), FDD_CONV_CHECK, &log);
            switch (err){
                case FDD_LOAD_ERROR:
                {
                    QMessageBox::critical(0, qApp->translate("Converter", "Error"), qApp->translate("Converter", "Error reading input file"));
                    continue_process = false;
                    break;
                }
                case FDD_LOAD_SIZE_SMALLER:
                case FDD_LOAD_SIZE_LARGER:
                {
                    if (size_yes_to_all) {
                        continue_process = true;
                    } else
                    if (size_no_to_all) {
                        continue_process = false;
                    } else
                    if (do_conversion) {
                        int res;
                        if (err == FDD_LOAD_SIZE_SMALLER)
                            res = QMessageBox::warning(0,
                                         qApp->translate("Converter", "Warning"),
                                         QString(qApp->translate("Converter", "%1: File size is too small. Ignore?")).arg(fileInfo_left.fileName()),
                                         (QMessageBox::StandardButton)warning_buttons
                                  );
                        else
                            res = QMessageBox::warning(0,
                                         qApp->translate("Converter", "Warning"),
                                         QString(qApp->translate("Converter", "%1: File size is too large. Ignore?")).arg(fileInfo_left.fileName()),
                                         (QMessageBox::StandardButton)warning_buttons
                                  ) == QMessageBox::Yes;
                        switch (res) {
                        case QMessageBox::Yes:
                            continue_process = true;
                            break;
                        case QMessageBox::No:
                            continue_process = false;
                            break;
                        case QMessageBox::YesToAll:
                            continue_process = true;
                            size_yes_to_all = true;
                            break;
                        case QMessageBox::NoToAll:
                            continue_process = false;
                            size_no_to_all = true;
                            break;
                        case QMessageBox::Abort:
                            abort_process = true;
                            break;
                        };

                    } else {
                        if (err == FDD_LOAD_SIZE_SMALLER)
                            log.append(qApp->translate("Converter", "Warning: File size is too small."));
                        else
                            log.append(qApp->translate("Converter", "Warning: File size is too large."));
                    }
                    break;
                }
                case FDD_LOAD_PARAMS_MISMATCH:
                {
                    QMessageBox::critical(0, qApp->translate("Converter", "Error"), qApp->translate("Converter", "File parameters mismatch"));
                    continue_process = false;
                    break;
                }
                case FDD_LOAD_FILE_CORRUPT:
                {
                    QMessageBox::critical(0, qApp->translate("Converter", "Error"), qApp->translate("Converter", "File is corrupt"));
                    continue_process = false;
                    break;
                }
                default:
                {
                    continue_process = true;
                    break;
                }
            };
            if (abort_process) break;
            if (continue_process && do_conversion) {
                err = file_convert(fddf, inp_ft, out_ft, fdd_track_formats, fdd_track_variants, td["id"].toString(), fileInfo_left.absoluteFilePath(), fileInfo_right.absoluteFilePath(), FDD_CONV_CONVERT, &log);
                switch (err){
                    case FDD_CONV_UNKNOWN_INPUT:
                    {
                        QMessageBox::critical(0, "Error", "Unknown input file type");
                        break;
                    }
                    case FDD_CONV_READ_ERROR:
                    {
                        QMessageBox::critical(0, qApp->translate("Converter", "Error"), qApp->translate("Converter", "Error reading input file"));
                        break;
                    }
                    case FDD_CONV_UNKNOWN_OUTPUT:
                    {
                        QMessageBox::critical(0, "Error", "Unknown output file type");
                        break;
                    }
                    case FDD_CONV_WRITE_ERROR:
                    {
                        QMessageBox::critical(0, qApp->translate("Converter", "Error"), qApp->translate("Converter", "Error writing output file"));
                        break;
                    }
                    default:
                    break;
                }
            } else {
                if (!processing_multiple) {
                    LoggerDlg dlg(this);
                    dlg.setLogText(&log);
                    dlg.exec();
                }
            }
        }
    } else
        QMessageBox::critical(0, qApp->translate("Converter", "Error"), qApp->translate("Converter", "Source and destination are not selected"));

}


void MainWindow::on_actionAbout_triggered()
{
    QDialog * about = new QDialog(0,0);

    Ui_About aboutUi;
    aboutUi.setupUi(about);

    about->show();
}

void MainWindow::load_config(void)
{
    //
    QFile file(QApplication::applicationDirPath().append("/config.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(0, qApp->translate("Main", "Error"), qApp->translate("Main", "Error reading config file"));
        return;
    }
    QByteArray config_contents = file.readAll();
    file.close();
    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(config_contents, &err);
    if (jsonDoc.isNull()) {
        QMessageBox::critical(0, qApp->translate("Main", "Error"), qApp->translate("Main", "Config parse error: %1").arg(err.errorString()));
        return;
    }
    QJsonObject jsonRoot = jsonDoc.object();
    file_formats = jsonRoot["file_formats"].toArray();
    target_formats = jsonRoot["target_formats"].toArray();
    fdd_formats = jsonRoot["fdd_formats"].toArray();
    fdd_track_variants = jsonRoot["fdd_track_variants"].toArray();
    fdd_track_formats = ConvertJsonHex(jsonRoot["fdd_track_formats"].toArray());
    fdd_track_descriptions = jsonRoot["fdd_track_descriptions"].toArray();
}

QJsonArray MainWindow::ConvertJsonHex(QJsonArray jsonArray)
{
    QJsonArray new_array;
    for (int i=0; i < jsonArray.size(); i++) {
        QJsonObject obj = jsonArray[i].toObject();
        QStringList keys = obj.keys();
        foreach (const QString &key, keys) {
            QString value = obj[key].toString();
            if (value.startsWith("0x")) {
                bool ok;
                obj[key] = value.mid(2).toInt(&ok, 16);
                if (!ok) {
                    QMessageBox::critical(0, qApp->translate("Main", "Error"), qApp->translate("Main", "Error converting config HEX value"));
                    return QJsonArray();
                }
            }
        }
        new_array.append(obj);
    }
    return new_array;
}

void MainWindow::on_leftFiles_clicked(const QModelIndex &index)
{
    ui->AnalyzeBtn->setEnabled(ui->leftFiles->selectionModel()->selectedRows().size() == 1);
}

#include "pdfmanager.h"

PDFmanager::PDFmanager() {}

QString PDFmanager::getStyleSheet()
{
  return
      R"S(
/*
reset css
*/
* {
  -webkit-tap-highlight-color: transparent;
}
*:focus {
  outline: none;
}
html,
body,
div,
span,
applet,
object,
iframe,
h1,
h2,
h3,
h4,
h5,
h6,
p,
blockquote,
pre,
a,
abbr,
acronym,
address,
big,
cite,
code,
del,
dfn,
em,
img,
ins,
kbd,
q,
s,
samp,
small,
strike,
strong,
sub,
sup,
tt,
var,
b,
u,
i,
dl,
dt,
dd,
ol,
ul,
li,
fieldset,
form,
label,
legend,
table,
caption,
tbody,
tfoot,
thead,
tr,
th,
td,
article,
aside,
canvas,
details,
embed,
figure,
figcaption,
footer,
header,
hgroup,
menu,
nav,
output,
ruby,
section,
summary,
time,
mark,
audio,
video {
  margin: 0;
  padding: 0;
  border: 0;
  vertical-align: baseline;
}
article,
aside,
details,
figcaption,
figure,
footer,
header,
hgroup,
menu,
nav,
section {
  display: block;
}
body,
#root {
  line-height: 1;
  overflow-wrap: anywhere;
  overflow-x: hidden;
}
ol,
ul {
  list-style: none;
}
blockquote,
q {
  quotes: none;
}
blockquote:before,
blockquote:after,
q:before,
q:after {
  content: "";
  content: none;
}
table {
  margin: 0;
  border-collapse: collapse;
  border-spacing: 0;
}

table, th, td {
  border: 1px solid;
}

h1, h2, p, td {
  text-align: center;
}

h1 {
  color: #007700;
}
)S";
}

QString PDFmanager::getAppDir()
{
  return QCoreApplication::applicationDirPath();
}

QString PDFmanager::getDesktopDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
}

void PDFmanager::createPDF(QString html, QString title)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QDateTime time = QDateTime::currentDateTime();
  QString appDir = getDesktopDir();
  QDir folder(appDir + "/отчеты");
  if (!folder.exists())
  {
    folder.mkdir(appDir + "/отчеты");
  }

  QString fileName = title + " " + time.toString("dd.MM.yyyy HH-mm-ss") + ".pdf";
  fileName.replace(" ", "_");

  QString filePath = appDir + "/отчеты/" + fileName;

  QPrinter printer(QPrinter::PrinterResolution);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setPageSize(QPageSize::A4);
  printer.setOutputFileName(filePath);

  qDebug() << printer.outputFileName();

  QTextDocument doc;

  doc.setDefaultStyleSheet(getStyleSheet());
  doc.setHtml(getHeader(time) + html + getFooter(time));
  doc.setPageSize(printer.pageRect(QPrinter::DevicePixel).size());

  doc.print(&printer);

  QMimeData *mimeData = new QMimeData();
  mimeData->setUrls({QUrl::fromLocalFile(filePath)});

  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setMimeData(mimeData);
  
  QApplication::restoreOverrideCursor();

  QMessageBox popup;

  popup.setTextFormat(Qt::MarkdownText);
  popup.setText("Отчет сохранен в папке отчеты на рабочем столе и скопирован в буфер обмена");

  popup.exec();
}

QString PDFmanager::getHeader(QDateTime time)
{
  return "<p>" + time.toString("dd.MM.yyyy HH:mm:ss") + "</p><h1 width=100% color='#007700'>ECO TAXI</h1>";
}

QString PDFmanager::getFooter(QDateTime time)
{
  return "<br><p>ECO TAXI</p><p>" + time.toString("dd.MM.yyyy HH:mm:ss") + "</p>";
}

void PDFmanager::ToPDF(QString title, QString dates, QList<QAbstractItemModel *> models, int start)
{
  QString html = "<h2>" + title + "</h2>";
  html += "<p>" + dates + "</p><br>";

  for (int i = 0; i < models.size(); i++)
  {
    html += modelToHTML(models[i], start != 0 && i == 0 ? 1 : 0);
  }

  createPDF(html, title + " " + dates);
}

QString PDFmanager::modelToHTML(QAbstractItemModel *model, int start)
{
    QString html;
    
    // Remove margin, and set table width to 100%
    html += "<table style='margin: 0;' margin=0 width=100%><tr>";
    
    // Add row number column if start == 1
    if (start == 1)
    {
        html += "<th>#</th>";
    }
    
    // Add headers
    for (int i = start; i < model->columnCount(); i++)
    {
        html += "<th>" + model->headerData(i, Qt::Horizontal).toString() + "</th>";
    }
    html += "</tr>";
    
    // Add rows
    for (int i = 0; i < model->rowCount(); i++)
    {
        html += "<tr>";
        if (start == 1)
        {
            html += "<td>" + QString::number(i + 1) + "</td>";
        }
        
        for (int j = start; j < model->columnCount(); j++)
        {
            QString cellData = model->index(i, j).data(Qt::DisplayRole).toString();
            QString header = model->headerData(j, Qt::Horizontal).toString();
            
            // Check if the header is "Инвестору" to apply green color
            if (header == "Инвестору" && start != 1)
            {
                html += "<td style='border: 1px solid black; color:#007700;'>" + cellData + "</td>";
            }
            else
            {
                html += "<td>" + cellData + "</td>";
            }
        }
        html += "</tr>";
    }
    
    html += "</table>";
    return html;
}



void PDFmanager::exportToPDF(QString title, QString dates, QList<QAbstractItemModel *> models, int start)
{
    ColumnSelectionDialog dialog(models, title, dates, start);
    
    dialog.exec();
}


void PDFmanager::exportCarReportByDays(const QMap<QDate, QAbstractItemModel*> &modelsByDay,
                                       const QString &title, const QString &dates)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString appDir = getDesktopDir();
    QDir folder(appDir + "/отчеты");
    if (!folder.exists()) {
        folder.mkdir(appDir + "/отчеты");
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    QString fileName = title + " " + currentTime.toString("dd.MM.yyyy HH-mm-ss") + ".pdf";
    fileName.replace(" ", "_");
    QString filePath = appDir + "/отчеты/" + fileName;

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize::A4);
    printer.setOutputFileName(filePath);

    // Формируем общий заголовок (будет отображен только один раз в начале)
    QString overallHeader = getHeader(currentTime);
    QString overallFooter = getFooter(currentTime);

    // Собираем HTML для каждого дня.
    QString combinedHtml;
    bool firstDay = true;
    // modelsByDay предполагается, что отсортирован по возрастанию даты
    for (auto it = modelsByDay.constBegin(); it != modelsByDay.constEnd(); ++it) {
        QDate day = it.key();
        QAbstractItemModel *model = it.value();

        // Формируем контент для данного дня: дата (без дополнительного заголовка) + таблица.
        QString dayContent;
        dayContent += "<p style='text-align: center; font-weight: bold; font-size: 14pt; margin-bottom: 10px;'>"
                      + day.toString("dd.MM.yyyy")
                      + "</p>";
        dayContent += modelToHTML(model, 0);

        // Для первого дня не добавляем page-break-before, для остальных – да.
        if (!firstDay)
            combinedHtml += "<div style='page-break-before: always;'>" + dayContent + "</div>";
        else {
            combinedHtml += "<div>" + dayContent + "</div>";
            firstDay = false;
        }
    }

    // Формируем итоговый HTML: общий заголовок + контент по дням + общий подвал (если нужно)
    QString finalHtml = overallHeader + combinedHtml + overallFooter;

    // Создаем QTextDocument и печатаем PDF.
    QTextDocument doc;
    doc.setDefaultStyleSheet(getStyleSheet());
    doc.setHtml(finalHtml);
    doc.setPageSize(printer.pageRect(QPrinter::DevicePixel).size());
    doc.print(&printer);

    QApplication::restoreOverrideCursor();

    QMimeData *mimeData = new QMimeData();
    mimeData->setUrls({ QUrl::fromLocalFile(filePath) });
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData);

    QMessageBox::information(nullptr, "PDF", "Отчёт сохранён в папке 'отчеты' на рабочем столе и скопирован в буфер обмена");
}

void PDFmanager::exportPDFbyDays(const QMap<QDate, QAbstractItemModel*> &modelsByDay,
                                       const QString &title, const QString &dates)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString appDir = getDesktopDir();
    QDir folder(appDir + "/отчеты");
    if (!folder.exists()) {
        folder.mkdir(appDir + "/отчеты");
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    QString fileName = title + " " + currentTime.toString("dd.MM.yyyy HH-mm-ss") + ".pdf";
    fileName.replace(" ", "_");
    QString filePath = appDir + "/отчеты/" + fileName;

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize::A4);
    printer.setOutputFileName(filePath);

    // Получаем отсортированные ключи
    QList<QDate> days = modelsByDay.keys();
    std::sort(days.begin(), days.end());

    QString combinedHtml;
    bool firstDay = true;
    for (const QDate &day : days) {
        QAbstractItemModel *model = modelsByDay.value(day);

        // Формируем контент для данного дня: выводим дату и ниже – таблицу
        QString dayContent;
        dayContent += "<p style='text-align: center; font-weight: bold; font-size: 14pt; margin-bottom: 10px;'>"
                      + day.toString("dd.MM.yyyy") + "</p>";
        dayContent += modelToHTML(model, 0);

        // Для первого дня не добавляем разрыв, для остальных – принудительный разрыв страницы перед блоком.
        if (!firstDay)
            combinedHtml += "<div style='display: block; page-break-before: always;'>" + dayContent + "</div>";
        else {
            combinedHtml += "<div>" + dayContent + "</div>";
            firstDay = false;
        }
    }

    // Добавляем общий заголовок и подвал, если необходимо.
    QString finalHtml = getHeader(currentTime) + combinedHtml + getFooter(currentTime);

    QTextDocument doc;
    doc.setDefaultStyleSheet(getStyleSheet());
    doc.setHtml(finalHtml);
    doc.setPageSize(printer.pageRect(QPrinter::DevicePixel).size());
    doc.print(&printer);

    QApplication::restoreOverrideCursor();

    QMimeData *mimeData = new QMimeData();
    mimeData->setUrls({ QUrl::fromLocalFile(filePath) });
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData);

    QMessageBox::information(nullptr, "PDF", "Отчет сохранен в папке 'отчеты' на рабочем столе и скопирован в буфер обмена");
}

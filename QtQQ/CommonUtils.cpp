#include "CommonUtils.h"
#include <QPainter>
#include <QFile>
#include <QWidget>
#include <QApplication>
#include <QSettings>

CommonUtils::CommonUtils()
{
}

//获取原头像
QPixmap CommonUtils::getRoundImage(const QPixmap& src, QPixmap& mask, QSize masksize)
{
	if (masksize == QSize(0, 0))
	{
		masksize = mask.size();
	}
	else
	{
		// scaled , 缩放
		mask = mask.scaled(masksize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	QImage resultImage(masksize, QImage::Format_ARGB32_Premultiplied);

	QPainter painter(&resultImage);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(resultImage.rect(), Qt::transparent);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.drawPixmap(0, 0, mask);
	painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	painter.drawPixmap(0, 0, src.scaled(masksize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	painter.end();

	return QPixmap::fromImage(resultImage);
}

//加载样式表
void CommonUtils::loadStyleSheet(QWidget* widget, const QString& sheetName)
{
	QFile file(":/Resources/QSS/" + sheetName + ".css");
	file.open(QFile::ReadOnly);

	if (file.isOpen())
	{
		widget->setStyleSheet("");
		QString qsstyleSheet = QLatin1String(file.readAll());
		widget->setStyleSheet(qsstyleSheet);
	}

	file.close();
}

void CommonUtils::setDefaultSkinColor(const QColor& color)
{
	// applicationDirPath() , 目录路径
	const QString&& path = QApplication::applicationDirPath() + "/" + QString("tradeprintinfo.ini");
	QSettings settings(path, QSettings::IniFormat);			// 对配置文件进行读写操作
	settings.setValue("DefaultSkin/red", color.red());
	settings.setValue("DefaultSkin/green", color.green());
	settings.setValue("DefaultSkin/blue", color.blue());
}

QColor CommonUtils::getDefaultSkinColor()
{
	QColor color;
	const QString&& path = QApplication::applicationDirPath() + "/" + QString("tradeprintinfo.ini");
	if (!QFile::exists(path))
	{
		setDefaultSkinColor(QColor(22, 154, 218));
	}

	QSettings settings(path, QSettings::IniFormat);
	color.setRed(settings.value("DefaultSkin/red").toInt());
	color.setGreen(settings.value("DefaultSkin/green").toInt());
	color.setBlue(settings.value("DefaultSkin/blue").toInt());

	return color;
}

bool CommonUtils::IsDigitString(QString strSource)
{
	bool bDigit = false;

	if (strSource.isEmpty()) return bDigit;

	QByteArray strByteArray = strSource.toLatin1();
	const char *chString = strByteArray.data();

	while (*chString && *chString >= '0' && *chString <= '9') chString++;

	if (*chString) bDigit = false;
	else bDigit = true;

	return bDigit;
}

bool CommonUtils::IsEnglish(QString str)
{
	QByteArray ba = str.toLatin1();
	const char *ch = ba.data();
	if (str.length() == 1)
	{
		while (*ch) {
			if ((*ch >= 'A' && *ch <= 'Z') || (*ch >= 'a' && *ch <= 'z')) {

			}
			else return false;

			ch++;
		}
	}
	return true;
}


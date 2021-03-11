#include <QtCore/QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QThread>

#include "Source/DllModule/cximagecrt_drill/src_header/s_cximageManager.h"
#pragma execution_character_set("utf-8")

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	// > 缓冲路径输出
	S_cximageManager::getInstance();
	qDebug() << QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cximagecrt_drill.gif";

	//【注意QPixmap不要在非GUI中创建，线程不安全，会报错！】

	// > 获取帧数
	//QFileInfo info = QFileInfo("F:/ps箱/qq表情ps/小爱丽丝表情包/小爱丽丝_3.gif");
	//qDebug() << info.exists();
	//qDebug() << S_cximageManager::getInstance()->getGIFFrameCount(info);

	// > 拆解gif
	//QFileInfo info = QFileInfo("F:/ps箱/小爱丽丝_5.gif");
	//QDir dir = QDir("F:/ps箱");
	//S_cximageManager::getInstance()->dismantlingGIF(info, dir, "png", "小爱丽丝_%1");
	//qDebug() << S_cximageManager::getInstance()->getLastDismantledGIFIntervalList();
	//qDebug() << S_cximageManager::getInstance()->getLastDismantledError();

	// > 执行合成
	//QList<QFileInfo> info_list = QList<QFileInfo>()
	//	<< QFileInfo("F:/ps箱/小爱丽丝_0.png")
	//	<< QFileInfo("F:/ps箱/小爱丽丝_1.png")
	//	<< QFileInfo("F:/ps箱/小爱丽丝_2.png")
	//	<< QFileInfo("F:/ps箱/小爱丽丝_3.png");
	//QFileInfo target_gif = QFileInfo("F:/ps箱/aaa.gif");
	//S_cximageManager::getInstance()->generateGIF(info_list, target_gif, 4, QList<int>() << 16 << 18 << 16 << 18);
	//qDebug() << S_cximageManager::getInstance()->getLastGenerateError();

	qDebug() << "-----";

	return a.exec();
}

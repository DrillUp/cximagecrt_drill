#include <QtCore/QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

#include "Source/DllModule/cximagecrt_drill/src_header/s_cximageManager.h"
#pragma execution_character_set("utf-8")

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	// > ����·�����
	qDebug() << QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cximagecrt_drill.gif";

	// > ��ȡ֡��
	//QFileInfo info = QFileInfo("F:/ps��/qq����ps/С����˿�����/С����˿_3.gif");
	//qDebug() << info.exists();
	//qDebug() << S_cximageManager::getInstance()->getGIFFrameCount(info);

	// > ���gif
	//QFileInfo info = QFileInfo("F:/ps��/qq����ps/С����˿�����/С����˿_5.gif");
	//QDir dir = QDir("F:/ps��");
	//S_cximageManager::getInstance()->dismantlingGIF(info, dir, "png", "С����˿_%1");
	//qDebug() << S_cximageManager::getInstance()->getLastDismantledGIFIntervalList();

	// > ִ�кϳ�
	QList<QFileInfo> info_list = QList<QFileInfo>()
		<< QFileInfo("F:/ps��/С����˿_1.png")
		<< QFileInfo("F:/ps��/С����˿_2.png")
		<< QFileInfo("F:/ps��/С����˿_3.png");
	QFileInfo target_gif = QFileInfo("F:/ps��/aaa.gif");
	S_cximageManager::getInstance()->generateGIF(info_list, target_gif, 4, QList<int>() << 4 << 8 << 40 );


	return a.exec();
}

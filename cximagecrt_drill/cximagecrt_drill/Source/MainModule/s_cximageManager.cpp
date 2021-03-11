#include "stdafx.h"
#include "s_cximageManager.h"

#include "Source/DllModule/cximagecrt/src_header/ximage.h"
#include "Source/DllModule/cximagecrt/src_header/ximagif.h"
#include "Source/Utils/common/p_FileOperater.h"

/*
-----==========================================================-----
		�ࣺ		ͼ��� ����������.cpp
		�汾��		v1.00
		���ߣ�		drill_up
		����ģ�飺	����ģ��
		���ܣ�		����ͼ���Ĳ��ֲ������ܡ�
					��ע�⣬�����Unicode���򣬲�Ҫ�������ַ�����
					����ϸ��.h��
-----==========================================================-----
*/
S_cximageManager::S_cximageManager(){
	S_cximageManager::cur_manager = this;
	this->_init();
}
S_cximageManager::~S_cximageManager(){
}

/*-------------------------------------------------
		����
*/
S_cximageManager* S_cximageManager::cur_manager = NULL;
S_cximageManager* S_cximageManager::getInstance() {
	if (cur_manager == NULL) {
		cur_manager = new S_cximageManager();
	}
	return cur_manager;
}
/*-------------------------------------------------
		��ʼ��
*/
void S_cximageManager::_init() {

	// > ������
	this->m_lastIntervalList = QList<int>();
	this->m_lastFileList = QList<QFileInfo>();
	this->m_lastDismantledError = "";

	// > �ϳɲ���
	this->m_lastGenerateError = "";

	// > ˽�в���
	this->m_CxImage = nullptr;
}


/*-------------------------------------------------
		��� - ִ�в��
*/
bool S_cximageManager::dismantlingGIF(QFileInfo gif_path, QDir image_dir_path, char* suffix){
	return this->dismantlingGIF(gif_path, image_dir_path, suffix, "%2_%1");
}
bool S_cximageManager::dismantlingGIF(QFileInfo gif_path, QDir image_dir_path, char* suffix, QString imageName){
	if (gif_path.exists() == false){ return false; }
	if (image_dir_path.exists() == false){ return false; }

	// > ���Ƶ�Ӣ��·��
	QFileInfo filePath_to_eng = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cximagecrt_drill.gif");
	P_FileOperater p_FileOperater = P_FileOperater();
	p_FileOperater.copy_File(gif_path, filePath_to_eng);

	// > ִ�в�⣨˽�У�
	bool success = this->dismantlingGIF_private(filePath_to_eng, suffix);
	if (success == false){ return false; };

	// > ���ƻ�ָ��·��
	QString suffix_str = QString(suffix).toLower().replace(".", "");
	this->m_lastFileList.clear();
	for (int i = 0; i < this->m_lastIntervalList.count(); i++){
		QFileInfo filePath_to2 = QFileInfo(image_dir_path.absolutePath() + "/" + QString(imageName).arg(i).arg(gif_path.completeBaseName()) + "." + suffix_str);
		QFileInfo filePath_from = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cximagecrt_drill_" + QString::number(i) + "." + suffix_str);
		p_FileOperater.copy_File(filePath_from, filePath_to2);
		this->m_lastFileList.append(filePath_to2);
	}
	return true;
}
/*-------------------------------------------------
		��� - ��ȡgif֡��
*/
int S_cximageManager::getGIFFrameCount(QFileInfo gif_path) {
	if (gif_path.exists() == false){ return 0; }

	// > ���Ƶ�Ӣ��·��
	QFileInfo filePath_to_eng = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cximagecrt_drill.gif");
	P_FileOperater p_FileOperater = P_FileOperater();
	p_FileOperater.copy_File(gif_path, filePath_to_eng);

	return this->getGIFFrameCount_private(filePath_to_eng);
}
/*-------------------------------------------------
		��� - ��ȡ�����֡�������
*/
QList<int> S_cximageManager::getLastDismantledGIFIntervalList(){
	QList<int> result_list = this->m_lastIntervalList;
	for (int i = 0; i < result_list.count(); i++){
		result_list.replace(i, result_list.at(i));		//��ȡ������֡��λ��qtĬ�϶�ȡ�����10����������������ģ�
	}
	return result_list;
}
/*-------------------------------------------------
		��� - ��ȡ������ļ��б�
*/
QList<QFileInfo> S_cximageManager::getLastDismantledGIFFileList(){
	return this->m_lastFileList;
}
/*-------------------------------------------------
		��� - ��ȡ���ʱ������Ϣ
*/
QString S_cximageManager::getLastDismantledError(){
	return this->m_lastDismantledError;
}


/*-------------------------------------------------
		���� - ִ�в�⣨˽�У�
*/
bool S_cximageManager::dismantlingGIF_private(QFileInfo gif_path, char* suffix){
	this->m_lastDismantledError = "";
	QString file_path = gif_path.absoluteFilePath();
	QString dir_path = gif_path.absolutePath();

	// > ��ʽ����
	ENUM_CXIMAGE_FORMATS format = CXIMAGE_FORMAT_PNG;							//��Ĭ��png��
	QString suffix_str = QString(suffix).toLower().replace(".", "");
	if (suffix == "bmp"){ format = CXIMAGE_FORMAT_BMP; }
	if (suffix == "tga"){ format = CXIMAGE_FORMAT_TGA; }
	if (suffix == "tif" || suffix == "tiff"){ format = CXIMAGE_FORMAT_TIF; }
	if (suffix == "jpg" || suffix == "jpeg"){ format = CXIMAGE_FORMAT_JPG; }

	// > ��ȡ֡��
	CxImage gif_img;
	gif_img.Load(file_path.toLocal8Bit(), CXIMAGE_FORMAT_GIF);
	int frame_count = gif_img.GetNumFrames();
	if (frame_count == 0){ this->m_lastDismantledError = "֡��Ϊ�㣬���ʧ�ܡ�"; return false; }

	// > ��ʼ���
	this->m_lastIntervalList.clear();
	for (int i = frame_count-1; i >= 0; i--) {
		QString file_png = dir_path + "/" + gif_path.completeBaseName() + "_" + QString::number(i) + "." + suffix_str;

		CxImage img = CxImage();	//��һ��Ҫ�½�����Ȼ�����pngδ�������ɫ������
		img.SetFrame(i);			//��Ҫָ����frame����load�����õ�֡��
		img.Load(file_path.toLocal8Bit(), CXIMAGE_FORMAT_GIF);
		img.Save(file_png.toLocal8Bit(), format);
		this->m_lastIntervalList.append(img.GetFrameDelay());

	}
	return true;
}
/*-------------------------------------------------
		���� - ��ȡgif֡����˽�У�
*/
int S_cximageManager::getGIFFrameCount_private(QFileInfo gif_path){
	QString path = gif_path.absoluteFilePath();
	CxImage img;
	img.Load(path.toLocal8Bit(), CXIMAGE_FORMAT_GIF);
	return img.GetNumFrames();
}



/*-------------------------------------------------
		�ϳ� - ִ�кϳ�
*/
bool S_cximageManager::generateGIF( QList<QFileInfo> file_list, QFileInfo gif_path, int frame_interval, QList<int> frame_intervalList ){
	QList<QFileInfo> file_eng_list = QList<QFileInfo>();
	P_FileOperater p_FileOperater = P_FileOperater();

	// > ��ȡͼƬ
	QList<QImage> image_list = QList<QImage>();
	for (int i = 0; i < file_list.count(); i++){
		QFileInfo info = file_list.at(i);
		QImage image = QImage();
		image.load(info.absoluteFilePath());
		image_list.push_back(image);
	}

	// > ��ȡ�߿�
	int max_width = 0;
	int max_height = 0;
	for (int i = 0; i < image_list.count(); i++){
		QImage image = image_list.at(i);
		int w = image.width();
		int h = image.height();
		if (max_width < w){
			max_width = w;
		}
		if (max_height < h){
			max_height = h;
		}
	}


	// > ���Ƶ�Ӣ��·��
	QString eng_gif_path = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cximagecrt_drill.gif";
	QString eng_pic_path = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cximagecrt_drill_";
	for (int i = 0; i < image_list.count(); i++){
		QImage image = image_list.at(i);
		if (image.isNull()){ continue; }

		// > ͼƬ���д���
		int xx = qFloor((max_width - image.width())*0.5);
		int yy = qFloor((max_height - image.height())*0.5);

		QPixmap bitmap(max_width, max_height);
		QColor color = QColor(0, 51, 0, 255);				//�̶� #003300��ɫΪ����ɫ��͸��
		bitmap.fill(color);									//����ɫ
		QPainter painter(&bitmap);							//����
		painter.drawPixmap(xx, yy, QPixmap::fromImage(image));
		painter.setPen(color);
		painter.drawPoint(0, 0);
		painter.end();

		QString file_eng_str = eng_pic_path + QString::number(i) + ".bmp";
		QImage new_image = bitmap.toImage();
		new_image = new_image.convertToFormat(QImage::Format_RGB888);	//����Ϊ24λ���ɫ rgb�ֱ�8λ��
		new_image.save(file_eng_str, "bmp");
		file_eng_list.append(QFileInfo(file_eng_str));

		//˵����
		//	1.	����ת��QImage::Format_Indexed8 ��� 8λ����ɫ��
		//		�������ɵ�����ɫ��ÿ��ͼƬ��colorTable��ɫ������һ����
		//		������gifʱ����ֱ���õ�һ��ͼƬ����ɫ�������֡������Ϳ����������������ɫ���ң�С����˿�仨�����⡿
		//	2.	�������ﻹ����cximage����24λͼƬ����ѹ����Ŀǰû���ҵ�ͳһ����ɫ+�Զ���ɫ�����ķ�����
	}
	
	// > ִ�кϳɣ�˽�У�
	bool success = this->generateGIF_private(file_eng_list, QFileInfo(eng_gif_path), frame_interval, frame_intervalList);
	if ( success == false){ return false; };


	// > ���ƻ�ָ��·��
	p_FileOperater.copy_File(QFileInfo(eng_gif_path), gif_path);
	return true;
}
/*-------------------------------------------------
		�ϳ� - ��ȡ�ϳ�ʱ������Ϣ
*/
QString S_cximageManager::getLastGenerateError(){
	return this->m_lastGenerateError;
}


/*-------------------------------------------------
		���� - ִ�кϳɣ�˽�У�
*/
bool S_cximageManager::generateGIF_private(QList<QFileInfo> file_list, QFileInfo gif_path, int frame_interval, QList<int> frame_intervalList){
	this->m_lastGenerateError = "";

	// > ����ͼ������
	CxImage** imgTank = new CxImage*[file_list.count()];
	if (imgTank == nullptr){ return false; }

	// > ��ʼ��ͼ������
	for (int i = 0; i < file_list.count(); i++){
		QString file_name = file_list.at(i).absoluteFilePath();
		imgTank[i] = new CxImage();
		if (i < frame_intervalList.count()){
			imgTank[i]->SetFrameDelay(frame_intervalList.at(i));
		}else{
			imgTank[i]->SetFrameDelay(frame_interval);
		}
		imgTank[i]->Load(file_name.toLocal8Bit(), CXIMAGE_FORMAT_BMP);
	}

	// > 8λɫ����
	for (int i = 0; i < file_list.count(); i++){

		//	RGBQUAD c = { 255, 255, 255, 0 };
		//	if (imgTank[i]->GetTransIndex() != 24){
		//		imgTank[i]->IncreaseBpp(24);
		//	}
		//	imgTank[i]->SetTransIndex(0);
		//	RGBQUAD rgbTrans = { 255, 255, 255, 0 };
		//	imgTank[i]->SetTransColor(rgbTrans);
		//
		//	imgTank[i]->DecreaseBpp(4, false, (RGBQUAD*)calloc(16 * sizeof(RGBQUAD), 1), 16);
		//
		//	if (!imgTank[i]->AlphaPaletteIsValid()){
		//		imgTank[i]->AlphaPaletteEnable(0);
		//	}

		imgTank[i]->SetTransIndex(0);			//����͸��ɫ
		RGBQUAD rgbTrans = { 0, 51, 0, 0 };		//�̶�rgb(0,51,0) #003300��ɫ͸��
		imgTank[i]->SetTransColor(rgbTrans);
		imgTank[i]->DecreaseBpp(8, false, &rgbTrans, 0);

		//	˵����
		//		1.	gif���ɱ�����8λ����ɫ�ſ��ԣ��������Ҫ������λ������С�
		//		2.	gif͸��ԭ���Ǳ��롾ѡһ������ɫ�������������ɫΪ͸��ɫ���ſ���͸�����൱����ɫ��ƭ��
		//		3.	����������ɫ�����Ա���˳��һ�£�������ֻѡ��һ��ͼƬ����ɫ��
		//			�����ɫ��һ����ͼƬ��仨��
	}

	// > �����ļ�
	CxIOFile gif_file;
	gif_file.Open(gif_path.absoluteFilePath().toLocal8Bit(), "wb");

	// > д��GIF
	CxImageGIF multiimage;
	multiimage.SetLoops(-1);		//������3��ֻ����3�Σ�
	multiimage.SetDisposalMethod(2);
	bool success = multiimage.Encode(&gif_file, imgTank, file_list.count(), false, false);
	if ( success == false){
		this->m_lastGenerateError = multiimage.GetLastError();
		return false;
	}

	gif_file.Close();
	return true;
}
/*-------------------------------------------------
		���� - ִ�кϳɣ�˽�У�ͼ�����ص㸳ֵ��

		���÷�����ʱ�ò��ϣ���Ҫ�����ǣ�24λ���ɫͼƬ �޷�תΪ8λ���ɫ���⡣��

bool S_cximageManager::generateGIF_private(QList<QImage> image_list, QFileInfo gif_path, int frame_interval, QList<int> frame_intervalList){
	this->m_lastGenerateError = "";

	// > ����ͼ������
	CxImage** imgTank = new CxImage*[image_list.count()];
	if (imgTank == nullptr){ return false; }

	for (int i = 0; i < image_list.count(); i++){
		QImage qimage = image_list.at(i);
		imgTank[i] = new CxImage();
		imgTank[i]->Create(qimage.width(), qimage.height(), 0, CXIMAGE_FORMAT_PNG);

		// > ����֡���
		if (i < frame_intervalList.count()){
			imgTank[i]->SetFrameDelay(frame_intervalList.at(i));
		}else{
			imgTank[i]->SetFrameDelay(frame_interval);
		}
		// > ��ɫ
		for (int yy = 0; yy < qimage.height(); yy++){
			for (int xx = 0; xx < qimage.width(); xx++){
				QRgb rgb = qimage.pixel(xx, yy);
				int r = qRed(rgb);
				int g = qGreen(rgb);
				int b = qBlue(rgb);
				int a = qAlpha(rgb);
				COLORREF color = (unsigned long)r + g >> 8 + b >> 16 + a >> 24;
				imgTank[i]->SetPixelColor(xx, yy, color);
			}
		}
	}

	// > �����ļ�
	CxIOFile gif_file;
	gif_file.Open(gif_path.absoluteFilePath().toLocal8Bit(), "wb");

	// > д��GIF
	CxImageGIF multiimage;
	//multiimage.SetLoops(3);	//�����ú�򿪣���ֻ����3�Σ�
	multiimage.SetDisposalMethod(2);
	bool success = multiimage.Encode(&gif_file, imgTank, image_list.count(), false, false);
	if ( success == false){
		this->m_lastGenerateError = multiimage.GetLastError();
		return false;
	}

	gif_file.Close();
	return true;
}
*/

#include "stdafx.h"
#include "s_cximageManager.h"

#include "Source/DllModule/cximagecrt/src_header/ximage.h"
#include "Source/DllModule/cximagecrt/src_header/ximagif.h"
#include "Source/Utils/common/p_FileOperater.h"

/*
-----==========================================================-----
		类：		图像库 操作管理器.cpp
		版本：		v1.00
		作者：		drill_up
		所属模块：	工具模块
		功能：		导出图像库的部分操作功能。
					【注意，这里非Unicode区域，不要用中文字符串】
					（详细见.h）
-----==========================================================-----
*/
S_cximageManager::S_cximageManager(){
	S_cximageManager::cur_manager = this;
	this->_init();
}
S_cximageManager::~S_cximageManager(){
}

/*-------------------------------------------------
		单例
*/
S_cximageManager* S_cximageManager::cur_manager = NULL;
S_cximageManager* S_cximageManager::getInstance() {
	if (cur_manager == NULL) {
		cur_manager = new S_cximageManager();
	}
	return cur_manager;
}
/*-------------------------------------------------
		初始化
*/
void S_cximageManager::_init() {

	// > 拆解参数
	this->m_lastIntervalList = QList<int>();
	this->m_lastFileList = QList<QFileInfo>();
	this->m_lastDismantledError = "";

	// > 合成参数
	this->m_lastGenerateError = "";

	// > 私有参数
	this->m_CxImage = nullptr;
}


/*-------------------------------------------------
		拆解 - 执行拆解
*/
bool S_cximageManager::dismantlingGIF(QFileInfo gif_path, QDir image_dir_path, char* suffix){
	return this->dismantlingGIF(gif_path, image_dir_path, suffix, "%2_%1");
}
bool S_cximageManager::dismantlingGIF(QFileInfo gif_path, QDir image_dir_path, char* suffix, QString imageName){
	if (gif_path.exists() == false){ return false; }
	if (image_dir_path.exists() == false){ return false; }

	// > 复制到英文路径
	QFileInfo filePath_to_eng = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cximagecrt_drill.gif");
	P_FileOperater p_FileOperater = P_FileOperater();
	p_FileOperater.copy_File(gif_path, filePath_to_eng);

	// > 执行拆解（私有）
	bool success = this->dismantlingGIF_private(filePath_to_eng, suffix);
	if (success == false){ return false; };

	// > 复制回指定路径
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
		拆解 - 获取gif帧数
*/
int S_cximageManager::getGIFFrameCount(QFileInfo gif_path) {
	if (gif_path.exists() == false){ return 0; }

	// > 复制到英文路径
	QFileInfo filePath_to_eng = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cximagecrt_drill.gif");
	P_FileOperater p_FileOperater = P_FileOperater();
	p_FileOperater.copy_File(gif_path, filePath_to_eng);

	return this->getGIFFrameCount_private(filePath_to_eng);
}
/*-------------------------------------------------
		拆解 - 获取拆解后的帧间隔数据
*/
QList<int> S_cximageManager::getLastDismantledGIFIntervalList(){
	QList<int> result_list = this->m_lastIntervalList;
	for (int i = 0; i < result_list.count(); i++){
		result_list.replace(i, result_list.at(i));		//获取到的是帧单位（qt默认读取器会大10倍，但这个是正常的）
	}
	return result_list;
}
/*-------------------------------------------------
		拆解 - 获取拆解后的文件列表
*/
QList<QFileInfo> S_cximageManager::getLastDismantledGIFFileList(){
	return this->m_lastFileList;
}
/*-------------------------------------------------
		拆解 - 获取拆解时错误信息
*/
QString S_cximageManager::getLastDismantledError(){
	return this->m_lastDismantledError;
}


/*-------------------------------------------------
		操作 - 执行拆解（私有）
*/
bool S_cximageManager::dismantlingGIF_private(QFileInfo gif_path, char* suffix){
	this->m_lastDismantledError = "";
	QString file_path = gif_path.absoluteFilePath();
	QString dir_path = gif_path.absolutePath();

	// > 格式设置
	ENUM_CXIMAGE_FORMATS format = CXIMAGE_FORMAT_PNG;							//（默认png）
	QString suffix_str = QString(suffix).toLower().replace(".", "");
	if (suffix == "bmp"){ format = CXIMAGE_FORMAT_BMP; }
	if (suffix == "tga"){ format = CXIMAGE_FORMAT_TGA; }
	if (suffix == "tif" || suffix == "tiff"){ format = CXIMAGE_FORMAT_TIF; }
	if (suffix == "jpg" || suffix == "jpeg"){ format = CXIMAGE_FORMAT_JPG; }

	// > 获取帧数
	CxImage gif_img;
	gif_img.Load(file_path.toLocal8Bit(), CXIMAGE_FORMAT_GIF);
	int frame_count = gif_img.GetNumFrames();
	if (frame_count == 0){ this->m_lastDismantledError = "帧数为零，拆解失败。"; return false; }

	// > 开始拆解
	this->m_lastIntervalList.clear();
	for (int i = frame_count-1; i >= 0; i--) {
		QString file_png = dir_path + "/" + gif_path.completeBaseName() + "_" + QString::number(i) + "." + suffix_str;

		CxImage img = CxImage();	//（一定要新建，不然会出现png未清理的颜色残留）
		img.SetFrame(i);			//（要指定了frame后，再load才能拿到帧）
		img.Load(file_path.toLocal8Bit(), CXIMAGE_FORMAT_GIF);
		img.Save(file_png.toLocal8Bit(), format);
		this->m_lastIntervalList.append(img.GetFrameDelay());

	}
	return true;
}
/*-------------------------------------------------
		操作 - 获取gif帧数（私有）
*/
int S_cximageManager::getGIFFrameCount_private(QFileInfo gif_path){
	QString path = gif_path.absoluteFilePath();
	CxImage img;
	img.Load(path.toLocal8Bit(), CXIMAGE_FORMAT_GIF);
	return img.GetNumFrames();
}



/*-------------------------------------------------
		合成 - 执行合成
*/
bool S_cximageManager::generateGIF( QList<QFileInfo> file_list, QFileInfo gif_path, int frame_interval, QList<int> frame_intervalList ){
	QList<QFileInfo> file_eng_list = QList<QFileInfo>();
	P_FileOperater p_FileOperater = P_FileOperater();

	// > 读取图片
	QList<QImage> image_list = QList<QImage>();
	for (int i = 0; i < file_list.count(); i++){
		QFileInfo info = file_list.at(i);
		QImage image = QImage();
		image.load(info.absoluteFilePath());
		image_list.push_back(image);
	}

	// > 获取高宽
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


	// > 复制到英文路径
	QString eng_gif_path = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cximagecrt_drill.gif";
	QString eng_pic_path = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cximagecrt_drill_";
	for (int i = 0; i < image_list.count(); i++){
		QImage image = image_list.at(i);
		if (image.isNull()){ continue; }

		// > 图片居中处理
		int xx = qFloor((max_width - image.width())*0.5);
		int yy = qFloor((max_height - image.height())*0.5);

		QPixmap bitmap(max_width, max_height);
		QColor color = QColor(0, 51, 0, 255);				//固定 #003300颜色为背景色，透明
		bitmap.fill(color);									//填充底色
		QPainter painter(&bitmap);							//画家
		painter.drawPixmap(xx, yy, QPixmap::fromImage(image));
		painter.setPen(color);
		painter.drawPoint(0, 0);
		painter.end();

		QString file_eng_str = eng_pic_path + QString::number(i) + ".bmp";
		QImage new_image = bitmap.toImage();
		new_image = new_image.convertToFormat(QImage::Format_RGB888);	//（存为24位真彩色 rgb分别8位）
		new_image.save(file_eng_str, "bmp");
		file_eng_list.append(QFileInfo(file_eng_str));

		//说明：
		//	1.	可以转成QImage::Format_Indexed8 变成 8位索引色。
		//		但是生成的索引色后，每张图片的colorTable颜色表，都不一样。
		//		【生成gif时，会直接拿第一张图片的颜色表对所有帧进行填涂，这就造成了索引颜色混乱，小爱丽丝变花的问题】
		//	2.	所以这里还是让cximage来对24位图片进行压缩，目前没有找到统一索引色+自动颜色背景的方法。
	}
	
	// > 执行合成（私有）
	bool success = this->generateGIF_private(file_eng_list, QFileInfo(eng_gif_path), frame_interval, frame_intervalList);
	if ( success == false){ return false; };


	// > 复制回指定路径
	p_FileOperater.copy_File(QFileInfo(eng_gif_path), gif_path);
	return true;
}
/*-------------------------------------------------
		合成 - 获取合成时错误信息
*/
QString S_cximageManager::getLastGenerateError(){
	return this->m_lastGenerateError;
}


/*-------------------------------------------------
		操作 - 执行合成（私有）
*/
bool S_cximageManager::generateGIF_private(QList<QFileInfo> file_list, QFileInfo gif_path, int frame_interval, QList<int> frame_intervalList){
	this->m_lastGenerateError = "";

	// > 建立图像数组
	CxImage** imgTank = new CxImage*[file_list.count()];
	if (imgTank == nullptr){ return false; }

	// > 初始化图像数组
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

	// > 8位色处理
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

		imgTank[i]->SetTransIndex(0);			//设置透明色
		RGBQUAD rgbTrans = { 0, 51, 0, 0 };		//固定rgb(0,51,0) #003300颜色透明
		imgTank[i]->SetTransColor(rgbTrans);
		imgTank[i]->DecreaseBpp(8, false, &rgbTrans, 0);

		//	说明：
		//		1.	gif生成必须是8位索引色才可以，否则必须要经过降位处理才行。
		//		2.	gif透明原理，是必须【选一个背景色】设置这个背景色为透明色，才可以透明。相当于颜色欺骗。
		//		3.	由于是索引色，所以必须顺序一致，该生成只选第一个图片的颜色表。
		//			如果颜色表不一样，图片会变花。
	}

	// > 建立文件
	CxIOFile gif_file;
	gif_file.Open(gif_path.absoluteFilePath().toLocal8Bit(), "wb");

	// > 写入GIF
	CxImageGIF multiimage;
	multiimage.SetLoops(-1);		//（设置3将只播放3次）
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
		操作 - 执行合成（私有，图像像素点赋值）

		【该方法暂时用不上，主要问题是：24位真彩色图片 无法转为8位真彩色问题。】

bool S_cximageManager::generateGIF_private(QList<QImage> image_list, QFileInfo gif_path, int frame_interval, QList<int> frame_intervalList){
	this->m_lastGenerateError = "";

	// > 建立图像数组
	CxImage** imgTank = new CxImage*[image_list.count()];
	if (imgTank == nullptr){ return false; }

	for (int i = 0; i < image_list.count(); i++){
		QImage qimage = image_list.at(i);
		imgTank[i] = new CxImage();
		imgTank[i]->Create(qimage.width(), qimage.height(), 0, CXIMAGE_FORMAT_PNG);

		// > 设置帧间隔
		if (i < frame_intervalList.count()){
			imgTank[i]->SetFrameDelay(frame_intervalList.at(i));
		}else{
			imgTank[i]->SetFrameDelay(frame_interval);
		}
		// > 填色
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

	// > 建立文件
	CxIOFile gif_file;
	gif_file.Open(gif_path.absoluteFilePath().toLocal8Bit(), "wb");

	// > 写入GIF
	CxImageGIF multiimage;
	//multiimage.SetLoops(3);	//（设置后打开，将只播放3次）
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

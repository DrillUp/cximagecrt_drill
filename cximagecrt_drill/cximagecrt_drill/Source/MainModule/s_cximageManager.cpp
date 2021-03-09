#include "stdafx.h"
#include "s_cximageManager.h"

#include "Source/DllModule/cximagecrt/src_header/ximage.h"
#include "Source/DllModule/cximagecrt/src_header/ximagif.h"
#include "Source/Utils/common/p_FileOperater.h"
#include <QImage>

/*
-----==========================================================-----
		类：		图像库 操作管理器.cpp
		版本：		v1.00
		作者：		drill_up
		所属模块：	工具模块
		功能：		导出图像库的部分操作功能。
					【注意，这里非Unicode区域，不要用中文字符串】

		目标：		-> 拆解GIF
					-> 合成GIF

		使用方法：
				> 合成：
					QList<QFileInfo> info_list = QList<QFileInfo>();
					info_list.push_back(QFileInfo("F:/新建文件夹/0.png"));
					info_list.push_back(QFileInfo("F:/新建文件夹/1.png"));
					info_list.push_back(QFileInfo("F:/新建文件夹/2.png"));
					S_GIFManager::getInstance()->generateGIF(info_list, QFileInfo("F:/aaa.gif"),4);
				> 拆解：
					S_GIFManager::getInstance()->dismantlingGIF(QFileInfo("F:/gif箱/aaa.gif"),QDir("F:/新建文件夹/"),"png");
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
		if ( image.width() == max_width && 
			 image.height() == max_height ){

			// > 直接复制文件
			QFileInfo file_eng = QFileInfo(eng_pic_path + QString::number(i) + ".png");
			p_FileOperater.copy_File(file_list.at(i), file_eng);
			file_eng_list.append(file_eng);
		
		}else{

			// > 图片居中处理
			//...
		}
	}

	// > 执行合成（私有）
	bool success = this->generateGIF_private(file_eng_list, QFileInfo(eng_gif_path), frame_interval, frame_intervalList);
	if (success == false){ return false; };

	// > 复制回指定路径
	p_FileOperater.copy_File(QFileInfo(eng_gif_path), gif_path);
	return true;
}



/*-------------------------------------------------
		操作 - 执行拆解（私有）
*/
bool S_cximageManager::dismantlingGIF_private(QFileInfo gif_path, char* suffix){
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
	if (frame_count == 0){ return false; }

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
		操作 - 执行合成（私有）
*/
bool S_cximageManager::generateGIF_private(QList<QFileInfo> file_list, QFileInfo gif_path, int frame_interval, QList<int> frame_intervalList){

	// > 建立图像数组
	CxImage** imgTank = new CxImage*[file_list.count()];
	if (imgTank == nullptr){ return false; }

	for( int i = 0; i < file_list.count(); i++ ){
		QString file_name = file_list.at(i).absoluteFilePath();
		imgTank[i] = new CxImage();
		if (i < frame_intervalList.count()){
			imgTank[i]->SetFrameDelay(frame_intervalList.at(i));
		}else{
			imgTank[i]->SetFrameDelay(frame_interval);
		}
		imgTank[i]->Load(file_name.toLocal8Bit(), CXIMAGE_FORMAT_PNG);
	}

	// > 建立文件
	CxIOFile gif_file;
	gif_file.Open(gif_path.absoluteFilePath().toLocal8Bit(), "wb");

	// > 写入GIF
	CxImageGIF multiimage;
	//multiimage.SetLoops(3);	//（设置后打开，将只播放3次）
	multiimage.SetDisposalMethod(2);
	multiimage.Encode(&gif_file, imgTank, file_list.count(), false, false);

	gif_file.Close();
	return true;
}


/*
int TraverseFolder(const string strFilePath, string strImageNameSets[])
{
	int iImageCount = 0;

	_finddata_t fileInfo;

	long handle = _findfirst(strFilePath.c_str(), &fileInfo);

	if (handle == -1L) {
		cerr << "failed to transfer files" << endl;
		return -1;
	}

	do {
		//cout << fileInfo.name <<endl;
		strImageNameSets[iImageCount] = (string)fileInfo.name;

		iImageCount++;

	} while (_findnext(handle, &fileInfo) == 0);

	return iImageCount;
}

void encoding_gif(string strImgPath, string strGifName)
{
	string strImgSets[100] = {};

	int iImgCount = TraverseFolder(strImgPath, strImgSets);

	string strTmp = strImgPath.substr(0, strImgPath.find_last_of("/") + 1);

	CxImage** img = new CxImage*[iImgCount];
	if (img == NULL) {
		cout << "new Cximage error!" << endl;
		return;
	}

	for (int i = 0; i < iImgCount; i++) {
		string tmp1;
		tmp1 = strTmp + strImgSets[i];
		img[i] = new CxImage;
		img[i]->Load(tmp1.c_str(), CXIMAGE_FORMAT_PNG);
	}

	CxIOFile hFile;
	hFile.Open(strGifName.c_str(), "wb");

	CxImageGIF multiimage;

	multiimage.SetLoops(3);
	multiimage.SetDisposalMethod(2);
	multiimage.Encode(&hFile, img, iImgCount, false, false);

	hFile.Close();

	}*/
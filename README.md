# cximagecrt_drill.dll 图像处理库（目前只有GIF生成和拆解）

	该dll是由 qt5.5 + vs2013 编写而成，能够打包成32位程序，支持xp系统。
	
	该dll 基于 cximagecrt.dll 封装了一层，过滤了古老的c++不支持中文的问题，并且支持QString。
	cximagecrt.dll 这个库我也编译了release版本，在DllModule里面能找到。
	
	工程目录下有两个项目：
		cximagecrt_drill 
		cximagecrt_drill_Test 
	
		两个项目相互独立，没有关联。
		在Test项目里面，放置了前者项目生成的 .h .lib .dll 三种文件，调用后实测有效。

## cximagecrt_drill.dll GIF生成/拆解库

这里我基于 cximagecrt.dll 封装了一层，过滤了古老的c++不支持中文的问题，并且支持QString。<br/>
cximagecrt.dll 这个库我也编译了release版本，在DllModule里面能找到。

	该dll是由 qt5.5 + vs2013 编写而成，能够打包成32位程序，支持xp系统。

>工程目录下有两个项目：<br/>
>cximagecrt_drill <br/>
>cximagecrt_drill_Test <br/>
><br/>
>两个项目相互独立，没有关联。<br/>
>在Test项目里面，放置了前者项目生成的 .h .lib .dll 三种文件，调用后实测有效。

1.Java Demo介绍:
  Java的API是对Cpp中的sample code通过JNI做的二次封装,所以和原生的录制SDK提供的C++ API不完全一样,大概结构是samples下agorasdk(这里shared C++的sample code)实现录制原生C++ API的接口以及对callback的处理,然后JNI层再封装agorasdk,通过JNI proxy层提供native接口供Java层调用.
    
2.Java环境:
  请确保您的Java 环境已经就绪,比如可以正常的编译并运行hello world.

3.demo简单使用说明:
  编译后bin文件夹下会生成librecording.so,此动态库是封装了samples下agorasdk APIs后生成的动态库文件,java demo可以通过JNI的方式调用.下面是具体的编译步骤及简单说明.
  第一步:
    使用 source build.sh pre_set xxxx 来进行预设值,有两个作用:
      1)设置本机JNI的路径,其中xxx是jni.h所在的文件绝对路径,可以使用locate jni.h 或者 find /usr -name jni.h 等方式来找到jni.h所在路径,如source build.sh pre_set /usr/lib/jvm/java-xxxxx-amd64/include;
      2)使得java中的CLASSPATH和LD_LIBRARY_PATH环境变量生效.
  第二步:
    build使用: ./build.sh build ; clean使用./build clean 
4.注意:
  a.bin文件夹是生成java可执行文件和C++ so的地方,在执行clean的时候,会把bin下面的so,.class,headers删除.
  b.Java调用so时会根据LD_LIBRARY_PATH环境变量来寻找,此值已经在build.sh脚本中临时指定到bin文件夹下;CLASSPATH也是临时指定到bin文件夹下,但这个两个环境变量只是临时设置,在其他窗口打开不会生效,需要再执行第一步方可,您也可以根据需要自行设置成全局变量.


#include <QCoreApplication>
#include <QCommandLineParser>
#include <QtXml>
#include <QTextCodec>
QString defKey = "\\bwarning\\b|\\btodo\\b|\\bfixme\\b|\\bfixed\\b|\\binfo\\b|\\bbug\\b";

/**
  * 函数功能: 将文本数据转换成unicode
  * 输入参数: [data] text data
  * 返 回 值: [QString]
  * 说    明:
  */
QString unicodeText( QByteArray &data)
{
    QTextCodec::ConverterState cs;
    //
    QTextCodec *txtCodec = QTextCodec::codecForName("GB2312");
    QString str = txtCodec->toUnicode( data.data(), data.size(), &cs);
    if( cs.invalidChars == 0 ){
        return str;
    }
    //
    txtCodec = QTextCodec::codecForName("UTF-8");
    str = txtCodec->toUnicode( data.data(), data.size(), &cs);
    if( cs.invalidChars == 0 ){
        return str;
    }
    return QString(data);
}
/**
  * 函数功能: 从uVison project file 获取文件列表
  * 输入参数: [projectPath] *.uvprojx
  * 返 回 值: [QStringList]
  * 说    明:
  */
QStringList getFileList(QString &projectPath)
{
    QDomDocument *xml;
    xml = new QDomDocument();
    QFile *file = new QFile(projectPath);

    if(!xml->setContent(file)) {
        delete xml;
        xml = NULL;
        qDebug() << "#error" << "xml object set content fail";
        return QStringList();
    }

    QString header = xml->documentElement().elementsByTagName("Header").item(0).toElement().text();

    if( !header.contains("uVision Project") ) {
        delete xml;
        qDebug() << "#error" << "not a uVision project file";
        return QStringList();
    }

    file->close();
    delete file;
    file = NULL;

    QStringList filePaths;
    QDomElement rootElement = xml->documentElement();
    QDomNodeList elementList = rootElement.elementsByTagName("File");
    qint32 count = elementList.count();

    for( qint32 i = 0; i < count; i++ ) {
        QDomElement tmp = elementList.at(i).toElement().firstChildElement("FilePath");
        QString path = QDir::fromNativeSeparators( tmp.text() );
        filePaths << path;
    }


    QString outputDir = rootElement.elementsByTagName("OutputDirectory").at(0).toElement().text();
    QFileInfo info( projectPath );
    QDir dir(info.path());
    dir.cd(outputDir);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);
    QFileInfoList list = dir.entryInfoList();
    QStringList includeList;
    foreach( QFileInfo info, list) {
        if( info.suffix() == "d" ) {
            file = new QFile( info.filePath() );

            if( file->open(QIODevice::ReadOnly)) {
                while( !file->atEnd() ) {
                    QString path = QString::fromLocal8Bit( file->readLine());
                    QRegExp reg (": .*\\.h");
                    reg.indexIn(path);
                    path = reg.capturedTexts().at(0);

                    if( !path.isEmpty() ) {
                        includeList << path.mid(2);
                    }
                }
            }
            delete file;
            file = NULL;
        }
    }
    includeList = includeList.toSet().toList();
    return filePaths << includeList;
}

/**
  * 函数功能: 扫描文件
  * 输入参数: [path]文件路径 [key]扫描的关键字
  * 返 回 值: Log message
  * 说    明:
  */
QStringList scanFile(QString &path, QString &key)
{
    QFile *file = new QFile(path);
    QStringList msgLst;

    if( file->open( QIODevice::ReadOnly) ) {
        qint32 linenum = 0;
        while( !file->atEnd() ) {
            QByteArray data = file->readLine();
            QString txt = unicodeText(data);
            linenum++;
            QRegExp reg(key);
            reg.setCaseSensitivity( Qt::CaseInsensitive );

            if( reg.indexIn(txt, 0) != -1 ) {
                QString type = reg.cap(0);
                QString location = path + "(" + QString::number(linenum) + ")";
                QString log("%1: [%2] %3");
                msgLst << log.arg( location, type, txt );
            }
        }
        file->close();
    }
    delete file;
    return msgLst;
}


/**
  * 函数功能:
  * 输入参数:
  * 返 回 值:
  * 说    明:
  */
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setApplicationVersion("1.0"); // 版本号
    a.setApplicationName("KeilTodo");
    QCommandLineParser cmdParser;
    // 定义实例
    cmdParser.setApplicationDescription( "list key word from keil project or source file"); // 描述可执行程序的属性
    cmdParser.addHelpOption();    // 添加帮助命令
    cmdParser.addVersionOption(); // 添加版本选择命令

    // 定义一个命令
    QCommandLineOption optKey("k"); // src file
    optKey.setValueName( "keyword" );   // 值名,设置ValueName后,解析器会认为此命令带值,可以根据名字索引值,强调必须带值
    optKey.setDescription("specific keyword to be scan"); // 命令选项描述
    cmdParser.addOption( optKey );

    // 任何不带'-'或"--"的参数，都是PositionalArgument,
    cmdParser.addPositionalArgument("files", QCoreApplication::translate("files", "any files or mdk-arm project file"));

    // 解析应用进程的参数
    if( argc == 1 ){
        cmdParser.showHelp();
        return 0;
    }
    cmdParser.process( a );

    QString key = defKey; // 使用默认的关键字
    // 扫描特定关键字
    if( cmdParser.isSet( optKey ) ){
        key.clear();
        QStringList keyLst = cmdParser.values(optKey);;
        foreach( QString k, keyLst ){
            if( !key.isEmpty() ){
                key.append("|");
            }
            k.append("\\b");
            k.prepend("\\b");
            key.append(k);
        }
    }
    // keil工程文件或者是其他文件
    QStringList files;
    files = cmdParser.positionalArguments();
    foreach( QString file, files ){
        QFileInfo info(file);
        if( info.isFile() ){
            QString str = key;
            qDebug().noquote().nospace() << "keyword - "<<str.remove("\\b").replace("|", " ");
            // keil工程文件
            if(info.suffix() == "uvprojx"){
                QStringList filePathList = getFileList(file);

                foreach( QString filepath, filePathList ) {
                    QStringList msgLst;
                    msgLst << scanFile(filepath, key);
                    foreach (QString msg, msgLst) {
                        qDebug().noquote()<<msg;
                    }
                }
            } else {
                // 其他文件
                QStringList msgLst;
                msgLst << scanFile(file, key);
                foreach (QString msg, msgLst) {
                    qDebug().noquote() << msg;
                }
            }
        } else {
            qDebug().noquote() << file << "is not a valid file";
        }
    }
    return 0;
}

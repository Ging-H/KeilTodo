#include <QCoreApplication>
#include <QCommandLineParser>
#include <QtXml>
#include <QTextCodec>

QString defKey = "\\s@warning|\\s@todo|\\s@fixme|\\s@fixed|\\s@info|\\s@bug";

/**
  * 函数功能: 将文本数据转换成unicode
  * 输入参数: [data] text data
  * 返 回 值: data的unicode字符串
  * 说    明:
  */
QString unicodeText( QByteArray &data)
{
    QTextCodec::ConverterState cs;
    //
    QTextCodec *txtCodec = QTextCodec::codecForName("GB2312");
    QString str = txtCodec->toUnicode( data.data(), data.size(), &cs);

    if( cs.invalidChars == 0 ) {
        return str;
    }

    //
    txtCodec = QTextCodec::codecForName("UTF-8");
    str = txtCodec->toUnicode( data.data(), data.size(), &cs);

    if( cs.invalidChars == 0 ) {
        return str;
    }

    return QString(data);
}
/**
  * 函数功能: 从uVison project file 获取文件列表
  * 输入参数: [projectPath] *.uvprojx
  * 返 回 值: Keil-MDK的里面包含的所有源文件列表
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
        qint32 line = 0;

        while( !file->atEnd() ) {
            QByteArray data = file->readLine();
            QString txt = unicodeText(data); // GB2312, UTF-8 to unicode
            line++;

            QRegularExpression regexp(key);
            regexp.setPatternOptions( QRegularExpression::CaseInsensitiveOption );

            /* 每行检测多个关键字 */
            QRegExp reg(key);
            reg.setCaseSensitivity( Qt::CaseInsensitive );
            qint32 pos = 0;
            while( (pos = reg.indexIn(txt, pos)) != -1 ){
                QString type = reg.cap(0).mid(2); // remove the first '@'
                QString file = path + "(" + QString::number(line) + ")";
                QString log("%1: [%2] %3");
                if( type == "{" || type == "}" ){
                    pos += reg.matchedLength();
                    continue;
                }else{
                    msgLst << log.arg( file, type, txt );
                    pos += reg.matchedLength();
                }
            }
//            /* 每行只检测一个关键字 */
//            QRegExp reg(key);
//            reg.setCaseSensitivity( Qt::CaseInsensitive );
//            if( reg.indexIn(txt, 0) != -1 ) {
//                QString type = reg.cap(0).mid(2); // remove the first '@'
//                QString location = path + "(" + QString::number(line) + ")";
//                QString log("%1: [%2] %3");
//                msgLst << log.arg( location, type, txt );
//            }
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
  * 说    明: 输入关键字: "-k=info"  标记关键字 "@info\s"(\s为空格)
  */
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setApplicationVersion("1.2"); // 版本号
    a.setApplicationName("KeilTodo");

    /* 命令解析器,用于提示使用方法 */
    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription( "list keyword from Keil-MDKv5 project or source file");
    cmdParser.addHelpOption();    // 添加-h选项
    cmdParser.addVersionOption(); // 添加-v选项
    // 自定义命令
    QCommandLineOption optKey("k");     // 添加-k选项,关键字
    optKey.setValueName( "keyword" );   // -k值名,设置ValueName后,解析器会认为此命令带值,可以根据名字索引值,强调必须带值
    optKey.setDescription("specific keyword to be scan"); // 命令选项描述
    cmdParser.addOption( optKey );

    QCommandLineOption optLst("l");     // -l,列出/列表
    optLst.setDescription("list all keyword in file");
    cmdParser.addOption( optLst );

    QCommandLineOption optHal("d");     // -h,包含hal库
    optHal.setDescription("search hal/ll drivers");
    cmdParser.addOption( optHal );

    // 任何不带'-'或"--"的参数，都是PositionalArgument,
    cmdParser.addPositionalArgument("files", QCoreApplication::translate("files", "Any number of text-files or one Keil-MDKv5 project file"));

    // 解析应用进程的参数
    if( argc == 1 ) {
        cmdParser.showHelp();
        return 0;
    }

    cmdParser.process( a );
    QString key;
    // 扫描所有关键字
    if( cmdParser.isSet( optLst ) ) {
        key.clear();
        key.append("\\s@[\\S]+");
        qDebug().noquote().nospace() << key;
    }else{
        if( cmdParser.isSet( optKey ) ) {
            // 扫描特定关键字
            QStringList keyLst;
            QString str;
            keyLst = cmdParser.values(optKey);
            foreach( QString k, keyLst ) {
                str.append(k).append(' ');
                k.prepend("\\s@").append('|');
                key.append(k);
            }
            if(key.endsWith('|')){
                key.chop(1);
            }

            qDebug().noquote().nospace() << "keyword - "<<str;
        }else{
            // 扫描默认关键字
            key = defKey;
            QStringList keyLst = key.split('|');
            QString str;
            foreach( QString k, keyLst ) {
                k.remove("\\s@").append(' ');
                str.append(k);
            }
            qDebug().noquote().nospace() << "keyword - "<<str;
        }
    }

    bool isSearchDriver = false;
    if( cmdParser.isSet(optHal) ){
        isSearchDriver = true;
    }

    // keil工程文件或者是其他文件
    QStringList files;
    files = cmdParser.positionalArguments();
    foreach( QString file, files ) {
        QFileInfo info(file);

        if( info.isFile() ) {
            // keil工程文件
            if(info.suffix() == "uvprojx") {
                QStringList filePathList = getFileList(file);
                foreach( QString filepath, filePathList ) {

                    // 跳过hal/ll驱动库文件
                    if( !isSearchDriver ){
                        if( filepath.contains(QRegExp("/Drivers/.*\\.[ch]")) ){
                            continue;
                        }
                    }

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

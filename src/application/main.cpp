#include "motion_ctrl_widget.h"
#include <QApplication>

// #include <QDebug>
// #include <QString>
// #include <QByteArray>
// #include <QRegularExpression>

// // Windows 堆栈捕获（MinGW/GCC）

// #include <windows.h>
// #include <dbghelp.h>
// #pragma comment(lib, "dbghelp.lib")
// static QString getStackTrace()
// {
//     QString stack;
//     void* stackFrames[64];
//     HANDLE process = GetCurrentProcess();
//     SymInitialize(process, NULL, TRUE);
//     WORD frames = CaptureStackBackTrace(0, 64, stackFrames, NULL);
//     SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
//     symbol->MaxNameLen = 255;
//     symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
//     for (WORD i = 0; i < frames; ++i) {
//         DWORD64 address = (DWORD64)stackFrames[i];
//         if (SymFromAddr(process, address, 0, symbol)) {
//             stack += QString("[%1] %2\n").arg(i).arg(symbol->Name);
//         } else {
//             stack += QString("[%1] %2\n").arg(i).arg("???");
//         }
//     }
//     free(symbol);
//     return stack;
// }


// void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
// {
//     // 只处理警告消息，且内容包含“signal not found”
//     if (type == QtWarningMsg && msg.contains("QObject::connect: signal not found")) {
//         QString fullMsg = QString("⚠️ %1\n\nCall stack:\n%2")
//                               .arg(msg)
//                               .arg(getStackTrace());
//         // 输出到标准错误（或者写入日志文件）
//         fprintf(stderr, "%s\n", qPrintable(fullMsg));
//         // 可选：触发一个断言，方便在调试器中中断
//         // Q_ASSERT(false);
//     }

//     // 继续默认处理（可保留原有行为）
//     // 注意：这里不再调用默认 handler，否则会重复输出
//     // 如果需要保留原有输出，可以调用 qDefaultMessageHandler
// }

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

         // 安装自定义消息处理器
    //qInstallMessageHandler(messageHandler);

    MotionCtrlWidget w;
    w.show();
    return a.exec();
}

#include "Logger.h"

#include <QByteArray>
#include <QString>

void QSK::Log([[maybe_unused]] QtMsgType a_type, [[maybe_unused]] const QMessageLogContext& a_context, [[maybe_unused]] const QString& a_msg)
{
	QByteArray localMsg = a_msg.toLocal8Bit();
	char buf[2048];

	switch (a_type)
	{
	case QtDebugMsg:
		sprintf_s(buf, "[Qt] %s (%s:%u, %s)\n", localMsg.constData(), a_context.file, a_context.line, a_context.function);
		SKSE::log::trace(std::string(buf));
		break;
	case QtInfoMsg:
		sprintf_s(buf, "[Qt] %s (%s:%u, %s)\n", localMsg.constData(), a_context.file, a_context.line, a_context.function);
		SKSE::log::info(std::string(buf));
		break;
	case QtWarningMsg:
		sprintf_s(buf, "[Qt] %s (%s:%u, %s)\n", localMsg.constData(), a_context.file, a_context.line, a_context.function);
		SKSE::log::warn(std::string(buf));
		break;
	case QtCriticalMsg:
		sprintf_s(buf, "[Qt] %s (%s:%u, %s)\n", localMsg.constData(), a_context.file, a_context.line, a_context.function);
		SKSE::log::critical(std::string(buf));
		break;
	case QtFatalMsg:
		sprintf_s(buf, "[Qt] %s (%s:%u, %s)\n", localMsg.constData(), a_context.file, a_context.line, a_context.function);
		SKSE::log::critical(std::string(buf));
		RE::Main::GetSingleton()->quitGame = true;
	}
}

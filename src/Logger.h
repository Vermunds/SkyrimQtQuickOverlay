#pragma once

#include <QDebug>

namespace QSK
{
	void Log(QtMsgType a_type, const QMessageLogContext& a_context, const QString& a_msg);
}

#pragma once

class QGuiApplication;
class QQuickRenderControl;
class QQuickWindow;
class QQmlEngine;
class QQmlComponent;
class QQuickItem;
class QEventLoop;

namespace QSK
{
	class SkyrimQuickApplication
	{
	public:
		void UpdateAndRender();
		void Initialize();

		static SkyrimQuickApplication* GetSingleton();
		static void SetDirty();  // Static function so I can use it as a QT slot without having to derive from QObject

		const bool isInitialized();

		void ProcessAllEvents();

	private:
		SkyrimQuickApplication(){};
		~SkyrimQuickApplication();
		SkyrimQuickApplication(const SkyrimQuickApplication&) = delete;
		SkyrimQuickApplication& operator=(const SkyrimQuickApplication&) = delete;

		QGuiApplication* m_app;
		QQuickRenderControl* m_renderControl;
		QQuickWindow* m_quickWindow;
		QQmlEngine* m_qmlEngine;
		QQmlComponent* m_qmlComponent;
		QQuickItem* m_rootItem;

		bool m_quickInitialized = false;
		bool m_quickDirty = true;
	};
}

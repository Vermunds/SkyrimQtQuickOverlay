#include "SkyrimQuickApplication.h"
#include "Logger.h"
#include "Renderer.h"

#include <QEventLoop>
#include <QGuiApplication>
#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickGraphicsDevice>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickRenderTarget>
#include <QQuickWindow>

namespace QSK
{
	void SkyrimQuickApplication::UpdateAndRender()
	{
		if (!m_quickDirty || !m_quickInitialized)
		{
			return;
		}

		m_quickDirty = false;

		m_renderControl->polishItems();

		m_renderControl->beginFrame();
		m_renderControl->sync();
		m_renderControl->render();
		m_renderControl->endFrame();  // Qt Quick's rendering commands are submitted to the device context here
	}

	void SkyrimQuickApplication::Initialize()
	{
		Renderer* renderer = Renderer::GetSingleton();

		QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11);
		qInstallMessageHandler(QSK::Log);

		int argc = 1;
		char* argv = { 0 };

		m_app = new QGuiApplication(argc, &argv);
		m_renderControl = new QQuickRenderControl(m_app);

		// Whenever something changed in the Quick scene, or rendering was
		// requested via others means (e.g. QQuickWindow::update()), it should
		// trigger rendering into the texture when preparing the next frame.
		QObject::connect(m_renderControl, &QQuickRenderControl::renderRequested, SetDirty);
		QObject::connect(m_renderControl, &QQuickRenderControl::sceneChanged, SetDirty);

		// No multisampling
		m_renderControl->setSamples(1);

		// Create a QQuickWindow that is associated with out render control. Note that this
		// window never gets created or shown, meaning that it will never get an underlying
		// native (platform) window.
		m_quickWindow = new QQuickWindow(m_renderControl);

		m_qmlEngine = new QQmlEngine(m_app);
		m_qmlComponent = new QQmlComponent(m_qmlEngine, QUrl("file:qml/main.qml"));
		if (m_qmlComponent->isError())
		{
			for (const QQmlError& error : m_qmlComponent->errors())
			{
				qWarning() << error.url() << error.line() << error;
			}
		}

		QObject* rootObject = m_qmlComponent->create();
		if (m_qmlComponent->isError())
		{
			for (const QQmlError& error : m_qmlComponent->errors())
			{
				qWarning() << error.url() << error.line() << error;
			}
		}

		m_rootItem = qobject_cast<QQuickItem*>(rootObject);
		m_rootItem->setSize(QSize(renderer->GetRenderWidth(), renderer->GetRenderHeight()));
		m_quickWindow->contentItem()->setSize(m_rootItem->size());
		m_quickWindow->setColor(Qt::transparent);
		m_quickWindow->setGeometry(0, 0, m_rootItem->width(), m_rootItem->height());

		m_rootItem->setParentItem(m_quickWindow->contentItem());

		// In addition to setGraphicsApi(), we need a call to
		// setGraphicsDevice to tell Qt Quick what ID3D11Device(Context) to use
		// (i.e. we want it to use ours, not to create new ones).
		m_quickWindow->setGraphicsDevice(QQuickGraphicsDevice::fromDeviceAndContext(renderer->GetDevice(), renderer->GetContext()));

		// Now we can kick off the scenegraph.
		if (!m_renderControl->initialize())
		{
			SKSE::stl::report_and_fail("Failed to initialize redirected Qt Quick rendering.");
		}

		// Redirect Qt Quick's output.
		m_quickWindow->setRenderTarget(QQuickRenderTarget::fromD3D11Texture(renderer->GetTexture(), QSize(renderer->GetRenderWidth(), renderer->GetRenderHeight()), 1));
		m_rootItem->forceActiveFocus();

		m_quickInitialized = true;
	}

	SkyrimQuickApplication* SkyrimQuickApplication::GetSingleton()
	{
		static SkyrimQuickApplication singleton;
		return &singleton;
	}
	void SkyrimQuickApplication::SetDirty()
	{
		SkyrimQuickApplication::GetSingleton()->m_quickDirty = true;
	}
	const bool SkyrimQuickApplication::isInitialized()
	{
		return m_quickInitialized;
	}
	void SkyrimQuickApplication::ProcessAllEvents()
	{
		return m_app->processEvents(QEventLoop::AllEvents);
	}
	SkyrimQuickApplication::~SkyrimQuickApplication()
	{
		m_app->exit();
	}
}

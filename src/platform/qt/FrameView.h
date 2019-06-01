/* Copyright (c) 2013-2019 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ui_FrameView.h"

#include <QBitmap>
#include <QImage>
#include <QList>
#include <QMutex>
#include <QPixmap>
#include <QSet>
#include <QTimer>

#include "AssetView.h"

#include <mgba-util/vfs.h>

#include <memory>

struct VFile;

namespace QGBA {

class CoreController;

class FrameView : public AssetView {
Q_OBJECT

public:
	FrameView(std::shared_ptr<CoreController> controller, QWidget* parent = nullptr);
	~FrameView();

public slots:
	void selectLayer(const QPointF& coord);
	void disableLayer(const QPointF& coord);
	void exportFrame();

protected:
#ifdef M_CORE_GBA
	void updateTilesGBA(bool force) override;
	void injectGBA();
#endif
#ifdef M_CORE_GB
	void updateTilesGB(bool force) override;
	void injectGB();
#endif

	bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
	void invalidateQueue(const QSize& = {});
	void updateRendered();
	void refreshVl();
	void newVl();

private:
	struct LayerId {
		enum {
			NONE = 0,
			BACKGROUND,
			WINDOW,
			SPRITE,
			BACKDROP
		} type = NONE;
		int index = -1;

		bool operator!=(const LayerId& other) const { return other.type != type || other.index != index; }
		operator uint() const { return (type << 8) | index; }
		QString readable() const;
	};

	struct Layer {
		LayerId id;
		bool enabled;
		QPixmap image;
		QRegion mask;
		QPointF location;
		bool repeats;
	};

	bool lookupLayer(const QPointF& coord, Layer*&);

	static void frameCallback(FrameView*, std::shared_ptr<bool>);

	Ui::FrameView m_ui;

	LayerId m_active{};

	int m_glowFrame;
	QTimer m_glowTimer;

	QMutex m_mutex{QMutex::Recursive};
	VFile* m_currentFrame = nullptr;
	VFile* m_nextFrame = nullptr;
	mCore* m_vl = nullptr;
	QImage m_framebuffer;

	QSize m_dims;
	QList<Layer> m_queue;
	QSet<LayerId> m_disabled;
	QPixmap m_composited;
	QPixmap m_rendered;
	mMapCacheEntry m_mapStatus[4][128 * 128] = {}; // TODO: Correct size

#ifdef M_CORE_GBA
	uint16_t m_gbaDispcnt;
#endif

	std::shared_ptr<bool> m_callbackLocker{std::make_shared<bool>(true)};
};

}
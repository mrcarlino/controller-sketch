#pragma once
#include <QWidget>
#include <QImage>
#include <QPointF>
#include <QColor>
#include <QTimer>

#include "ControllerInput.h"

class RenderCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit RenderCanvas(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updatesFromController();

    // Paint helpers
    void drawFlashOverlay(QPainter& painter);
    void drawReticle(QPainter& painter);
    void drawColorWheel(QPainter& painter);

    // Input logic helpers
    void handleBrushMovement();
    void handleBrushOpacity();
    void handleBrushPressure();
    void handleColorWheelInput();
    void handleClearCanvas();
    void handleSaveCanvas();
    void drawBrush();

    bool saveCanvasToPng(const QImage& image, const std::string& prefix = "ControllerSketch");

    // Brush state
    QColor mBrushColor;
    int mBrushSize;
    int mBrushOpacity;
    int mSelectedColorIndex;

    // Canvas
    QImage mCanvasBuffer;
    QColor mCanvasColor;
    QPointF mCursorPos;

    // State flags
    bool mIsDrawing;
    bool mIsColorWheelActive;
    bool mShowFlash;

    // Input
    ControllerInput* mController;

    // Timers
    QTimer* mUpdateTimer;
    QTimer mFlashTimer;   
};
#include "RenderCanvas.h"
#include "Brush.h"
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QColorDialog>
#include <iostream>
#include <cmath>
#include <chrono>
#include <ctime>
#include <sstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static constexpr float brushMoveSpeed = 6.0f;
static constexpr int maxBrushSize = 20;
static constexpr int minOpacity = 80;
static constexpr int maxOpacity = 255;
static constexpr int colorWheelRadius = 100;
static constexpr int colorBubbleSize = 30;
static const QVector<QColor> presetColors = { Qt::black, Qt::red, Qt::green, Qt::blue, Qt::yellow, Qt::magenta, Qt::cyan, Qt::white };

RenderCanvas::RenderCanvas(QWidget* parent) :
    QWidget(parent),
    mCursorPos(400, 300),
    mCanvasColor(Qt::white),
    mBrushColor(Qt::blue),
    mBrushSize(20),
    mBrushOpacity(255),
    mSelectedColorIndex(0),
    mIsColorWheelActive(false),
    mIsDrawing(false),
    mShowFlash(false),
    mController(new ControllerInput()),
    mUpdateTimer(new QTimer(this))
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    // Create persist canvas using QImage
    mCanvasBuffer = QImage(size(), QImage::Format_ARGB32);
    mCanvasBuffer.fill(mCanvasColor);

    connect(mUpdateTimer, &QTimer::timeout, this, &RenderCanvas::updatesFromController);
    mUpdateTimer->start(16);
}

void RenderCanvas::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    // Draw canvas
    painter.drawImage(0, 0, mCanvasBuffer);

    // Visual cue for saved image
    drawFlashOverlay(painter);

    // Draw reticle if not drawing
    drawReticle(painter);

    // Draw color wheel if active
    drawColorWheel(painter);
}

void RenderCanvas::resizeEvent(QResizeEvent* event)
{
    QImage newImage(size(), QImage::Format_ARGB32);
    newImage.fill(mCanvasColor); // Fill with background color
    QPainter painter(&newImage); // Painter on new canvas
    painter.drawImage(0, 0, mCanvasBuffer); // Copy old image onto new canvas
    mCanvasBuffer = newImage; // Replace the old canvas with resized one
    QWidget::resizeEvent(event);
}

void RenderCanvas::drawFlashOverlay(QPainter& painter)
{
    if (mShowFlash)
    {
        painter.fillRect(rect(), QColor(255, 255, 255, 200));  // white overlay
    }
}

void RenderCanvas::drawReticle(QPainter& painter)
{
    if (!mIsDrawing)
    {
        QPen pen(Qt::gray);
        pen.setWidth(2);
        painter.setPen(pen);
        
        int size = 10;

        painter.drawLine(QPointF(mCursorPos.x() - size, mCursorPos.y()),
                         QPointF(mCursorPos.x() + size, mCursorPos.y()));
        painter.drawLine(QPointF(mCursorPos.x(), mCursorPos.y() - size),
                         QPointF(mCursorPos.x(), mCursorPos.y() + size));
    }
}

void RenderCanvas::drawColorWheel(QPainter& painter)
{
    if (mIsColorWheelActive && !presetColors.isEmpty())
    {
        int cx = width() / 2;
        int cy = height() / 2;
        int count = presetColors.size();

        // Background overlay
        QRadialGradient bgGradient(QPointF(cx, cy), colorWheelRadius);
        bgGradient.setColorAt(0.0, QColor(80, 80, 80, 245));   // center lighter
        bgGradient.setColorAt(1.0, QColor(40, 40, 40, 245));   // edge darker
        painter.setBrush(bgGradient); // Translucent black
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(cx, cy), colorWheelRadius, colorWheelRadius);

        // Draw highlighted wedges
        float startAngle = (360.0 / count) * mSelectedColorIndex - (180.0 / count);
        float spanAngle = 360.0 / count;

        QPainterPath wedge;
        wedge.moveTo(cx, cy);
        wedge.arcTo(cx - colorWheelRadius, cy - colorWheelRadius, 2 * colorWheelRadius, 2 * colorWheelRadius, -startAngle, -spanAngle);
        wedge.closeSubpath();

        painter.setBrush(QColor(255, 255, 255, 40));
        painter.setPen(Qt::NoPen);
        painter.drawPath(wedge);

        // Draw color bubbles
        for (int i = 0; i < count; i++)
        {
            float angle = 2 * M_PI * i / count;
            int x = cx + colorWheelRadius * std::cos(angle);
            int y = cy + colorWheelRadius * std::sin(angle);

            QRect rect(x - colorBubbleSize / 2, y - colorBubbleSize / 2, colorBubbleSize, colorBubbleSize);

            // Outline for bubble
            painter.setBrush(Qt::NoBrush);
            QPen outline(Qt::white);
            outline.setWidth(3);
            painter.setPen(outline);
            painter.drawEllipse(rect.adjusted(-2, -2, 2, 2));

            // Draw color circle
            QRadialGradient gradient(x, y, colorBubbleSize / 2);
            gradient.setColorAt(0.0, presetColors[i].lighter(130));
            gradient.setColorAt(1.0, presetColors[i].darker(130));
            painter.setBrush(gradient);
            painter.setPen(Qt::black);
            painter.drawEllipse(rect);
        }
    }
}

void RenderCanvas::updatesFromController()
{
    if (mController && mController->isConnected())
    {
        mController->pollEvents();

        // Brush movement
        handleBrushMovement();

        // L1 opens the brush color wheel
        handleColorWheelInput();

        // L2 controls brush opacity
        handleBrushOpacity();

        // L3 to clear canvas
        handleClearCanvas();

        // R2 controls brush pressure
        handleBrushPressure();

        // Draw brush onto the canvas
        drawBrush();

        // Share button to save image
        handleSaveCanvas();

        update();
    }
}

void RenderCanvas::handleBrushMovement()
{
    float dx = mController->getAxis(SDL_CONTROLLER_AXIS_LEFTX);
    float dy = mController->getAxis(SDL_CONTROLLER_AXIS_LEFTY);
    mCursorPos.rx() += dx * brushMoveSpeed;
    mCursorPos.ry() += dy * brushMoveSpeed;
    // Keep inside canvas
    mCursorPos.setX(qBound(0.0, mCursorPos.x(), (double)width()));
    mCursorPos.setY(qBound(0.0, mCursorPos.y(), (double)height()));
}

void RenderCanvas::handleBrushOpacity()
{
    float opacityInput = mController->getAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT); // 0.0 to 1.0
    
    if (opacityInput > 0.05f)
    {
        float curved = std::pow(opacityInput, 2.0f);  // smooth curve
        int dynamicOpacity = static_cast<int>(curved * (maxOpacity - minOpacity)) + minOpacity;
        mBrushOpacity = std::clamp(dynamicOpacity, minOpacity, maxOpacity);
    }
    else
    {
        mBrushOpacity = minOpacity;  // subtle default when not actively adjusting
    }
}

void RenderCanvas::handleBrushPressure()
{
     float pressure = mController->getAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
    mIsDrawing = pressure > 0.05f;
    mBrushSize = pressure * maxBrushSize;
}

void RenderCanvas::drawBrush()
{
    if (mIsDrawing)
    {
        Brush brush(mBrushColor, mBrushSize, mBrushOpacity);
        QPainter painter(&mCanvasBuffer);
        brush.draw(painter, mCursorPos);
    }
}

void RenderCanvas::handleColorWheelInput()
{
    mIsColorWheelActive = mController->isButtonPressed(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);

    if (mIsColorWheelActive)
    {
        float rx = mController->getAxis(SDL_CONTROLLER_AXIS_RIGHTX);
        float ry = mController->getAxis(SDL_CONTROLLER_AXIS_RIGHTY);
        float magnitude = std::sqrt(rx * rx + ry * ry);

        if (magnitude > 0.3f && !presetColors.isEmpty())
        {
            float angle = std::atan2(ry, rx); // -PI to PI
            if (angle < 0) angle += 2 * M_PI; // Normalize to 0 to 2PI

            int count = presetColors.size();
            mSelectedColorIndex = static_cast<int>((angle / (2 * M_PI)) * count) % count;
        }
    }
    else if (!presetColors.isEmpty() && mSelectedColorIndex < presetColors.size())
    {
        mBrushColor = presetColors[mSelectedColorIndex];
    }
}

void RenderCanvas::handleClearCanvas()
{
    if (mController->isButtonPressed(SDL_CONTROLLER_BUTTON_LEFTSTICK))
    {
        mCanvasBuffer.fill(mCanvasColor);
        update();
    }
}

void RenderCanvas::handleSaveCanvas()
{
    static bool sharePressed = false;
    bool isShareActive = mController->isButtonPressed(SDL_CONTROLLER_BUTTON_START);

    if (isShareActive && !sharePressed)
    {
        bool result = saveCanvasToPng(mCanvasBuffer);
        std::cout << "Save result: " << (result ? "Success" : "FAIL") << std::endl;
    }
    sharePressed = isShareActive;
}

bool RenderCanvas::saveCanvasToPng(const QImage& image, const std::string& prefix)
{
    QImage rgbaImage = image.convertToFormat(QImage::Format_RGBA8888);
    if (rgbaImage.isNull())
        return false;

    int width = rgbaImage.width();
    int height = rgbaImage.height();
    const unsigned char* data = rgbaImage.bits();
    int stride = rgbaImage.bytesPerLine();

    // Create a unique filename with epoch timestamp
    auto now = std::chrono::system_clock::now();
    auto now_secs = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    std::ostringstream oss;
    oss << prefix << "_" << now_secs << ".png";
    std::string filename = oss.str();

    mShowFlash = true;
    mFlashTimer.singleShot(300, this, [this]() {
        mShowFlash = false;
        update();
    });

    // Save color image using stb
    int result = stbi_write_png(filename.c_str(), width, height, 4, data, stride);
    return result != 0;
}

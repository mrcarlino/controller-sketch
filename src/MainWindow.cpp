#include "MainWindow.h"
#include "RenderCanvas.h"

MainWindow::MainWindow() :
    mCanvas(new RenderCanvas(this))
{
    setCentralWidget(mCanvas);
    setWindowTitle("ControllerSketch");
    resize(800, 600); // TODO: fullscreen
}
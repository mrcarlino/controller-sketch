#pragma once
#include <memory>
#include <QMainWindow>

class RenderCanvas;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
private:
    RenderCanvas* mCanvas;
};
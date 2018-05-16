#include "scenetooltexturebrush.hpp"

#include <QFrame>
#include <QIcon>
#include <QTableWidget>
#include <QHBoxLayout>

#include <QWidget>
#include <QSpinBox>
#include <QGroupBox>
#include <QSlider>
#include <QEvent>
#include <QDropEvent>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <QDrag>
#include <QTableWidget>
#include <QHeaderView>
#include <QApplication>
#include <QSizePolicy>

#include "scenetool.hpp"
#include "../../model/doc/document.hpp"
#include "../../model/world/data.hpp"
#include "../../model/world/idtable.hpp"
#include "../../model/world/landtexture.hpp"


CSVWidget::BrushSizeControls::BrushSizeControls(const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    mBrushSizeSlider = new QSlider(Qt::Horizontal);
    mBrushSizeSlider->setTickPosition(QSlider::TicksBothSides);
    mBrushSizeSlider->setTickInterval(10);
    mBrushSizeSlider->setRange(1, 50);
    mBrushSizeSlider->setSingleStep(1);

    mBrushSizeSpinBox = new QSpinBox;
    mBrushSizeSpinBox->setRange(1, 50);
    mBrushSizeSpinBox->setSingleStep(1);

    mLayoutSliderSize = new QHBoxLayout;
    mLayoutSliderSize->addWidget(mBrushSizeSlider);
    mLayoutSliderSize->addWidget(mBrushSizeSpinBox);

    connect(mBrushSizeSlider, SIGNAL(valueChanged(int)), mBrushSizeSpinBox, SLOT(setValue(int)));
    connect(mBrushSizeSpinBox, SIGNAL(valueChanged(int)), mBrushSizeSlider, SLOT(setValue(int)));

    setLayout(mLayoutSliderSize);
}

CSVWidget::TextureBrushWindow::TextureBrushWindow(CSMDoc::Document& document, QWidget *parent)
    : QFrame(parent, Qt::Popup),
    mBrushShape(0),
    mBrushSize(0),
    mBrushTexture("L0#0"),
    mDocument(document)
{
    CSMWorld::IdTable& ltexTable = dynamic_cast<CSMWorld::IdTable&> (
        *mDocument.getData().getTableModel (CSMWorld::UniversalId::Type_LandTextures));

    int landTextureFilename = ltexTable.findColumnIndex(CSMWorld::Columns::ColumnId_Texture);

    QModelIndex index = ltexTable.getModelIndex (mBrushTexture, landTextureFilename);
    mBrushTextureLabel = "Selected texture: " + mBrushTexture + " ";
    mSelectedBrush = new QLabel(QString::fromStdString(mBrushTextureLabel) + ltexTable.data(index).value<QString>());

    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->setSpacing(0);
    layoutMain->setContentsMargins(4,0,4,4);

    QHBoxLayout *layoutHorizontal = new QHBoxLayout;
    layoutHorizontal->setSpacing(0);
    layoutHorizontal->setContentsMargins (QMargins (0, 0, 0, 0));

    configureButtonInitialSettings(mButtonPoint);
    configureButtonInitialSettings(mButtonSquare);
    configureButtonInitialSettings(mButtonCircle);
    configureButtonInitialSettings(mButtonCustom);

    mButtonPoint->setToolTip (toolTipPoint);
    mButtonSquare->setToolTip (toolTipSquare);
    mButtonCircle->setToolTip (toolTipCircle);
    mButtonCustom->setToolTip (toolTipCustom);

    QButtonGroup* brushButtonGroup = new QButtonGroup(this);
    brushButtonGroup->addButton(mButtonPoint);
    brushButtonGroup->addButton(mButtonSquare);
    brushButtonGroup->addButton(mButtonCircle);
    brushButtonGroup->addButton(mButtonCustom);

    brushButtonGroup->setExclusive(true);

    layoutHorizontal->addWidget(mButtonPoint, 0, Qt::AlignTop);
    layoutHorizontal->addWidget(mButtonSquare, 0, Qt::AlignTop);
    layoutHorizontal->addWidget(mButtonCircle, 0, Qt::AlignTop);
    layoutHorizontal->addWidget(mButtonCustom, 0, Qt::AlignTop);

    mHorizontalGroupBox = new QGroupBox(tr(""));
    mHorizontalGroupBox->setLayout(layoutHorizontal);

    layoutMain->addWidget(mHorizontalGroupBox);
    layoutMain->addWidget(mSizeSliders);
    layoutMain->addWidget(mSelectedBrush);

    setLayout(layoutMain);

    connect(mButtonPoint, SIGNAL(clicked()), this, SLOT(setBrushShape()));
    connect(mButtonSquare, SIGNAL(clicked()), this, SLOT(setBrushShape()));
    connect(mButtonCircle, SIGNAL(clicked()), this, SLOT(setBrushShape()));
    connect(mButtonCustom, SIGNAL(clicked()), this, SLOT(setBrushShape()));
}

void CSVWidget::TextureBrushWindow::configureButtonInitialSettings(QPushButton *button)
{
  button->setSizePolicy (QSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed));
  button->setContentsMargins (QMargins (0, 0, 0, 0));
  button->setIconSize (QSize (48-6, 48-6));
  button->setFixedSize (48, 48);
  button->setCheckable(true);
}

void CSVWidget::TextureBrushWindow::setBrushTexture(std::string brushTexture)
{
    CSMWorld::IdTable& ltexTable = dynamic_cast<CSMWorld::IdTable&> (
        *mDocument.getData().getTableModel (CSMWorld::UniversalId::Type_LandTextures));
    int landTextureFilename = ltexTable.findColumnIndex(CSMWorld::Columns::ColumnId_Texture);

    mBrushTexture = brushTexture;
    mBrushTextureLabel = "Selected texture: " + mBrushTexture + " ";
    QModelIndex index = ltexTable.getModelIndex (mBrushTexture, landTextureFilename);
    mSelectedBrush->setText(QString::fromStdString(mBrushTextureLabel) + ltexTable.data(index).value<QString>());
    emit passBrushShape(mBrushShape); // update icon

}

void CSVWidget::TextureBrushWindow::setBrushSize(int brushSize)
{
    mBrushSize = brushSize;
    emit passBrushSize(mBrushSize);
}

void CSVWidget::TextureBrushWindow::setBrushShape()
{
    if(mButtonPoint->isChecked()) mBrushShape = 0;
    if(mButtonSquare->isChecked()) mBrushShape = 1;
    if(mButtonCircle->isChecked()) mBrushShape = 2;
    if(mButtonCustom->isChecked()) mBrushShape = 3;
    emit passBrushShape(mBrushShape);
}

void CSVWidget::SceneToolTextureBrush::adjustToolTips()
{
}

CSVWidget::SceneToolTextureBrush::SceneToolTextureBrush (SceneToolbar *parent, const QString& toolTip, CSMDoc::Document& document)
: SceneTool (parent, Type_TopAction),
    mToolTip (toolTip),
    mDocument (document),
    mTextureBrushWindow(new TextureBrushWindow(document, this))
{
    mBrushHistory.resize(1);
    mBrushHistory[0] = "L0#0";

    setAcceptDrops(true);
    connect(mTextureBrushWindow, SIGNAL(passBrushShape(int)), this, SLOT(setButtonIcon(int)));
    setButtonIcon(mTextureBrushWindow->mBrushShape);

    mPanel = new QFrame (this, Qt::Popup);

    QHBoxLayout *layout = new QHBoxLayout (mPanel);

    layout->setContentsMargins (QMargins (0, 0, 0, 0));

    mTable = new QTableWidget (0, 2, this);

    mTable->setShowGrid (true);
    mTable->verticalHeader()->hide();
    mTable->horizontalHeader()->hide();
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    mTable->horizontalHeader()->setSectionResizeMode (0, QHeaderView::Stretch);
    mTable->horizontalHeader()->setSectionResizeMode (1, QHeaderView::Stretch);
#else
    mTable->horizontalHeader()->setResizeMode (0, QHeaderView::Stretch);
    mTable->horizontalHeader()->setResizeMode (1, QHeaderView::Stretch);
#endif
    mTable->setSelectionMode (QAbstractItemView::NoSelection);

    layout->addWidget (mTable);

    connect (mTable, SIGNAL (clicked (const QModelIndex&)),
        this, SLOT (clicked (const QModelIndex&)));

}

void CSVWidget::SceneToolTextureBrush::setButtonIcon (int brushShape)
{
    QString tooltip = "Brush settings <p>Currently selected: ";
    if(brushShape == 0)
    {
        setIcon (QIcon (QPixmap (":scenetoolbar/brush-point")));
        tooltip += dynamic_cast<QString&> (mTextureBrushWindow->toolTipPoint);
    }
    if(brushShape == 1)
    {
        setIcon (QIcon (QPixmap (":scenetoolbar/brush-square")));
        tooltip += dynamic_cast<QString&> (mTextureBrushWindow->toolTipSquare);
    }
    if(brushShape == 2)
    {
        setIcon (QIcon (QPixmap (":scenetoolbar/brush-circle")));
        tooltip += dynamic_cast<QString&> (mTextureBrushWindow->toolTipCircle);
    }
    if(brushShape == 3)
    {
        setIcon (QIcon (QPixmap (":scenetoolbar/brush-custom")));
        tooltip += dynamic_cast<QString&> (mTextureBrushWindow->toolTipCustom);
    }
    tooltip += "<p>Selected texture: " + QString::fromStdString(mTextureBrushWindow->mBrushTexture) + " ";

    CSMWorld::IdTable& ltexTable = dynamic_cast<CSMWorld::IdTable&> (
        *mDocument.getData().getTableModel (CSMWorld::UniversalId::Type_LandTextures));
    int landTextureFilename = ltexTable.findColumnIndex(CSMWorld::Columns::ColumnId_Texture);
    QModelIndex index = ltexTable.getModelIndex (mTextureBrushWindow->mBrushTexture, landTextureFilename);
    tooltip += ltexTable.data(index).value<QString>();

    tooltip += "<br>(drop texture here to change)";
    setToolTip (tooltip);
}

void CSVWidget::SceneToolTextureBrush::showPanel (const QPoint& position)
{
    updatePanel();
    mPanel->move (position);
    mPanel->show();
}

void CSVWidget::SceneToolTextureBrush::updatePanel()
{
    mTable->setRowCount (mBrushHistory.size());

    for (int i = mBrushHistory.size()-1; i >= 0; --i)
    {
        CSMWorld::IdTable& ltexTable = dynamic_cast<CSMWorld::IdTable&> (
            *mDocument.getData().getTableModel (CSMWorld::UniversalId::Type_LandTextures));
        int landTextureFilename = ltexTable.findColumnIndex(CSMWorld::Columns::ColumnId_Texture);
        QModelIndex index = ltexTable.getModelIndex (mBrushHistory[i], landTextureFilename);
        mTable->setItem (i, 1, new QTableWidgetItem (ltexTable.data(index).value<QString>()));
        mTable->setItem (i, 0, new QTableWidgetItem (QString::fromStdString(mBrushHistory[i])));
    }
}

void CSVWidget::SceneToolTextureBrush::updateBrushHistory (const std::string& brushTexture)
{
    mBrushHistory.insert(mBrushHistory.begin(), brushTexture);
    if(mBrushHistory.size() > 5) mBrushHistory.pop_back();
}

void CSVWidget::SceneToolTextureBrush::clicked (const QModelIndex& index)
{
    if (index.column()==0 || index.column()==1)
    {
        std::string brushTexture = mBrushHistory[index.row()];
        std::swap(mBrushHistory[index.row()], mBrushHistory[0]);
        mTextureBrushWindow->setBrushTexture(brushTexture);
        emit passTextureId(brushTexture);
        updatePanel();
        mPanel->hide();
    }
}

void CSVWidget::SceneToolTextureBrush::activate ()
{
    QPoint position = QCursor::pos();
    mTextureBrushWindow->move (position);
    mTextureBrushWindow->show();
}

void CSVWidget::SceneToolTextureBrush::dragEnterEvent (QDragEnterEvent *event)
{
    emit passEvent(event);
    event->accept();
}
void CSVWidget::SceneToolTextureBrush::dropEvent (QDropEvent *event)
{
    emit passEvent(event);
    event->accept();
}
